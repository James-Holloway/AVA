#ifndef AVA_EXAMPLES_FRAMEWORK_HPP
#define AVA_EXAMPLES_FRAMEWORK_HPP

#include <ava/ava.hpp>
#include <ava/raii.hpp>
#include <GLFW/glfw3.h>
#define GLM_FORCE_XYZW_ONLY
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

class AvaFramework
{
public:
    explicit AvaFramework(const std::string& appTitle = "AVA Framework", const uint32_t windowWidth = 1600, const uint32_t windowHeight = 900, const ava::Version apiVersion = {1, 3, 0}): windowWidth(windowWidth), windowHeight(windowHeight), appTitle(appTitle), apiVersion(apiVersion)
    {
        glfwInit();
    }

    virtual ~AvaFramework()
    {
        glfwTerminate();
    }

    GLFWwindow* window = nullptr;
    uint32_t windowWidth = 1600;
    uint32_t windowHeight = 900;
    std::string appTitle = "AVA Framework";
    ava::Version apiVersion{1, 3, 0};
    VkSurfaceKHR surface = nullptr;
    bool vsync = true;
    vk::Format surfaceFormat = vk::Format::eB8G8R8A8Unorm;

    void run()
    {
        glfwInit();

        // Configure state
        ava::CreateInfo createInfo;
        createInfo.appName = appTitle;
        createInfo.apiVersion = apiVersion;
#ifndef NDEBUG
        createInfo.debug = true;
#endif

        uint32_t extensionsCount;
        const auto extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
        for (uint32_t i = 0; i < extensionsCount; i++)
        {
            createInfo.extraInstanceExtensions.push_back(extensions[i]);
        }

        configure(createInfo);

        ava::configureState(createInfo);

        // Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(static_cast<int>(windowWidth), static_cast<int>(windowHeight), appTitle.c_str(), nullptr, nullptr);
        if (window == nullptr)
        {
            throw std::runtime_error("GLFW Window could not be created");
        }

        // Create state from window's surface
        glfwCreateWindowSurface(ava::getVulkanInstance(), window, nullptr, &surface);
        ava::createState(surface);

        // Initialize any user variables
        init();

        // Call a resize to create swapchain and any size-based user variables
        resize();

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            // Run an update function
            update();

            uint32_t currentFrame, imageIndex;
            // Recreate swapchain and framebuffer if a resize is needed
            if (ava::resizeNeeded())
            {
                ava::deviceWaitIdle();
                resize();
            }

            // Start a new frame
            const auto& commandBuffer = ava::raii::startFrame(&currentFrame, &imageIndex);
            if (commandBuffer == nullptr)
            {
                continue;
            }

            commandBuffer->start();

            draw(commandBuffer, currentFrame, imageIndex);

            commandBuffer->end();
            ava::presentFrame();
        }

        ava::deviceWaitIdle();

        cleanup();
        ava::destroyState();
    }

    virtual void configure(ava::CreateInfo& createInfo)
    {
    }

    virtual void init()
    {
    }

    virtual void draw(const ava::raii::CommandBuffer::Ptr& commandBuffer, uint32_t currentFrame, uint32_t imageIndex)
    {
    }

    virtual void update()
    {
    }

    virtual void cleanup()
    {
    }

    virtual void resize()
    {
        recreateSwapchain();
        const auto swapchainExtent = ava::getSwapchainExtent();
        windowWidth = swapchainExtent.width;
        windowHeight = swapchainExtent.height;
    }

    virtual void recreateSwapchain()
    {
        ava::createSwapchain(surface, surfaceFormat, vk::ColorSpaceKHR::eSrgbNonlinear, vsync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate);
    }
};

#endif
