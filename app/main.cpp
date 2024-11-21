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

struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;
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

        // Create VAO
        auto vao = ava::createVAO({ava::VertexAttribute::CreateVec2(), ava::VertexAttribute::CreateVec3()});

        // Create shaders for pipeline
        std::vector<ava::Shader> shaders{};
        shaders.push_back(ava::createShader("shaders/test.vert.spv", vk::ShaderStageFlagBits::eVertex));
        shaders.push_back(ava::createShader("shaders/test.frag.spv", vk::ShaderStageFlagBits::eFragment));

        // Pipeline creation
        ava::GraphicsPipelineCreationInfo graphicsPipelineCreateInfo{};
        ava::populateGraphicsPipelineCreationInfo(graphicsPipelineCreateInfo, shaders, renderPass, 0, vao, false, false);
        auto pipeline = ava::createGraphicsPipeline(graphicsPipelineCreateInfo);

        // Destroy shaders after pipeline creation
        for (auto& shader : shaders)
        {
            ava::destroyShader(shader);
        }
        shaders.clear();

        // Create VBO & IBO
        const auto vertices = std::vector<Vertex>{
            {glm::vec2{-0.5f, -0.5f}, glm::vec3{0.0f, 0.0f, 0.0f}},
            {glm::vec2{0.5f, -0.5f}, glm::vec3{1.0f, 0.0f, 0.0f}},
            {glm::vec2{-0.5f, 0.5f}, glm::vec3{0.0f, 1.0f, 0.0f}},
            {glm::vec2{0.5f, 0.5f}, glm::vec3{1.0f, 1.0f, 0.0f}},
        };
        const auto indices = std::vector<uint16_t>{
            0, 1, 2, 1, 3, 2
        };
        ava::VBO vbo = ava::createVBO(vao, vertices);
        ava::IBO ibo = ava::createIBO(indices);

        // Create descriptor pool
        auto descriptorPool = ava::createDescriptorPool(pipeline);
        auto set0 = ava::allocateDescriptorSet(descriptorPool, 0u);

        auto uboBuffer = ava::createUniformBuffer(sizeof(UBO));
        UBO uboData{};
        uboData.offset = glm::vec2(0.25f, 0.25f);
        ava::updateBuffer(uboBuffer, uboData);

        ava::bindBuffer(set0, 0, uboBuffer);

        constexpr uint32_t imageData[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFF00FFFFu};
        auto image = ava::createImage2D({2, 2}, vk::Format::eR8G8B8A8Unorm);
        ava::updateImage(image, imageData, sizeof(imageData));

        auto imageView = ava::createImageView(image);
        auto sampler = ava::createSampler(vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest);
        ava::bindImage(set0, 1, image, imageView, sampler);

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

            const auto& commandBuffer = cbRet.value();
            ava::startCommandBuffer(commandBuffer);

            // Begin render pass
            vk::ClearValue clearColor{{1.0f, 0.0f, 1.0f, 1.0f}};
            ava::beginRenderPass(commandBuffer, renderPass, framebuffer, {clearColor});
            {
                ava::bindGraphicsPipeline(commandBuffer, pipeline);
                ava::bindDescriptorSet(commandBuffer, set0);
                ava::bindVBO(commandBuffer, vbo);
                ava::bindIBO(commandBuffer, ibo);
                ava::drawIndexed(commandBuffer);
                // TODO: attachments
            }
            ava::endRenderPass(commandBuffer);

            ava::endCommandBuffer(commandBuffer);
            ava::presentFrame();
        }

        ava::deviceWaitIdle();
        ava::destroyIBO(ibo);
        ava::destroyVBO(vbo);
        ava::destroyVAO(vao);
        ava::destroySampler(sampler);
        ava::destroyImageView(imageView);
        ava::destroyImage(image);
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
