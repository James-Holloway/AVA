#include <framework.hpp>

template <typename T>
using Pointer = ava::raii::Pointer<T>;

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

struct UBO
{
    glm::mat4 mvp;
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

class Cube final : public AvaFramework
{
public:
    Cube() : AvaFramework("AVA Cube")
    {
    }

    ~Cube() override = default;

    Pointer<ava::raii::RenderPass> renderPass;
    Pointer<ava::raii::GraphicsPipeline> graphicsPipeline;
    std::vector<Pointer<ava::raii::Framebuffer>> framebuffers;

    Pointer<ava::raii::VBO> vbo;
    Pointer<ava::raii::IBO> ibo;
    Pointer<ava::raii::Buffer> ubo;

    Pointer<ava::raii::DescriptorPool> pool;
    Pointer<ava::raii::DescriptorSet> set0;

    void init() override
    {
        const auto vao = ava::raii::VAO::create({ava::VertexAttribute::CreateVec3(), ava::VertexAttribute::CreateVec3()}); // Vertex

        // Render pass
        {
            const ava::RenderPassCreateInfo creationInfo{
                {ava::createSimpleColorAttachmentInfo(surfaceFormat, true, true)}, 1
            };
            renderPass = ava::raii::RenderPass::create(creationInfo);
        }

        // Graphics pipeline
        {
            const auto shaders = std::vector{
                ava::raii::Shader::create("cube.slang.spv", vk::ShaderStageFlagBits::eVertex, "vertex"),
                ava::raii::Shader::create("cube.slang.spv", vk::ShaderStageFlagBits::eFragment, "pixel"),
            };

            ava::GraphicsPipelineCreationInfo creationInfo{};
            ava::raii::populateGraphicsPipelineCreationInfo(creationInfo, shaders, renderPass, 0, vao, false, false);
            graphicsPipeline = ava::raii::GraphicsPipeline::create(creationInfo);
        }

        // Descriptors
        pool = ava::raii::DescriptorPool::create(graphicsPipeline);
        set0 = pool->allocateDescriptorSet(0);

        // Cube model
        vbo = ava::raii::VBO::create(vao, cubeVertices, sizeof(cubeVertices));
        ibo = ava::raii::IBO::create(cubeIndices, std::size(cubeIndices));

        // Cube descriptor set
        ubo = ava::raii::Buffer::createUniform(sizeof(UBO));
        set0->bindBuffer(0, ubo);
    }

    void update() override
    {
        const auto time = static_cast<float>(glfwGetTime()) * 0.25f;

        UBO uboData{};
        const auto model = glm::eulerAngleYXZ(std::sin(time + 0.125f) * glm::pi<float>() * 2.0f, std::cos(time * 0.7025f) * glm::pi<float>() * 2.0f, 0.0f);
        const auto view = glm::lookAt(glm::vec3{0.0f, 0.0f, -6.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
        const auto projection = glm::perspective(glm::radians(60.0f), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.01f, 100.0f);
        uboData.mvp = projection * view * model;

        ubo->update(uboData);
    }

    void draw(const ava::raii::Pointer<ava::raii::CommandBuffer>& commandBuffer, const uint32_t currentFrame, const uint32_t imageIndex) override
    {
        vk::ClearValue clearValue{{0.0f, 0.0f, 0.0f, 1.0f}};
        commandBuffer->beginRenderPass(renderPass, framebuffers.at(imageIndex), {clearValue});
        commandBuffer->bindGraphicsPipeline(graphicsPipeline);
        commandBuffer->bindVBO(vbo);
        commandBuffer->bindIBO(ibo);
        commandBuffer->bindDescriptorSet(set0);
        commandBuffer->drawIndexed();
        commandBuffer->endRenderPass();
    }

    void cleanup() override
    {
        renderPass.reset();
        graphicsPipeline.reset();
        framebuffers.clear();
        set0.reset();
        pool.reset();
        vbo.reset();
        ibo.reset();
        ubo.reset();
    }

    void resize() override
    {
        AvaFramework::resize();
        framebuffers = ava::raii::Framebuffer::createSwapchain(renderPass);
    }
};

int main()
{
    Cube cube;
    cube.run();
    return 0;
}
