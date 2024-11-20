#include <iostream>
#include <stdexcept>
#include <ava/ava.hpp>
#include <GLFW/glfw3.h>

#include "ava/ava.hpp"
#include <glm/glm.hpp>

struct UBO
{
    glm::vec2 offset;
};

int main()
{
    try
    {
        glfwInit();

        // Configure state
        ava::CreateInfo createInfo;
        createInfo.appName = "AVA app";
        createInfo.apiVersion = {1, 2, 0};
        createInfo.debug = true;

        uint32_t extensionsCount;
        const auto extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
        for (uint32_t i = 0; i < extensionsCount; i++)
        {
            createInfo.extraInstanceExtensions.push_back(extensions[i]);
        }

        ava::configureState(createInfo);

        // Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        auto window = glfwCreateWindow(1600, 900, "AVA App", nullptr, nullptr);
        if (window == nullptr)
        {
            throw std::runtime_error("GLFW Window could not be created");
        }

        // Create state from window's surface
        VkSurfaceKHR surface;
        glfwCreateWindowSurface(ava::getVulkanInstance(), window, nullptr, &surface);
        ava::createState(surface);

        // Create swapchain
        constexpr vk::Format surfaceFormat = vk::Format::eB8G8R8A8Unorm;
        constexpr bool vsync = true;
        auto recreateSwapchain = [&] { ava::createSwapchain(surface, surfaceFormat, vk::ColorSpaceKHR::eSrgbNonlinear, vsync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate); };
        recreateSwapchain();

        // Create render pass and framebuffer
        const ava::RenderPassCreateInfo renderPassCreateInfo{
            {
                ava::RenderPassAttachmentInfo{ava::createSimpleColorAttachmentInfo(surfaceFormat, true, true),},
            },
            1
        };
        auto renderPass = ava::createRenderPass(renderPassCreateInfo);
        auto framebuffer = ava::createSwapchainFramebuffer(renderPass);

        // Create shaders for pipeline
        std::vector<ava::Shader> shaders{};
        shaders.push_back(ava::createShader("shaders/test.vert.spv", vk::ShaderStageFlagBits::eVertex));
        shaders.push_back(ava::createShader("shaders/test.frag.spv", vk::ShaderStageFlagBits::eFragment));

        // Pipeline creation
        ava::GraphicsPipelineCreationInfo graphicsPipelineCreateInfo{};
        ava::populateGraphicsPipelineCreationInfo(graphicsPipelineCreateInfo, shaders, renderPass, 0, nullptr, false, false);
        auto pipeline = ava::createGraphicsPipeline(graphicsPipelineCreateInfo);

        // Destroy shaders after pipeline creation
        for (auto& shader : shaders)
        {
            ava::destroyShader(shader);
        }
        shaders.clear();

        // Create descriptor pool
        auto descriptorPool = ava::createDescriptorPool(pipeline);
        auto uboSet = ava::allocateDescriptorSet(descriptorPool, 0u);

        auto uboBuffer = ava::createUniformBuffer(sizeof(UBO));
        UBO uboData{};
        uboData.offset = glm::vec2(0.25f, 0.25f);
        ava::updateBuffer(uboBuffer, uboData);

        ava::bindBuffer(uboSet, 0, uboBuffer);

        // Main loop
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            uint32_t currentFrame, imageIndex;

            // Recreate swapchain and framebuffer if a resize is needed
            if (ava::resizeNeeded())
            {
                ava::deviceWaitIdle();
                recreateSwapchain();
                ava::destroyFramebuffer(framebuffer);
                framebuffer = ava::createSwapchainFramebuffer(renderPass);
            }

            // Start a new frame
            auto cbRet = ava::startFrame(&currentFrame, &imageIndex);
            if (!cbRet.has_value())
            {
                continue;
            }

            uboData.offset = glm::vec2(std::sin(glfwGetTime()) * 0.5f, std::cos(glfwGetTime()) * 0.5f);
            ava::updateBuffer(uboBuffer, uboData);

            auto commandBuffer = cbRet.value();
            ava::startCommandBuffer(commandBuffer);

            // Begin render pass
            vk::ClearValue clearColor{{1.0f, 0.0f, 1.0f, 1.0f}};
            ava::beginRenderPass(commandBuffer, renderPass, framebuffer, {clearColor});
            {
                ava::bindGraphicsPipeline(commandBuffer, pipeline);
                ava::bindDescriptorSet(commandBuffer, uboSet);
                ava::draw(commandBuffer, 3);
                // TODO: attachments, images
            }
            ava::endRenderPass(commandBuffer);

            ava::endCommandBuffer(commandBuffer);
            ava::presentFrame();
        }

        ava::deviceWaitIdle();
        ava::destroyBuffer(uboBuffer);
        ava::destroyDescriptorPool(descriptorPool);
        ava::destroyGraphicsPipeline(pipeline);
        ava::destroyFramebuffer(framebuffer);
        ava::destroyRenderPass(renderPass);
        ava::destroyState();

        glfwTerminate();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
