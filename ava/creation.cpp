#include "creation.hpp"

#include "ava.hpp"
#include "detail/commandBuffer.hpp"
#include "detail/vulkan.hpp"
#include "detail/detail.hpp"
#include "detail/state.hpp"
#include "detail/debugCallback.hpp"
#include "detail/image.hpp"

namespace ava
{
    using namespace detail;

    static void destroyState(bool destroyInstance);

    extern std::vector<const char*> rayTracingDeviceExtensions;
    extern std::vector<const char*> rayTracingPre13DeviceExtensions;
    extern std::vector<const char*> rayTracingPre12DeviceExtensions;

    static CreateInfo createInfo;

    void configureState(const CreateInfo& ci)
    {
        if (State.stateConfigured)
        {
            destroyState(true);
        }

        createInfo = ci;

        // Get system info
        const auto systemInfo = vkb::SystemInfo::get_system_info();
        AVA_CHECK(systemInfo.has_value(), "Failed to get system info: " + systemInfo.error().message());

        // Configure instance
        vkb::InstanceBuilder instanceBuilder;
        instanceBuilder
            .set_app_name(createInfo.appName.c_str())
            .set_app_version(createInfo.appVersion.major, createInfo.appVersion.minor, createInfo.appVersion.patch)
            .set_engine_name("ava")
            .set_engine_version(AVAVersion.major, AVAVersion.minor, AVAVersion.patch)
            .require_api_version(createInfo.apiVersion.major, createInfo.apiVersion.minor, createInfo.apiVersion.patch);

        State.apiVersion = createInfo.apiVersion;

        if (createInfo.debug)
        {
            instanceBuilder.set_debug_callback(&debugCallback);
            if (systemInfo->validation_layers_available)
            {
                instanceBuilder.enable_validation_layers();
            }
        }

        for (auto& layer : createInfo.extraLayers)
        {
            instanceBuilder.enable_layer(layer);
        }
        for (auto& extension : createInfo.extraInstanceExtensions)
        {
            instanceBuilder.enable_extension(extension);
        }

        // Create vkb instance
        auto ibRet = instanceBuilder.build();
        AVA_CHECK(ibRet.has_value(), "Failed to create a Vulkan instance: " + ibRet.error().message());

        State.vkbInstance = ibRet.value();
        State.instance = State.vkbInstance.instance;

        // Create dispatch loader
        auto getInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(State.instance.getProcAddr("vkGetInstanceProcAddr"));
        State.dispatchLoader = vk::detail::DispatchLoaderDynamic(State.instance, getInstanceProcAddr);

        State.stateConfigured = true;
    }

    void createState(vk::SurfaceKHR surface)
    {
        if (State.stateCreated)
        {
            destroyState(false);
        }
        State.surface = surface;

        // Configure physical device
        vkb::PhysicalDeviceSelector physicalDeviceSelector{State.vkbInstance};

        auto deviceFeatures = createInfo.deviceFeatures;

        // Ordered as 11, 12, [13], [14]
        // 11, 12 are added if Vulkan >= 1.2 [13 is added if Vulkan >= 1.3], [14 is added if Vulkan >= 1.4]

        auto physicalDeviceFeatures11 = createInfo.deviceVulkan11Features;
        auto physicalDeviceFeatures12 = createInfo.deviceVulkan12Features;
        auto physicalDeviceFeatures13 = createInfo.deviceVulkan13Features;
        auto physicalDeviceFeatures14 = createInfo.deviceVulkan14Features;
        physicalDeviceFeatures11.pNext = &physicalDeviceFeatures12;
        physicalDeviceFeatures12.pNext = createInfo.physicalPNextChain;;

        if (createInfo.apiVersion.major > 1 || createInfo.apiVersion.minor >= 3)
        {
            physicalDeviceFeatures12.pNext = &physicalDeviceFeatures13;
            physicalDeviceFeatures13.pNext = createInfo.physicalPNextChain;

            if (createInfo.apiVersion.minor >= 4)
            {
                physicalDeviceFeatures13.pNext = &physicalDeviceFeatures14;
                physicalDeviceFeatures14.pNext = createInfo.physicalPNextChain;
            }
        }

        physicalDeviceSelector
            .set_surface(surface)
            .set_minimum_version(createInfo.apiVersion.major, createInfo.apiVersion.minor)
            .add_required_extensions(createInfo.extraDeviceExtensions);

        // If developer has requested buffer device address then enable buffer device address in vma allocator create flags
        if (physicalDeviceFeatures12.bufferDeviceAddress)
        {
            // Add buffer device address to vma allocator create flags
            createInfo.vmaAllocatorCreateFlags |= vma::AllocatorCreateFlagBits::eBufferDeviceAddress;
            State.shaderDeviceAddressEnabled = true;
        }

        // Enable ray tracing features, vkb handles duplicate extension features
        if (createInfo.enableRayTracing)
        {
            AVA_CHECK(createInfo.apiVersion.major == 1 && createInfo.apiVersion.minor >= 1, "Cannot enable ray tracing when ");
            if (!State.rayTracingQueried)
            {
                AVA_WARN("Ray tracing enabled and State created without first querying ray tracing support");
            }

            deviceFeatures.shaderInt64 = true;

            vk::PhysicalDeviceRayTracingPipelineFeaturesKHR pipelineFeatures{};
            pipelineFeatures.rayTracingPipeline = true;

            vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
            accelerationStructureFeatures.accelerationStructure = true;
            accelerationStructureFeatures.descriptorBindingAccelerationStructureUpdateAfterBind = true;

            vk::PhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
            rayQueryFeatures.rayQuery = true;

            // Add physical device extension features
            physicalDeviceSelector.add_required_extension_features(pipelineFeatures);
            physicalDeviceSelector.add_required_extension_features(accelerationStructureFeatures);
            physicalDeviceSelector.add_required_extension_features(rayQueryFeatures);

            // Vulkan 1.2 or greater
            if (createInfo.apiVersion.major == 1 && createInfo.apiVersion.minor >= 2)
            {
                physicalDeviceFeatures12.bufferDeviceAddress = true;

                // Descriptor indexing features
                physicalDeviceFeatures12.descriptorIndexing = true;

                physicalDeviceFeatures12.shaderSampledImageArrayNonUniformIndexing = true;
                physicalDeviceFeatures12.shaderUniformBufferArrayNonUniformIndexing = true;
                physicalDeviceFeatures12.shaderStorageBufferArrayNonUniformIndexing = true;
                physicalDeviceFeatures12.descriptorBindingSampledImageUpdateAfterBind = true;
                physicalDeviceFeatures12.descriptorBindingUniformBufferUpdateAfterBind = true;
                physicalDeviceFeatures12.descriptorBindingStorageBufferUpdateAfterBind = true;
                physicalDeviceFeatures12.descriptorBindingStorageTexelBufferUpdateAfterBind = true;
                physicalDeviceFeatures12.descriptorBindingUniformTexelBufferUpdateAfterBind = true;
                physicalDeviceFeatures12.runtimeDescriptorArray = true;
            }
            else // Vulkan 1.1
            {
                vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressFeatures{};
                bufferDeviceAddressFeatures.bufferDeviceAddress = true;

                vk::PhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures{};
                descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = true;
                descriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing = true;
                descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing = true;
                descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = true;
                descriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = true;
                descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = true;
                descriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = true;
                descriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = true;
                descriptorIndexingFeatures.runtimeDescriptorArray = true;

                physicalDeviceSelector.add_required_extension_features(bufferDeviceAddressFeatures);
                physicalDeviceSelector.add_required_extension_features(descriptorIndexingFeatures);
            }

            // Add buffer device address to vma allocator create flags
            createInfo.vmaAllocatorCreateFlags |= vma::AllocatorCreateFlagBits::eBufferDeviceAddress;
            State.shaderDeviceAddressEnabled = true;

            // Add device extensions
            physicalDeviceSelector.add_required_extensions(rayTracingDeviceExtensions);
            if (State.apiVersion.major == 1 && State.apiVersion.minor < 2) // if version is less than Vulkan 1.2 then add pre 1.2 requirements
            {
                physicalDeviceSelector.add_required_extensions(rayTracingPre12DeviceExtensions);
            }
            if (State.apiVersion.major == 1 && State.apiVersion.minor < 3) // if version is less than Vulkan 1.3 then add pre 1.3 requirements
            {
                physicalDeviceSelector.add_required_extensions(rayTracingPre13DeviceExtensions);
            }

            // #define AVA_ALLOW_NV_RAYTRACING_VALIDATION
#ifdef AVA_ALLOW_NV_RAYTRACING_VALIDATION
            if (createInfo.debug)
            {
                physicalDeviceSelector.add_required_extension(vk::NVRayTracingValidationExtensionName);
                vk::PhysicalDeviceRayTracingValidationFeaturesNV rayTracingValidationFeatures{};
                rayTracingValidationFeatures.rayTracingValidation = true;
                physicalDeviceSelector.add_required_extension_features(rayTracingValidationFeatures);
            }
#endif

            State.rayTracingEnabled = true;
        }

        physicalDeviceSelector.set_required_features(deviceFeatures);

        // Only add the physicalDeviceFeatures11 if Vulkan >= 1.2
        // This means that physicalPNext can only be used when Vulkan 1.2 or higher is used, meaning extensions can only be used on Vulkan 1.2 and above
        // We can't use PhysicalDeviceFeatures2 because vk-bootstrap prevents us from using it
        if (createInfo.apiVersion.major == 1 && createInfo.apiVersion.minor >= 2)
        {
            physicalDeviceSelector.add_required_extension_features(physicalDeviceFeatures11);
        }

        // Create physical device
        auto pdsRet = physicalDeviceSelector.select();
        AVA_CHECK(pdsRet.has_value(), "Failed to create Vulkan physical device: " + pdsRet.error().message());

        State.vkbPhysicalDevice = pdsRet.value();
        State.physicalDevice = State.vkbPhysicalDevice.physical_device;

        // Logical device creation
        vkb::DeviceBuilder deviceBuilder{State.vkbPhysicalDevice};
        auto dbRet = deviceBuilder.build();
        AVA_CHECK(dbRet.has_value(), "Failed to create Vulkan logical device: " + dbRet.error().message());

        State.vkbDevice = dbRet.value();
        State.device = State.vkbDevice.device;

        // Create VMA Allocator
        vma::AllocatorCreateInfo allocatorCreateInfo;
        allocatorCreateInfo.vulkanApiVersion = vk::makeApiVersion(0, createInfo.apiVersion.major, createInfo.apiVersion.minor, createInfo.apiVersion.patch);
        allocatorCreateInfo.flags = createInfo.vmaAllocatorCreateFlags;
        allocatorCreateInfo.instance = State.instance;
        allocatorCreateInfo.physicalDevice = State.physicalDevice;
        allocatorCreateInfo.device = State.device;

        State.allocator = vma::createAllocator(allocatorCreateInfo);
        AVA_CHECK(State.allocator, "Failed to create VMA allocator");

        // Create queues
        State.presentQueueFamilyIndex = State.vkbDevice.get_queue_index(vkb::QueueType::present).value();
        State.presentQueue = State.vkbDevice.get_queue(vkb::QueueType::present).value();
        AVA_CHECK(State.presentQueue, "Could not create Vulkan Present Queue");

        State.graphicsQueueFamilyIndex = State.vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
        State.graphicsQueue = State.vkbDevice.get_queue(vkb::QueueType::graphics).value();
        AVA_CHECK(State.graphicsQueue, "Could not create Vulkan Graphics Queue");
        State.graphicsQueueFlags = static_cast<vk::QueueFlags>(State.vkbDevice.queue_families.at(State.graphicsQueueFamilyIndex).queueFlags);

        State.computeQueueFamilyIndex = State.vkbDevice.get_queue_index(vkb::QueueType::compute).value();
        State.computeQueue = State.vkbDevice.get_queue(vkb::QueueType::compute).value();
        AVA_CHECK(State.computeQueue, "Could not create Vulkan Compute Queue");
        State.computeQueueFlags = static_cast<vk::QueueFlags>(State.vkbDevice.queue_families.at(State.computeQueueFamilyIndex).queueFlags);

        // Create sync objects
        vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        vk::FenceCreateInfo fenceCreateInfo{vk::FenceCreateFlagBits::eSignaled};
        for (uint32_t i = 0; i < State.framesInFlight; i++)
        {
            State.imageAvailableSemaphores.push_back(State.device.createSemaphore(semaphoreCreateInfo));
            State.renderFinishedSemaphores.push_back(State.device.createSemaphore(semaphoreCreateInfo));
            State.inFlightGraphicsFences.push_back(State.device.createFence(fenceCreateInfo));
        }

        // Create command pools (graphics & compute)
        vk::CommandPoolCreateInfo graphicsPoolCreateInfo;
        graphicsPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        graphicsPoolCreateInfo.queueFamilyIndex = State.graphicsQueueFamilyIndex;
        State.graphicsCommandPool = State.device.createCommandPool(graphicsPoolCreateInfo);
        AVA_CHECK(State.graphicsCommandPool, "Failed to create Vulkan Graphics Command Pool");

        vk::CommandPoolCreateInfo computePoolCreateInfo;
        computePoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        computePoolCreateInfo.queueFamilyIndex = State.computeQueueFamilyIndex;
        State.computeCommandPool = State.device.createCommandPool(computePoolCreateInfo);
        AVA_CHECK(State.computeCommandPool, "Failed to create Vulkan Compute Command Pool");

        // Allocate frame graphic command buffers
        State.frameGraphicsCommandBuffers = createGraphicsCommandBuffers(State.framesInFlight, false);

        // Check lazily allocated memory is available
        const auto memoryProperties = State.physicalDevice.getMemoryProperties();
        for (auto& memoryType : memoryProperties.memoryTypes)
        {
            if ((memoryType.propertyFlags & vk::MemoryPropertyFlagBits::eLazilyAllocated) != vk::MemoryPropertyFlags{})
            {
                State.lazyGpuMemoryAvailable = true;
                break;
            }
        }

        if (State.rayTracingEnabled)
        {
            vk::PhysicalDeviceProperties2 deviceProperties{};
            deviceProperties.pNext = &State.accelerationStructureProperties;
            State.physicalDevice.getProperties2(&deviceProperties);

            deviceProperties.pNext = &State.rayTracingPipelineProperties;
            State.physicalDevice.getProperties2(&deviceProperties);
        }

        State.stateCreated = true;
    }

    static void destroyState(const bool destroyInstance)
    {
        // Device wait idle
        if (State.device != nullptr)
        {
            State.device.waitIdle();
        }

        // Destroy state
        {
            // Destroy sync objects
            for (auto& imageAvailableSemaphore : State.imageAvailableSemaphores)
            {
                if (imageAvailableSemaphore)
                    State.device.destroySemaphore(imageAvailableSemaphore);
            }
            State.imageAvailableSemaphores.clear();
            for (auto& renderFinishedSemaphore : State.renderFinishedSemaphores)
            {
                if (renderFinishedSemaphore)
                    State.device.destroySemaphore(renderFinishedSemaphore);
            }
            State.renderFinishedSemaphores.clear();
            for (auto& inFlightGraphicsFence : State.inFlightGraphicsFences)
            {
                if (inFlightGraphicsFence)
                    State.device.destroyFence(inFlightGraphicsFence);
            }
            State.inFlightGraphicsFences.clear();
            State.frameGraphicsCommandBuffers.clear();

            // Destroy command pools
            if (State.graphicsCommandPool)
            {
                State.device.destroyCommandPool(State.graphicsCommandPool);
            }
            if (State.computeCommandPool)
            {
                State.device.destroyCommandPool(State.computeCommandPool);
            }

            // Destroy swapchain and swapchain image views
            if (State.vkbSwapchain)
            {
                for (auto& imageView : State.swapchainImageViews)
                {
                    State.device.destroyImageView(imageView);
                    imageView = nullptr;
                }
                for (auto& avaImageView : State.swapchainAvaImageViews)
                {
                    delete avaImageView;
                    avaImageView = nullptr;
                }
                for (auto& avaImage : State.swapchainAvaImages)
                {
                    delete avaImage;
                    avaImage = nullptr;
                }

                vkb::destroy_swapchain(State.vkbSwapchain);
                State.swapchainImages.clear();
                State.swapchainImageViews.clear();
                State.swapchainImageCount = 0;
                State.swapchainExtent = vk::Extent2D{0, 0};
            }

            // Destroy VMA allocator
            if (State.allocator)
            {
                State.allocator.destroy();
                State.allocator = nullptr;
            }
            // Destroy logical device
            if (State.vkbDevice)
            {
                vkb::destroy_device(State.vkbDevice);
                State.device = nullptr;
            }

            // Remove physical device
            if (State.vkbPhysicalDevice)
            {
                State.physicalDevice = nullptr;
            }
            State.resizeNeeded = true;
            State.shaderDeviceAddressEnabled = false;
            State.rayTracingEnabled = false;
            State.stateCreated = false;
        }

        // Destroy user's surface
        if (State.instance && State.surface)
        {
            State.instance.destroySurfaceKHR(State.surface);
            State.surface = nullptr;
        }

        if (destroyInstance)
        {
            // Destroy instance
            if (State.vkbInstance)
            {
                vkb::destroy_instance(State.vkbInstance);
                State.instance = nullptr;
            }
            State.stateConfigured = false;
        }
    }

    void destroyState()
    {
        destroyState(true);
    }

    void createSwapchain(const vk::SurfaceKHR surface, const vk::Extent2D extent, const vk::Format desiredFormat, const vk::ColorSpaceKHR colorSpace, const vk::PresentModeKHR presentMode)
    {
        for (auto& imageView : State.swapchainImageViews)
        {
            State.device.destroyImageView(imageView);
            imageView = nullptr;
        }
        for (auto& avaImageView : State.swapchainAvaImageViews)
        {
            delete avaImageView;
            avaImageView = nullptr;
        }
        for (auto& avaImage : State.swapchainAvaImages)
        {
            delete avaImage;
            avaImage = nullptr;
        }

        vkb::SwapchainBuilder swapchainBuilder{State.vkbDevice, surface};
        vk::SurfaceFormatKHR surfaceFormat{desiredFormat, colorSpace};
        swapchainBuilder
            .set_desired_format(surfaceFormat)
            .set_desired_present_mode(static_cast<VkPresentModeKHR>(presentMode))
            .set_desired_extent(extent.width, extent.height);
        if (State.vkbSwapchain)
        {
            swapchainBuilder.set_old_swapchain(State.vkbSwapchain);
        }

        auto sbRet = swapchainBuilder.build();
        if (State.vkbSwapchain)
        {
            if (sbRet.has_value()) // If swapchain recreated
            {
                vkb::destroy_swapchain(State.vkbSwapchain);
            }
            else
            {
                State.vkbSwapchain.swapchain = nullptr;
            }
        }
        else
        {
            AVA_CHECK(sbRet.has_value(), "Failed to create Vulkan swapchain: " + sbRet.error().message());
        }

        State.vkbSwapchain = sbRet.value();
        State.swapchain = State.vkbSwapchain.swapchain;

        State.swapchainImageFormat = static_cast<vk::Format>(State.vkbSwapchain.image_format);
        State.swapchainExtent = State.vkbSwapchain.extent;
        State.swapchainImageCount = State.vkbSwapchain.image_count;

        // Assign the swapchain
        State.swapchainImageViews.resize(State.swapchainImageCount);
        State.swapchainImages.resize(State.swapchainImageCount);
        State.swapchainAvaImages.resize(State.swapchainImageCount);
        State.swapchainAvaImageViews.resize(State.swapchainImageCount);

        const auto images = State.vkbSwapchain.get_images().value();
        const auto imageViews = State.vkbSwapchain.get_image_views().value();
        for (size_t i = 0; i < State.swapchainImageCount; i++)
        {
            State.swapchainImages[i] = images[i];
            State.swapchainImageViews[i] = imageViews[i];

            const auto memoryRequirements = State.device.getImageMemoryRequirements(images[i]);
            State.swapchainAvaImages[i] = new detail::Image();
            State.swapchainAvaImages[i]->image = images[i];
            State.swapchainAvaImages[i]->allocationInfo.size = memoryRequirements.size;
            State.swapchainAvaImages[i]->creationInfo.extent = vk::Extent3D{State.swapchainExtent, 1};
            State.swapchainAvaImages[i]->creationInfo.format = State.swapchainImageFormat;
            State.swapchainAvaImages[i]->creationInfo.arrayLayers = 1;
            State.swapchainAvaImages[i]->creationInfo.mipLevels = 1;
            State.swapchainAvaImages[i]->creationInfo.usage = static_cast<vk::ImageUsageFlags>(State.vkbSwapchain.image_usage_flags);
            State.swapchainAvaImages[i]->imageLayout = vk::ImageLayout::eUndefined;
            State.swapchainAvaImages[i]->isSwapchainImage = true;

            State.swapchainAvaImageViews[i] = new detail::ImageView();
            State.swapchainAvaImageViews[i]->imageView = imageViews[i];
            State.swapchainAvaImageViews[i]->format = State.swapchainImageFormat;
            State.swapchainAvaImageViews[i]->isSwapchainImageView = true;
        }

        State.resizeNeeded = false;
    }
}
