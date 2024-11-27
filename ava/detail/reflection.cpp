#include "reflection.hpp"

#include <set>
#include "spirv_cross.hpp"

namespace ava::detail
{
    // https://github.com/KhronosGroup/SPIRV-Cross/wiki/Reflection-API-user-guide

    static spv::ExecutionModel getExecutionModelFromStage(vk::ShaderStageFlagBits stage)
    {
        switch (stage)
        {
        case vk::ShaderStageFlagBits::eVertex:
            return spv::ExecutionModelVertex;
        case vk::ShaderStageFlagBits::eFragment:
            return spv::ExecutionModelFragment;
        case vk::ShaderStageFlagBits::eCompute:
            return spv::ExecutionModelGLCompute;
        case vk::ShaderStageFlagBits::eGeometry:
            return spv::ExecutionModelGeometry;
        case vk::ShaderStageFlagBits::eTessellationControl:
            return spv::ExecutionModelTessellationControl;
        case vk::ShaderStageFlagBits::eTessellationEvaluation:
            return spv::ExecutionModelTessellationEvaluation;
        case vk::ShaderStageFlagBits::eRaygenKHR:
            return spv::ExecutionModelRayGenerationKHR;
        case vk::ShaderStageFlagBits::eMissKHR:
            return spv::ExecutionModelMissKHR;
        case vk::ShaderStageFlagBits::eClosestHitKHR:
            return spv::ExecutionModelClosestHitKHR;
        case vk::ShaderStageFlagBits::eAnyHitKHR:
            return spv::ExecutionModelAnyHitKHR;
        case vk::ShaderStageFlagBits::eIntersectionKHR:
            return spv::ExecutionModelIntersectionKHR;
        case vk::ShaderStageFlagBits::eCallableKHR:
            return spv::ExecutionModelCallableKHR;
        case vk::ShaderStageFlagBits::eMeshEXT:
            return spv::ExecutionModelMeshEXT;
        case vk::ShaderStageFlagBits::eTaskEXT:
            return spv::ExecutionModelTaskEXT;
        default:
            throw std::runtime_error("Cannot get spv::ExecutionModel from unhandled stage");
        }
    }

    static ReflectedShaderInfo reflectShader(const Shader* const& shader)
    {
        ReflectedShaderInfo info;
        auto stage = shader->stage;

        auto addLayoutBinding = [&info, &stage](uint32_t set, uint32_t binding, vk::DescriptorType descriptorType, uint32_t descriptorCount) -> void
        {
            if (set >= info.layoutBindings.size())
            {
                info.layoutBindings.resize(set + 1);
            }

            info.layoutBindings[set].emplace_back(binding, descriptorType, descriptorCount, stage);
        };

        spirv_cross::Compiler compiler(reinterpret_cast<const uint32_t*>(shader->spriv.data()), shader->spriv.size() / sizeof(uint32_t));
        auto updateResources = [&]<typename T>(const spirv_cross::SmallVector<T>& resources, vk::DescriptorType descriptorType, std::optional<vk::DescriptorType> dimBufferDescriptorType = {}) -> void
        {
            for (const auto& resource : resources)
            {
                auto set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

                const auto& spirType = compiler.get_type(resource.type_id);
                uint32_t descriptorCount = 1;
                for (auto size : spirType.array) // array[4][2] is 2 * 4 descriptor counts (arrays of arrays are backwards)
                {
                    if (size == 0) size = 64; // array sizes can be 0 if they are unsized, let's presume 64 array size if unsized
                    descriptorCount *= size;
                }
                // TODO: allow specialization constant sized arrays

                // If image dim is DimBuffer then take use the dimBufferDescriptorType if it exists, otherwise use the main descriptorType
                auto outDescType = (spirType.image.dim == spv::DimBuffer) ? dimBufferDescriptorType.value_or(descriptorType) : descriptorType;
                addLayoutBinding(set, binding, outDescType, descriptorCount);
            }
        };

        auto entryPoints = compiler.get_entry_points_and_stages();
        compiler.set_entry_point(shader->entry, getExecutionModelFromStage(shader->stage));

        // Shader resource inputs
        auto shaderResources = compiler.get_shader_resources();
        updateResources(shaderResources.sampled_images, vk::DescriptorType::eCombinedImageSampler); // sampler2D
        updateResources(shaderResources.separate_samplers, vk::DescriptorType::eSampler); // sampler/samplerShadow
        updateResources(shaderResources.separate_images, vk::DescriptorType::eCombinedImageSampler, vk::DescriptorType::eUniformTexelBuffer); // texture2D, samplerBuffer
        updateResources(shaderResources.storage_images, vk::DescriptorType::eStorageBuffer, vk::DescriptorType::eStorageTexelBuffer); // image2D, imageBuffer
        updateResources(shaderResources.uniform_buffers, vk::DescriptorType::eUniformBuffer); // uniform UBO {}
        updateResources(shaderResources.storage_buffers, vk::DescriptorType::eStorageBuffer); // buffer SSBO {}
        updateResources(shaderResources.subpass_inputs, vk::DescriptorType::eInputAttachment); // subpass input
        updateResources(shaderResources.acceleration_structures, vk::DescriptorType::eAccelerationStructureKHR);

        // Push constants
        for (const auto& pushConstantResource : shaderResources.push_constant_buffers)
        {
            const auto& spirType = compiler.get_type(pushConstantResource.base_type_id);
            auto size = compiler.get_declared_struct_size(spirType);
            auto offset = compiler.get_decoration(pushConstantResource.id, spv::DecorationOffset);
            info.pushConstants.emplace_back(stage, offset, size);
        }

        return info;
    }

    static void combine(ReflectedShaderInfo& out, const ReflectedShaderInfo& in)
    {
        // Layout bindings
        for (uint32_t set = 0; set < in.layoutBindings.size(); set++)
        {
            if (set >= out.layoutBindings.size())
            {
                out.layoutBindings.resize(set + 1);
            }

            auto& outSetBindings = out.layoutBindings[set];
            const auto& inSetBindings = in.layoutBindings[set];

            std::set<uint32_t> duplicatedBindings;

            // Handle duplicate bindings
            for (auto& outBinding : outSetBindings)
            {
                for (const auto& inBinding : inSetBindings)
                {
                    if (outBinding.binding == inBinding.binding && outBinding.descriptorType == inBinding.descriptorType)
                    {
                        outBinding.descriptorCount = std::max(outBinding.descriptorCount, inBinding.descriptorCount);
                        outBinding.stageFlags |= inBinding.stageFlags;

                        duplicatedBindings.insert(outBinding.binding);
                    }
                }
            }

            // Insert new
            for (const auto& inBinding : inSetBindings)
            {
                if (!duplicatedBindings.contains(inBinding.binding))
                {
                    outSetBindings.push_back(inBinding);
                }
            }
        }

        // Push constants (duplication of offset + size is fine as long as shader stages are not overlapping)
        for (const auto& inPushConstant : in.pushConstants)
        {
            out.pushConstants.push_back(inPushConstant);
        }
    }

    ReflectedShaderInfo reflect(const std::vector<Shader*>& shaders)
    {
        ReflectedShaderInfo globalInfo;

        for (const auto& shader : shaders)
        {
            ReflectedShaderInfo shaderInfo = reflectShader(shader);
            combine(globalInfo, shaderInfo);
        }

        return globalInfo;
    }
}
