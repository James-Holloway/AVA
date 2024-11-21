#include "compute.hpp"

#include "detail/commandBuffer.hpp"
#include "detail/detail.hpp"
#include "detail/reflection.hpp"
#include "detail/shaders.hpp"
#include "detail/state.hpp"

namespace ava
{
    ComputePipeline createComputePipeline(const ComputePipelineCreationInfo& pipelineCreationInfo)
    {
        AVA_CHECK(detail::State.device, "Cannot create a graphics pipeline with an invalid State device");
        AVA_CHECK(pipelineCreationInfo.shader != nullptr && pipelineCreationInfo.shader->module, "Cannot create a compute pipeline with an invalid shader");
        AVA_CHECK(pipelineCreationInfo.shader->stage == vk::ShaderStageFlagBits::eCompute, "Cannot create a compute pipeline with a non-compute shader");

        const auto [layoutBindings, pushConstants] = detail::reflect({pipelineCreationInfo.shader});
        // Create descriptor set layouts
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        descriptorSetLayouts.reserve(layoutBindings.size());
        for (auto& setLayouts : layoutBindings)
        {
            vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, setLayouts};
            descriptorSetLayouts.push_back(detail::State.device.createDescriptorSetLayout(layoutInfo));
        }

        // Shader stages
        vk::PipelineShaderStageCreateInfo computeStage{vk::PipelineShaderStageCreateFlags{}, pipelineCreationInfo.shader->stage, pipelineCreationInfo.shader->module, pipelineCreationInfo.shader->entry.c_str(), nullptr, nullptr}; // TODO: Allow specialization constants

        // Pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setSetLayouts(descriptorSetLayouts);
        pipelineLayoutInfo.setPushConstantRanges(pushConstants);

        auto pipelineLayout = detail::State.device.createPipelineLayout(pipelineLayoutInfo);

        vk::ComputePipelineCreateInfo computePipelineCreateInfo{};
        computePipelineCreateInfo
            .setFlags({})
            .setStage(computeStage)
            .setLayout(pipelineLayout)
            .setBasePipelineHandle(nullptr)
            .setBasePipelineIndex(-1);

        auto pipeline = detail::State.device.createComputePipeline(nullptr, computePipelineCreateInfo);
        vk::detail::resultCheck(pipeline.result, "Failed to create a compute pipeline");

        // Create ava compute pipeline
        const auto outComputePipeline = new detail::ComputePipeline();
        outComputePipeline->layout = pipelineLayout;
        outComputePipeline->pipeline = pipeline.value;
        outComputePipeline->layoutBindings = layoutBindings;
        outComputePipeline->pushConstants = pushConstants;
        outComputePipeline->descriptorSetLayouts = descriptorSetLayouts;
        return outComputePipeline;
    }

    void destroyComputePipeline(ComputePipeline& pipeline)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(pipeline != nullptr, "Cannot destroy an invalid compute pipeline");
        AVA_CHECK_NO_EXCEPT_RETURN(detail::State.device, "Cannot destroy compute pipeline when State's device is invalid")

        if (pipeline->pipeline)
        {
            detail::State.device.destroyPipeline(pipeline->pipeline);
        }

        if (pipeline->layout)
        {
            detail::State.device.destroyPipelineLayout(pipeline->layout);
        }

        for (const auto& descriptorSetLayout : pipeline->descriptorSetLayouts)
        {
            detail::State.device.destroyDescriptorSetLayout(descriptorSetLayout);
        }

        delete pipeline;
        pipeline = nullptr;
    }

    void bindComputePipeline(const CommandBuffer& commandBuffer, const ComputePipeline& pipeline)
    {
        AVA_CHECK(pipeline != nullptr && pipeline->pipeline && pipeline->layout, "Cannot bind invalid graphics pipeline");
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot bind a pipeline to an invalid command buffer");
        AVA_CHECK((commandBuffer->queueFlags & vk::QueueFlagBits::eCompute) != vk::QueueFlags{}, "Cannot bind a compute pipeline to a non-compute capable command buffer");

        commandBuffer->commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline->pipeline);

        commandBuffer->currentPipelineLayout = pipeline->layout;
        commandBuffer->currentPipelineBindPoint = vk::PipelineBindPoint::eCompute;
        commandBuffer->pipelineCurrentlyBound = true;
    }

    void dispatch(const CommandBuffer& commandBuffer, const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot dispatch from an invalid command buffer");
        AVA_CHECK((commandBuffer->queueFlags & vk::QueueFlagBits::eCompute) != vk::QueueFlags{}, "Cannot dispatch from a non-compute capable command buffer");
        AVA_CHECK(commandBuffer->pipelineCurrentlyBound && commandBuffer->currentPipelineBindPoint == vk::PipelineBindPoint::eCompute, "Cannot dispatch when the command buffer's most recent pipeline is not a compute pipeline");

        commandBuffer->commandBuffer.dispatch(groupCountX, groupCountY, groupCountZ);
    }
}
