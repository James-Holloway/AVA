#ifndef AVA_CREATION_HPP
#define AVA_CREATION_HPP

#include <string>
#include <vector>
#include "detail/vulkan.hpp"
#include "version.hpp"

namespace ava
{
    struct CreateInfo
    {
        std::string appName; // App name
        Version apiVersion{1, 3, 0}; // Vulkan API version (recommended minimum 1.2 for full functionality such as Vulkan extension usage)
        Version appVersion{1, 0, 0}; // App version
        bool debug = false; // Create a debug context with validation layers
        bool enableRayTracing = false; // Enables ray tracing if supported (query support first from ava/rayTracing.hpp) (requires at least Vulkan 1.1, at least 1.2 recommended)
        std::vector<const char*> extraLayers{}; // Extra instance layers to enable
        std::vector<const char*> extraInstanceExtensions{}; // Extra instance extensions
        std::vector<const char*> extraDeviceExtensions{}; // Extra device extensions
        vk::PhysicalDeviceFeatures deviceFeatures{}; // Set the desired physical device features
        vk::PhysicalDeviceVulkan11Features deviceVulkan11Features{}; // Set desired Vulkan 1.1 features (Requires Vulkan 1.2! (not a typo))
        vk::PhysicalDeviceVulkan12Features deviceVulkan12Features{}; // Set desired Vulkan 1.2 features (Requires Vulkan 1.2!)
        vk::PhysicalDeviceVulkan13Features deviceVulkan13Features{}; // Set desired Vulkan 1.3 features (Requires Vulkan 1.3!)
        void* physicalPNextChain = nullptr; // Requires Vulkan 1.2 due to the implementation conflicts with vk-bootstrap
        vma::AllocatorCreateFlags vmaAllocatorCreateFlags = {}; // Configure the VMA allocator with these flags. Set these if you are using features like BufferDeviceAddress
    };

    // Configure state before you create your window (also creates Vulkan Instance)
    // Any further calls to configure will destroy the state before recreating the Vulkan Instance
    void configureState(const CreateInfo& createInfo);
    // Pass your window's Vulkan surface when creating the state. Creates the main state (devices, queues, command pools, sync objects)
    void createState(vk::SurfaceKHR surface);
    void destroyState();

    // Also used for recreation of swapchain
    void createSwapchain(vk::SurfaceKHR surface, vk::Extent2D extent, vk::Format desiredFormat = vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear, vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo);
}

#endif
