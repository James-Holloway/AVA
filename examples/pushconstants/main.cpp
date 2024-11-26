#include <framework.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

struct PushConstants
{
    glm::mat4 mvp;
    glm::vec4 color;
};

Vertex cubeVertices[] = {
    // Back face
    {{-1.0f, -1.0f, -1.0f}, {0, 0, -1}},
    {{1.0f, -1.0f, -1.0f}, {0, 0, -1}},
    {{-1.0f, 1.0f, -1.0f}, {0, 0, -1}},
    {{1.0f, 1.0f, -1.0f}, {0, 0, -1}},
    // Front face
    {{-1.0f, -1.0f, 1.0f}, {0, 0, 1}},
    {{1.0f, -1.0f, 1.0f}, {0, 0, 1}},
    {{-1.0f, 1.0f, 1.0f}, {0, 0, 1}},
    {{1.0f, 1.0f, 1.0f}, {0, 0, 1}},
    // Left face
    {{-1.0f, -1.0f, -1.0f}, {-1, 0, 0}},
    {{-1.0f, -1.0f, 1.0f}, {-1, 0, 0}},
    {{-1.0f, 1.0f, -1.0f}, {-1, 0, 0}},
    {{-1.0f, 1.0f, 1.0f}, {-1, 0, 0}},
    // Right face
    {{1.0f, -1.0f, -1.0f}, {1, 0, 0}},
    {{1.0f, -1.0f, 1.0f}, {1, 0, 0}},
    {{1.0f, 1.0f, -1.0f}, {1, 0, 0}},
    {{1.0f, 1.0f, 1.0f}, {1, 0, 0}},
    // Bottom face
    {{-1.0f, -1.0f, -1.0f}, {0, -1, 0}},
    {{1.0f, -1.0f, -1.0f}, {0, -1, 0}},
    {{-1.0f, -1.0f, 1.0f}, {0, -1, 0}},
    {{1.0f, -1.0f, 1.0f}, {0, -1, 0}},
    // Top face
    {{-1.0f, 1.0f, -1.0f}, {0, 1, 0}},
    {{1.0f, 1.0f, -1.0f}, {0, 1, 0}},
    {{-1.0f, 1.0f, 1.0f}, {0, 1, 0}},
    {{1.0f, 1.0f, 1.0f}, {0, 1, 0}},
};

uint16_t cubeIndices[] = {
    3, 1, 0, 0, 2, 3, // Back face
    4, 5, 7, 7, 6, 4, // Front face
    8, 9, 11, 11, 10, 8, // Left face
    15, 13, 12, 12, 14, 15, // Right face
    16, 17, 19, 19, 18, 16, // Bottom face
    23, 21, 20, 20, 22, 23, // Top face
};

class PushConstantsExample : public AvaFramework
{
public:
    PushConstantsExample() : AvaFramework("AVA Push Constants")
    {
    }

    ~PushConstantsExample() override = default;

    ava::raii::RenderPass::Ptr renderPass;
    ava::raii::GraphicsPipeline::Ptr pipeline;
    std::vector<ava::raii::Framebuffer::Ptr> framebuffers;
    ava::raii::VIBO::Ptr vibo;

    void init() override
    {
        const auto vao = ava::raii::VAO::create({ava::VertexAttribute::CreateVec3(), ava::VertexAttribute::CreateVec3()});

        {
            ava::RenderPassCreationInfo creationInfo
            {
                {ava::createSimpleColorAttachmentInfo(surfaceFormat, true, true)}, 1
            };
            renderPass = ava::raii::RenderPass::create(creationInfo);
        }

        {
            std::vector shaders{
                ava::raii::Shader::create("pushconstants.slang.spv", vk::ShaderStageFlagBits::eVertex, "vertex"),
                ava::raii::Shader::create("pushconstants.slang.spv", vk::ShaderStageFlagBits::eFragment, "fragment")
            };
            ava::GraphicsPipelineCreationInfo creationInfo;
            ava::raii::populateGraphicsPipelineCreationInfo(creationInfo, shaders, renderPass, 0, vao, false, false);
            creationInfo.rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
            pipeline = ava::raii::GraphicsPipeline::create(creationInfo);
        }

        vibo = ava::raii::VIBO::create(vao, cubeVertices, sizeof(cubeVertices), cubeIndices, std::size(cubeIndices), 0);
    }

    void draw(const ava::raii::CommandBuffer::Ptr& commandBuffer, uint32_t currentFrame, uint32_t imageIndex) override
    {
        vk::ClearValue colorClearValue = {{0.0f, 0.0f, 0.0f, 1.0f}};
        commandBuffer->beginRenderPass(renderPass, framebuffers.at(imageIndex), {colorClearValue});

        commandBuffer->bindGraphicsPipeline(pipeline);
        commandBuffer->bindVIBO(vibo);

        const auto view = glm::lookAt(glm::vec3{4.0f, 0, 8.0f}, glm::vec3{4.0f, 0, 0}, glm::vec3{0, 1, 0});
        auto projection = glm::perspective(glm::radians(45.0f), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.01f, 100.0f);
        projection[1][1] *= -1.0f;
        const auto viewProj = projection * view;

        const auto time = static_cast<float>(glfwGetTime()) * 0.5f;

        constexpr glm::vec3 colors[] = {
            glm::vec3{1.0f, 0.0f, 0.0f},
            glm::vec3{0.0f, 1.0f, 0.0f},
            glm::vec3{0.0f, 0.0f, 1.0f},
            glm::vec3{1.0f, 1.0f, 0.0f},
            glm::vec3{1.0f, 0.0f, 1.0f},
        };
        for (int i = 0; i < 5; i++)
        {
            const auto model = glm::translate(glm::mat4{1.0f}, glm::vec3{static_cast<float>(i) * 2.0f, 0.0f, 0.0f})
                * glm::eulerAngleYXZ(std::sin(time + static_cast<float>(i)), std::cos(time + static_cast<float>(i)), 0.0f)
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.5f});

            PushConstants pushConstants{};
            pushConstants.mvp = viewProj * model;
            pushConstants.color = glm::vec4{colors[i], 1.0f};

            commandBuffer->pushConstants(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, pushConstants);
            commandBuffer->drawIndexed();
        }

        commandBuffer->endRenderPass();
    }

    void cleanup() override
    {
        vibo.reset();
        pipeline.reset();
        framebuffers.clear();
        renderPass.reset();
    }

    void resize() override
    {
        AvaFramework::resize();
        framebuffers = ava::raii::Framebuffer::createSwapchain(renderPass);
    }
};


int main()
{
    PushConstantsExample pushConstants;
    pushConstants.run();
    return 0;
}
