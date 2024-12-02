#include "rayTracingPipeline.hpp"
#include "detail/rayTracingPipeline.hpp"

#include "buffer.hpp"
#include "detail/buffer.hpp"
#include "detail/commandBuffer.hpp"
#include "detail/detail.hpp"
#include "detail/reflection.hpp"
#include "detail/state.hpp"
#include "detail/utility.hpp"

namespace ava
{
    extern bool hasShaderType(const std::vector<Shader>& shaders, vk::ShaderStageFlagBits stage);
    extern std::vector<uint32_t> getShaderIndicesFromType(const std::vector<Shader>& shaders, vk::ShaderStageFlagBits stage);

    detail::ShaderBindingTable* createShaderBindingTable(vk::Pipeline pipeline, const uint32_t missShaderCount, const uint32_t hitShaderCount, const uint32_t callableShaderCount)
    {
        const uint32_t handleCount = 1 + missShaderCount + hitShaderCount + callableShaderCount;
        const vk::DeviceSize bufferSize = detail::State.rayTracingPipelineProperties.shaderGroupBaseAlignment * handleCount;
        const uint32_t handleSize = detail::State.rayTracingPipelineProperties.shaderGroupHandleSize;
        const uint32_t handleSizeAligned = ava::detail::alignUp(handleSize, detail::State.rayTracingPipelineProperties.shaderGroupHandleAlignment);

        const auto buffer = ava::createBuffer(bufferSize, vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eTransferSrc, MemoryLocation::eCpuToGpu, detail::State.rayTracingPipelineProperties.shaderGroupHandleAlignment);

        const auto bufferAddress = detail::getBufferDeviceAddress(buffer);
        const vk::StridedDeviceAddressRegionKHR rayGenRegion{bufferAddress, handleSizeAligned, handleSizeAligned * 1};

        const auto missRegionStart = detail::alignUp(rayGenRegion.size, static_cast<vk::DeviceSize>(detail::State.rayTracingPipelineProperties.shaderGroupBaseAlignment));
        const vk::StridedDeviceAddressRegionKHR missRegion{bufferAddress + missRegionStart, handleSizeAligned, handleSizeAligned * missShaderCount};

        const auto hitRegionStart = missRegionStart + detail::alignUp(missRegion.size, static_cast<vk::DeviceSize>(detail::State.rayTracingPipelineProperties.shaderGroupBaseAlignment));
        const vk::StridedDeviceAddressRegionKHR hitRegion{bufferAddress + hitRegionStart, handleSizeAligned, handleSizeAligned * hitShaderCount};

        const auto callableRegionStart = hitRegionStart + detail::alignUp(hitRegion.size, static_cast<vk::DeviceSize>(detail::State.rayTracingPipelineProperties.shaderGroupBaseAlignment));
        const vk::StridedDeviceAddressRegionKHR callableRegion{bufferAddress + callableRegionStart, handleSizeAligned, handleSizeAligned * callableShaderCount};

        const size_t dataSize = handleCount * handleSize;
        const auto handles = detail::State.device.getRayTracingShaderGroupHandlesKHR<uint8_t>(pipeline, 0u, handleCount, dataSize, detail::State.dispatchLoader);
        auto getHandle = [&](const uint32_t i) { return handles.data() + i * handleSize; };

        const auto bufferMappedStart = buffer->mapped;
        uint32_t handleIndex = 0;

        auto bufferMapped = bufferMappedStart;
        auto incrementBufferMapped = [&](const uint32_t i) { bufferMapped = reinterpret_cast<void*>(reinterpret_cast<size_t>(bufferMappedStart) + i); };

        // RayGen handles
        std::memcpy(bufferMapped, getHandle(handleIndex++), handleSize);

        // Miss handles
        bufferMapped = reinterpret_cast<void*>(reinterpret_cast<size_t>(bufferMappedStart) + missRegionStart);
        for (uint32_t i = 0; i < missShaderCount; i++)
        {
            std::memcpy(bufferMapped, getHandle(handleIndex++), handleSize);
            incrementBufferMapped(missRegion.stride);
        }

        // Hit handles
        bufferMapped = reinterpret_cast<void*>(reinterpret_cast<size_t>(bufferMappedStart) + hitRegionStart);
        for (uint32_t i = 0; i < hitShaderCount; i++)
        {
            std::memcpy(bufferMapped, getHandle(handleIndex++), handleSize);
            incrementBufferMapped(hitRegion.stride);
        }

        // Callable handles
        bufferMapped = reinterpret_cast<void*>(reinterpret_cast<size_t>(bufferMappedStart) + callableRegionStart);
        for (uint32_t i = 0; i < callableShaderCount; i++)
        {
            std::memcpy(bufferMapped, getHandle(handleIndex++), handleSize);
            incrementBufferMapped(callableRegion.stride);
        }

        const auto outShaderBindingTable = new detail::ShaderBindingTable();
        outShaderBindingTable->rayGenRegion = rayGenRegion;
        outShaderBindingTable->missRegion = missRegion;
        outShaderBindingTable->hitRegion = hitRegion;
        outShaderBindingTable->callableRegion = callableRegion;
        outShaderBindingTable->handleCount = handleCount;
        outShaderBindingTable->missShaderCount = missShaderCount;
        outShaderBindingTable->hitShaderCount = hitShaderCount;
        outShaderBindingTable->callbackShaderCount = callableRegionStart;
        outShaderBindingTable->buffer = buffer;
        return outShaderBindingTable;
    }

    void destroyShaderBindingTable(detail::ShaderBindingTable*& shaderBindingTable)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(shaderBindingTable != nullptr, "Cannot destroy invalid shader binding table");

        ava::destroyBuffer(shaderBindingTable->buffer);

        delete shaderBindingTable;
        shaderBindingTable = nullptr;
    }

    RayTracingPipeline createRayTracingPipeline(const RayTracingPipelineCreationInfo& creationInfo)
    {
        AVA_CHECK(detail::State.device, "Cannot create a ray tracing pipeline when State's device is invalid");
        AVA_CHECK(detail::State.rayTracingEnabled, "Cannot create a ray tracing pipeline when ray tracing is not enabled");
        AVA_CHECK(!creationInfo.shaders.empty(), "Cannot create ray tracing pipeline without shaders");
        const auto raygenShaderIndices = getShaderIndicesFromType(creationInfo.shaders, vk::ShaderStageFlagBits::eRaygenKHR);
        AVA_CHECK(!raygenShaderIndices.empty(), "Cannot create a ray tracing pipeline with no raygen shader");
        AVA_CHECK(raygenShaderIndices.size() == 1, "Cannot create a ray tracing pipeline with more than 1 raygen shader");
        const auto closestHitShaderIndices = getShaderIndicesFromType(creationInfo.shaders, vk::ShaderStageFlagBits::eClosestHitKHR);
        AVA_CHECK(!closestHitShaderIndices.empty(), "Cannot create a ray tracing pipeline without any closest hit shaders");
        const auto missShaderIndices = getShaderIndicesFromType(creationInfo.shaders, vk::ShaderStageFlagBits::eMissKHR);
        AVA_CHECK(!missShaderIndices.empty(), "Cannot create a ray tracing pipeline without any miss shaders");

        const auto anyHitShaderIndices = getShaderIndicesFromType(creationInfo.shaders, vk::ShaderStageFlagBits::eAnyHitKHR);
        const auto intersectionShaderIndices = getShaderIndicesFromType(creationInfo.shaders, vk::ShaderStageFlagBits::eIntersectionKHR);
        const auto callableShaderIndices = getShaderIndicesFromType(creationInfo.shaders, vk::ShaderStageFlagBits::eCallableKHR);

        auto maxRayRecursionDepth = creationInfo.maxRayRecursionDepth;
        if (maxRayRecursionDepth > detail::State.rayTracingPipelineProperties.maxRayRecursionDepth)
        {
            AVA_WARN("Ray tracing pipeline's maxRayRecursionDepth is greater than rayTracingPipelineProperties' maxRayRecursionDepth, clamping");
            maxRayRecursionDepth = detail::State.rayTracingPipelineProperties.maxRayRecursionDepth;
        }

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos{};
        shaderStageCreateInfos.reserve(creationInfo.shaders.size());
        for (auto& shader : creationInfo.shaders)
        {
            shaderStageCreateInfos.emplace_back(vk::PipelineShaderStageCreateFlags{}, shader->stage, shader->module, shader->entry.c_str(), nullptr, nullptr); // TODO: Allow specialization constants
        }

        std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups;
        shaderGroups.reserve(creationInfo.shaders.size());
        // First is the raygen
        shaderGroups.emplace_back(vk::RayTracingShaderGroupTypeKHR::eGeneral, raygenShaderIndices.at(0), vk::ShaderUnusedKHR, vk::ShaderUnusedKHR, vk::ShaderUnusedKHR);

        // Add the miss shaders
        for (auto currentShaderIndex : missShaderIndices)
        {
            shaderGroups.emplace_back(vk::RayTracingShaderGroupTypeKHR::eGeneral, currentShaderIndex, vk::ShaderUnusedKHR, vk::ShaderUnusedKHR, vk::ShaderUnusedKHR);
        }

        // Add the closest hits (and any hit & intersection if they are between this and next any hit shader)
        for (uint32_t i = 0; i < closestHitShaderIndices.size(); i++)
        {
            uint32_t currentShaderIndex = closestHitShaderIndices.at(i);
            uint32_t nextShaderIndex = ((i + 1) < closestHitShaderIndices.size()) ? closestHitShaderIndices.at(i + 1) : 0xFFFF'FFFFu;

            uint32_t anyHitShaderIndex = vk::ShaderUnusedKHR;
            // Populate any hit shader index if between this and next any hit shader
            for (auto currentAnyHitShaderIndex : anyHitShaderIndices)
            {
                if (currentAnyHitShaderIndex > currentShaderIndex && currentAnyHitShaderIndex < nextShaderIndex)
                {
                    anyHitShaderIndex = currentAnyHitShaderIndex;
                }
            }
            // Populate intersection shader index if between this and next any hit shader
            uint32_t intersectionShaderIndex = vk::ShaderUnusedKHR;
            for (auto currentIntersectionShaderIndex : intersectionShaderIndices)
            {
                if (currentIntersectionShaderIndex > currentShaderIndex && currentIntersectionShaderIndex < nextShaderIndex)
                {
                    intersectionShaderIndex = currentIntersectionShaderIndex;
                }
            }

            auto shaderType = (intersectionShaderIndex != vk::ShaderUnusedKHR) ? vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup : vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
            shaderGroups.emplace_back(shaderType, vk::ShaderUnusedKHR, currentShaderIndex, anyHitShaderIndex, intersectionShaderIndex);
        }

        // Add callable shaders
        for (auto currentShaderIndex : callableShaderIndices)
        {
            shaderGroups.emplace_back(vk::RayTracingShaderGroupTypeKHR::eGeneral, currentShaderIndex, vk::ShaderUnusedKHR, vk::ShaderUnusedKHR, vk::ShaderUnusedKHR);
        }

        const auto [layoutBindings, pushConstants] = detail::reflect(creationInfo.shaders);

        // Create descriptor set layouts
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        descriptorSetLayouts.reserve(layoutBindings.size());
        for (auto& setLayouts : layoutBindings)
        {
            vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, setLayouts};
            descriptorSetLayouts.push_back(detail::State.device.createDescriptorSetLayout(layoutInfo));
        }

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.flags = {};
        pipelineLayoutCreateInfo.setSetLayouts(descriptorSetLayouts);
        pipelineLayoutCreateInfo.setPushConstantRanges(pushConstants);

        const auto pipelineLayout = detail::State.device.createPipelineLayout(pipelineLayoutCreateInfo);

        vk::RayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo{};
        rayTracingPipelineCreateInfo.flags = {};
        rayTracingPipelineCreateInfo.layout = pipelineLayout;
        rayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth = maxRayRecursionDepth;
        rayTracingPipelineCreateInfo.basePipelineHandle = nullptr;
        rayTracingPipelineCreateInfo.basePipelineIndex = -1;
        rayTracingPipelineCreateInfo.setStages(shaderStageCreateInfos);
        rayTracingPipelineCreateInfo.setGroups(shaderGroups);
        rayTracingPipelineCreateInfo.pDynamicState = nullptr;

        auto pipeline = detail::State.device.createRayTracingPipelineKHR(nullptr, nullptr, rayTracingPipelineCreateInfo, nullptr, detail::State.dispatchLoader);
        if (pipeline.result != vk::Result::eSuccess)
        {
            detail::State.device.destroyPipelineLayout(pipelineLayout);
            vk::detail::resultCheck(pipeline.result, "Failed to create ray tracing pipeline");
        }

        auto shaderBindingTable = createShaderBindingTable(pipeline.value, missShaderIndices.size(), closestHitShaderIndices.size(), callableShaderIndices.size());

        auto outPipeline = new detail::RayTracingPipeline();
        outPipeline->layout = pipelineLayout;
        outPipeline->pipeline = pipeline.value;
        outPipeline->layoutBindings = layoutBindings;
        outPipeline->pushConstants = pushConstants;
        outPipeline->descriptorSetLayouts = descriptorSetLayouts;
        outPipeline->shaderBindingTable = shaderBindingTable;
        return outPipeline;
    }

    void destroyRayTracingPipeline(RayTracingPipeline& rayTracingPipeline)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(rayTracingPipeline != nullptr, "Cannot destroy an invalid ray tracing pipeline");

        if (rayTracingPipeline->pipeline != nullptr)
        {
            detail::State.device.destroyPipeline(rayTracingPipeline->pipeline);
        }
        if (rayTracingPipeline->layout != nullptr)
        {
            detail::State.device.destroyPipelineLayout(rayTracingPipeline->layout);
        }
        for (const auto& setLayout : rayTracingPipeline->descriptorSetLayouts)
        {
            detail::State.device.destroyDescriptorSetLayout(setLayout);
        }
        if (rayTracingPipeline->shaderBindingTable != nullptr)
        {
            destroyShaderBindingTable(rayTracingPipeline->shaderBindingTable);
        }

        delete rayTracingPipeline;
        rayTracingPipeline = nullptr;
    }

    void populateRayTracingPipelineCreationInfo(RayTracingPipelineCreationInfo& rayTracingPipelineCreationInfo, const std::vector<Shader>& shaders)
    {
        rayTracingPipelineCreationInfo.shaders = shaders;
    }

    void bindRayTracingPipeline(const CommandBuffer& commandBuffer, const RayTracingPipeline& rayTracingPipeline)
    {
        AVA_CHECK(rayTracingPipeline != nullptr, "Cannot bind an invalid ray tracing pipeline to a command buffer");
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot bind an ray tracing pipeline to an invalid command buffer");

        commandBuffer->commandBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rayTracingPipeline->pipeline);
        commandBuffer->currentPipelineLayout = rayTracingPipeline->layout;
        commandBuffer->currentPipelineBindPoint = vk::PipelineBindPoint::eRayTracingKHR;
        commandBuffer->pipelineCurrentlyBound = true;
        commandBuffer->lastRayTracingPipeline = rayTracingPipeline;
    }

    void traceRays(const CommandBuffer& commandBuffer, uint32_t width, uint32_t height, uint32_t depth)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot trace rays with an invalid command buffer");
        AVA_CHECK(commandBuffer->pipelineCurrentlyBound && commandBuffer->currentPipelineBindPoint == vk::PipelineBindPoint::eRayTracingKHR, "Cannot trace rays when a ray tracing pipeline has not been bound");
        AVA_CHECK(commandBuffer->lastRayTracingPipeline != nullptr && commandBuffer->lastRayTracingPipeline->shaderBindingTable != nullptr, "Cannot trace rays when last ray tracing pipeline is invalid")

        const auto& shaderBindingTable = commandBuffer->lastRayTracingPipeline->shaderBindingTable;
        commandBuffer->commandBuffer.traceRaysKHR(shaderBindingTable->rayGenRegion, shaderBindingTable->missRegion, shaderBindingTable->hitRegion, shaderBindingTable->callableRegion, width, height, depth, detail::State.dispatchLoader);
    }
}
