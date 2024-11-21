#include "commandBuffer.hpp"

#include "detail.hpp"
#include "state.hpp"

namespace ava::detail
{
    static CommandBufferPtr wrapCommandBuffer(const vk::CommandBuffer& commandBuffer, const vk::CommandBufferAllocateInfo& allocateInfo, const vk::QueueFlagBits queue)
    {
        auto outCommandBuffer = std::make_shared<CommandBuffer>();
        outCommandBuffer->commandBuffer = commandBuffer;
        outCommandBuffer->allocateInfo = allocateInfo;
        outCommandBuffer->primaryQueue = queue;

        switch (queue)
        {
        case vk::QueueFlagBits::eTransfer:
        case vk::QueueFlagBits::eGraphics:
            outCommandBuffer->familyQueueIndex = State.graphicsQueueFamilyIndex;
            outCommandBuffer->queueFlags = State.graphicsQueueFlags;
            break;
        case vk::QueueFlagBits::eCompute:
            outCommandBuffer->familyQueueIndex = State.computeQueueFamilyIndex;
            outCommandBuffer->queueFlags = State.computeQueueFlags;
            break;
        // ReSharper disable once CppDFAUnreachableCode
        default:
            break;
        }

        return outCommandBuffer;
    }

    std::vector<CommandBufferPtr> createGraphicsCommandBuffers(const uint32_t count, const bool secondary)
    {
        AVA_CHECK(count > 0, "Cannot create graphics command buffers with a count of 0");
        AVA_CHECK(State.graphicsCommandPool, "Cannot create graphics command buffers from a non-existent State pool");

        vk::CommandBufferAllocateInfo allocateInfo{};
        allocateInfo.commandPool = State.graphicsCommandPool;
        allocateInfo.commandBufferCount = count;
        allocateInfo.level = secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary;

        auto commandBuffers = State.device.allocateCommandBuffers(allocateInfo);
        std::vector<CommandBufferPtr> outCommandBuffers;
        outCommandBuffers.reserve(count);
        for (const auto& commandBuffer : commandBuffers)
        {
            outCommandBuffers.push_back(wrapCommandBuffer(commandBuffer, allocateInfo, vk::QueueFlagBits::eGraphics));
        }
        return outCommandBuffers;
    }

    std::vector<CommandBufferPtr> createComputeCommandBuffers(const uint32_t count, const bool secondary)
    {
        AVA_CHECK(count > 0, "Cannot create compute command buffers with a count of 0");
        AVA_CHECK(State.computeCommandPool, "Cannot create compute command buffers from a non-existent State pool");

        vk::CommandBufferAllocateInfo allocateInfo{};
        allocateInfo.commandPool = State.computeCommandPool;
        allocateInfo.commandBufferCount = count;
        allocateInfo.level = secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary;

        auto commandBuffers = State.device.allocateCommandBuffers(allocateInfo);
        std::vector<CommandBufferPtr> outCommandBuffers;
        outCommandBuffers.reserve(count);
        for (const auto& commandBuffer : commandBuffers)
        {
            outCommandBuffers.push_back(wrapCommandBuffer(commandBuffer, allocateInfo, vk::QueueFlagBits::eCompute));
        }
        return outCommandBuffers;
    }

    CommandBufferPtr getCurrentVulkanCommandBuffer()
    {
        return State.frameGraphicsCommandBuffers[State.currentFrame];
    }

    std::vector<CommandBufferPtr> getFrameGraphicsCommandBuffers()
    {
        return State.frameGraphicsCommandBuffers;
    }
}
