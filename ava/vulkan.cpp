#include "./vulkan.hpp"

#include "detail/state.hpp"

namespace ava
{
    using namespace detail;

    vk::Instance getVulkanInstance()
    {
        return State.instance;
    }

    vk::PhysicalDevice getVulkanPhysicalDevice()
    {
        return State.physicalDevice;
    }

    vk::Device getVulkanDevice()
    {
        return State.device;
    }

    vk::SwapchainKHR getVulkanSwapchain()
    {
        return State.swapchain;
    }

    vk::Queue getVulkanQueue(const vk::QueueFlagBits queueType)
    {
        switch (queueType)
        {
        case vk::QueueFlagBits::eGraphics:
        case vk::QueueFlagBits::eTransfer:
            return State.graphicsQueue;
        case vk::QueueFlagBits::eCompute:
            return State.computeQueue;
        default:
            throw std::runtime_error("Unhandled queue type");
        }
    }

    vk::CommandBuffer getCurrentVulkanCommandBuffer()
    {
        return State.frameGraphicsCommandBuffers[State.currentFrame];
    }

    std::vector<vk::CommandBuffer> getFrameGraphicsCommandBuffers()
    {
        return State.frameGraphicsCommandBuffers;
    }
}
