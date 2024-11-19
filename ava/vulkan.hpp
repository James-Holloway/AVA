#ifndef AVA_VULKAN_HPP
#define AVA_VULKAN_HPP

// Optional header which exposes the Vulkan parts of the State

#include "detail/vulkan.hpp"

namespace ava
{
    vk::Instance getVulkanInstance();
    vk::PhysicalDevice getVulkanPhysicalDevice();
    vk::Device getVulkanDevice();
    vk::SwapchainKHR getVulkanSwapchain();
    vk::Queue getVulkanQueue(vk::QueueFlagBits queueType);
    vk::CommandBuffer getCurrentVulkanCommandBuffer();
    std::vector<vk::CommandBuffer> getFrameGraphicsCommandBuffers();
}

#endif
