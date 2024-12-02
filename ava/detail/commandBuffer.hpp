#ifndef AVA_DETAIL_COMMANDBUFFER_HPP
#define AVA_DETAIL_COMMANDBUFFER_HPP

#include "./vulkan.hpp"
#include "../types.hpp"
#include <memory>

namespace ava::detail
{
    struct CommandBuffer
    {
        vk::CommandBuffer commandBuffer;
        vk::CommandBufferAllocateInfo allocateInfo;
        vk::QueueFlags queueFlags;
        vk::QueueFlagBits primaryQueue;
        bool isSingleTime = false;
        bool started = false;

        // State
        vk::Extent2D currentRenderPassExtent;
        vk::PipelineLayout currentPipelineLayout;
        vk::PipelineBindPoint currentPipelineBindPoint;
        bool pipelineCurrentlyBound = false;
        uint32_t familyQueueIndex = ~0u;

        uint32_t lastBoundIndexBufferIndexCount = 0;

        // Track objects internally rather than in RAII wrapper
        std::vector<std::shared_ptr<void>> trackedObjects;
        // Ray tracing specific
        ava::RayTracingPipeline lastRayTracingPipeline;
    };

    using CommandBufferPtr = std::shared_ptr<CommandBuffer>;

    std::vector<CommandBufferPtr> createGraphicsCommandBuffers(uint32_t count, bool secondary = false);
    std::vector<CommandBufferPtr> createComputeCommandBuffers(uint32_t count, bool secondary = false);

    CommandBufferPtr getCurrentVulkanCommandBuffer();
    std::vector<CommandBufferPtr> getFrameGraphicsCommandBuffers();
}

#endif
