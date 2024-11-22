#ifndef AVA_DETAIL_STATE_HPP
#define AVA_DETAIL_STATE_HPP

#include "./vulkan.hpp"
#include "../version.hpp"
#include "../types.hpp"
#include <atomic>
#include <memory>

namespace ava::detail
{
    struct CommandBuffer;

    struct State
    {
        bool stateConfigured = false;
        bool stateCreated = false;

        vkb::Instance vkbInstance;
        vk::Instance instance;

        Version apiVersion;

        vkb::PhysicalDevice vkbPhysicalDevice;
        vk::PhysicalDevice physicalDevice;

        vkb::Device vkbDevice;
        vk::Device device;

        vkb::Swapchain vkbSwapchain;
        vk::SwapchainKHR swapchain;
        std::vector<vk::Image> swapchainImages;
        std::vector<vk::ImageView> swapchainImageViews;
        vk::Format swapchainImageFormat = vk::Format::eUndefined;
        vk::Extent2D swapchainExtent;
        uint32_t swapchainImageCount = 0;

        std::vector<ava::Image> swapchainAvaImages;
        std::vector<ava::ImageView> swapchainAvaImageViews;

        vk::SurfaceKHR surface; // User's surface - we still destroy it when State is destroyed

        vma::Allocator allocator;

        vk::DispatchLoaderDynamic dispatchLoader;

        uint32_t framesInFlight = 2;
        uint32_t currentFrame = 0;
        uint32_t imageIndex = 0;
        bool resizeNeeded = true;
        bool frameStarted = false;

        uint32_t presentQueueFamilyIndex = ~0u;
        vk::Queue presentQueue;
        uint32_t graphicsQueueFamilyIndex = ~0u;
        vk::Queue graphicsQueue;
        vk::QueueFlags graphicsQueueFlags;
        uint32_t computeQueueFamilyIndex = ~0u;
        vk::Queue computeQueue;
        vk::QueueFlags computeQueueFlags;

        vk::CommandPool graphicsCommandPool;
        vk::CommandPool computeCommandPool;

        std::vector<std::shared_ptr<CommandBuffer>> frameGraphicsCommandBuffers;

        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> inFlightGraphicsFences;

        std::atomic<uint32_t> descriptorPoolIndexCounter = 0;
    };

    inline State State;
}

#endif
