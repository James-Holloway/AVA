#ifndef AVA_DETAIL_COMMANDBUFFER_HPP
#define AVA_DETAIL_COMMANDBUFFER_HPP

#include "./vulkan.hpp"
#include <memory>

namespace ava::detail
{
    struct CommandBuffer
    {
        vk::CommandBuffer commandBuffer;
        vk::CommandBufferAllocateInfo allocateInfo;
        vk::QueueFlagBits queue;

        // State
        vk::Extent2D currentRenderPassExtent;
        vk::PipelineLayout currentPipelineLayout;
        vk::PipelineBindPoint currentPipelineBindPoint;
        bool pipelineCurrentlyBound = false;
        uint32_t familyQueueIndex = ~0u;

        uint32_t lastBoundIndexBufferIndexCount = 0;
    };

    using CommandBufferPtr = std::shared_ptr<CommandBuffer>;

    std::vector<CommandBufferPtr> createGraphicsCommandBuffers(uint32_t count, bool secondary = false);
    std::vector<CommandBufferPtr> createComputeCommandBuffers(uint32_t count, bool secondary = false);

    CommandBufferPtr getCurrentVulkanCommandBuffer();
    std::vector<CommandBufferPtr> getFrameGraphicsCommandBuffers();
}

#endif
