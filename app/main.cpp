#include <iostream>
#include <stdexcept>
#include <ava/ava.hpp>
#include <GLFW/glfw3.h>

#include "ava/ava.hpp"
#include "ava/raii.hpp"
#include <glm/glm.hpp>

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
        createInfo.apiVersion = {1, 3, 0};
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
        {
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
            std::vector<ava::Shader> graphicsShaders{};
            graphicsShaders.push_back(ava::createShader("shaders/test.slang.spv", vk::ShaderStageFlagBits::eVertex, "vertex"));
            graphicsShaders.push_back(ava::createShader("shaders/test.slang.spv", vk::ShaderStageFlagBits::eFragment, "pixel"));

            // Pipeline creation
            ava::GraphicsPipelineCreationInfo graphicsPipelineCreateInfo{};
            ava::populateGraphicsPipelineCreationInfo(graphicsPipelineCreateInfo, graphicsShaders, renderPass, 0, vao, false, false);
            auto graphicsPipeline = ava::createGraphicsPipeline(graphicsPipelineCreateInfo);

            // Destroy shaders after pipeline creation
            for (auto& shader : graphicsShaders)
            {
                ava::destroyShader(shader);
            }
            graphicsShaders.clear();

            auto computeShader = ava::createShader("shaders/test_comp.slang.spv", vk::ShaderStageFlagBits::eCompute, "compute");
            auto computePipeline = ava::createComputePipeline({computeShader});
            ava::destroyShader(computeShader);

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

            // Create descriptor pools
            auto graphicsDescriptorPool = ava::createDescriptorPool(graphicsPipeline);
            auto graphicsSet0 = ava::allocateDescriptorSet(graphicsDescriptorPool, 0u);

            auto computeDescriptorPool = ava::createDescriptorPool(computePipeline);
            auto computeSet0 = ava::allocateDescriptorSet(computeDescriptorPool, 0u);

            auto offsetBuffer = ava::raii::Buffer::createBuffer(sizeof(glm::vec2), ava::DEFAULT_STORAGE_BUFFER_USAGE | ava::DEFAULT_UNIFORM_BUFFER_USAGE);

            auto timeBuffer = ava::raii::Buffer::createUniformBuffer(sizeof(float));
            timeBuffer->update(static_cast<float>(glfwGetTime()));

            ava::bindBuffer(graphicsSet0, 0, offsetBuffer->buffer);
            ava::bindBuffer(computeSet0, 0, timeBuffer->buffer);
            ava::bindBuffer(computeSet0, 1, offsetBuffer->buffer);

            constexpr uint32_t imageData[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFF00FFFFu};
            auto image = ava::createImage2D({2, 2}, vk::Format::eR8G8B8A8Unorm);
            ava::updateImage(image, imageData, sizeof(imageData));

            auto imageView = ava::createImageView(image);
            auto sampler = ava::createSampler(vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest);
            ava::bindImage(graphicsSet0, 1, image, imageView, sampler);

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
                // Update the compute time buffer
                timeBuffer->update(static_cast<float>(glfwGetTime()));

                const auto& commandBuffer = cbRet.value();
                ava::startCommandBuffer(commandBuffer);

                // Compute offset buffer
                ava::bindComputePipeline(commandBuffer, computePipeline);
                ava::bindDescriptorSet(commandBuffer, computeSet0);
                ava::dispatch(commandBuffer, 1, 1, 1);

                // Begin render pass
                vk::ClearValue clearColor{{1.0f, 0.0f, 1.0f, 1.0f}};
                ava::beginRenderPass(commandBuffer, renderPass, framebuffer, {clearColor});
                {
                    ava::bindGraphicsPipeline(commandBuffer, graphicsPipeline);
                    ava::bindDescriptorSet(commandBuffer, graphicsSet0);
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
            ava::destroyDescriptorPool(graphicsDescriptorPool);
            ava::destroyDescriptorPool(computeDescriptorPool);
            ava::destroyComputePipeline(computePipeline);
            ava::destroyGraphicsPipeline(graphicsPipeline);
            ava::destroyFramebuffer(framebuffer);
            ava::destroyRenderPass(renderPass);
        }
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
