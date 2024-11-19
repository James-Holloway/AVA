#include "commandBuffer.hpp"

#include "detail.hpp"
#include "state.hpp"

namespace ava::detail
{
    std::vector<vk::CommandBuffer> createGraphicsCommandBuffers(const uint32_t count, const bool secondary)
    {
        AVA_CHECK(count > 0, "Cannot create graphics command buffers with a count of 0");
        AVA_CHECK(State.graphicsCommandPool, "Cannot create graphics command buffers from a non-existent State pool");

        vk::CommandBufferAllocateInfo allocateInfo{};
        allocateInfo.commandPool = State.graphicsCommandPool;
        allocateInfo.commandBufferCount = count;
        allocateInfo.level = secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary;

        return State.device.allocateCommandBuffers(allocateInfo);
    }

    std::vector<vk::CommandBuffer> createComputeCommandBuffers(const uint32_t count, const bool secondary)
    {
        AVA_CHECK(count > 0, "Cannot create compute command buffers with a count of 0");
        AVA_CHECK(State.computeCommandPool, "Cannot create compute command buffers from a non-existent State pool");

        vk::CommandBufferAllocateInfo allocateInfo{};
        allocateInfo.commandPool = State.computeCommandPool;
        allocateInfo.commandBufferCount = count;
        allocateInfo.level = secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary;

        return State.device.allocateCommandBuffers(allocateInfo);
    }
}
