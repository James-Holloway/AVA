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
            auto renderPass = ava::raii::RenderPass::create(renderPassCreateInfo);
            auto framebuffer = ava::raii::Framebuffer::createSwapchain(renderPass);

            // Create VAO
            auto vao = ava::raii::VAO::create({ava::VertexAttribute::CreateVec2(), ava::VertexAttribute::CreateVec3()});

            // Create shaders for pipeline
            std::vector<ava::Shader> graphicsShaders{};
            graphicsShaders.push_back(ava::createShader("shaders/test.slang.spv", vk::ShaderStageFlagBits::eVertex, "vertex"));
            graphicsShaders.push_back(ava::createShader("shaders/test.slang.spv", vk::ShaderStageFlagBits::eFragment, "pixel"));

            // Pipeline creation
            ava::GraphicsPipelineCreationInfo graphicsPipelineCreateInfo{};
            ava::populateGraphicsPipelineCreationInfo(graphicsPipelineCreateInfo, graphicsShaders, *renderPass, 0, *vao, false, false);
            auto graphicsPipeline = ava::raii::GraphicsPipeline::create(graphicsPipelineCreateInfo);

            // Destroy shaders after pipeline creation
            for (auto& shader : graphicsShaders)
            {
                ava::destroyShader(shader);
            }
            graphicsShaders.clear();

            auto computeShader = ava::createShader("shaders/test_comp.slang.spv", vk::ShaderStageFlagBits::eCompute, "compute");
            auto computePipeline = ava::raii::ComputePipeline::create({computeShader});
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
            auto vbo = ava::raii::VBO::create(vao, vertices);
            auto ibo = ava::raii::IBO::create(indices);

            // Create descriptor pools
            auto graphicsDescriptorPool = ava::raii::DescriptorPool::create(graphicsPipeline);
            auto graphicsSet0 = graphicsDescriptorPool->allocateDescriptorSet(0u);

            auto computeDescriptorPool = ava::raii::DescriptorPool::create(computePipeline);
            auto computeSet0 = computeDescriptorPool->allocateDescriptorSet(0u);

            auto offsetBuffer = ava::raii::Buffer::createBuffer(sizeof(glm::vec2), ava::DEFAULT_STORAGE_BUFFER_USAGE | ava::DEFAULT_UNIFORM_BUFFER_USAGE);

            auto timeBuffer = ava::raii::Buffer::createUniformBuffer(sizeof(float));
            timeBuffer->update(static_cast<float>(glfwGetTime()));

            graphicsSet0->bindBuffer(0, offsetBuffer);
            computeSet0->bindBuffer(0, timeBuffer);
            computeSet0->bindBuffer(1, offsetBuffer);

            constexpr uint32_t imageData[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFF00FFFFu};
            auto image = ava::raii::Image::create2D({2, 2}, vk::Format::eR8G8B8A8Unorm);
            image->update(imageData, sizeof(imageData));

            auto imageView = image->createImageView();
            auto sampler = ava::raii::Sampler::create(vk::Filter::eNearest, vk::SamplerMipmapMode::eNearest);
            graphicsSet0->bindImage(1, image, imageView, sampler);

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
                    framebuffer = ava::raii::Framebuffer::createSwapchain(renderPass);
                }

                // Start a new frame
                const auto& commandBuffer = ava::raii::startFrame(&currentFrame, &imageIndex);
                if (commandBuffer == nullptr)
                {
                    continue;
                }
                // Update the compute time buffer
                timeBuffer->update(static_cast<float>(glfwGetTime()));

                commandBuffer->start();

                // Compute offset buffer
                commandBuffer->bindComputePipeline(computePipeline);
                commandBuffer->bindDescriptorSet(computeSet0);
                commandBuffer->dispatch(1, 1, 1);

                // Begin render pass
                vk::ClearValue clearColor{{1.0f, 0.0f, 1.0f, 1.0f}};
                commandBuffer->beginRenderPass(renderPass, framebuffer, {clearColor});
                {
                    commandBuffer->bindGraphicsPipeline(graphicsPipeline);
                    commandBuffer->bindDescriptorSet(graphicsSet0);
                    commandBuffer->bindVBO(vbo);
                    commandBuffer->bindIBO(ibo);
                    commandBuffer->drawIndexed();
                    // TODO: attachments
                }
                commandBuffer->endRenderPass();

                commandBuffer->end();
                ava::presentFrame();
            }

            ava::deviceWaitIdle();
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
