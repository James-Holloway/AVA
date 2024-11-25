#include <framework.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <random>
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include <tiny_obj_loader.h>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct UBO
{
    glm::mat4 viewProjection;
    glm::mat4 model;
    float tessAlpha;
    float tessLevel;
};

constexpr auto depthFormat = vk::Format::eD32Sfloat;

class Tessellation : public AvaFramework
{
public:
    Tessellation() : AvaFramework("AVA Tessellation")
    {
    }

    ~Tessellation() override = default;

    ava::raii::RenderPass::Ptr renderPass;
    ava::raii::GraphicsPipeline::Ptr graphicsPipeline;
    std::vector<ava::raii::Framebuffer::Ptr> framebuffers;

    ava::raii::Buffer::Ptr ubo;
    ava::raii::VBO::Ptr model;
    uint32_t vertexCount;

    ava::raii::DescriptorPool::Ptr descriptorPool;
    ava::raii::DescriptorSet::Ptr set0;

    ava::raii::Image::Ptr depthImage;
    ava::raii::ImageView::Ptr depthImageView;

    void configure(ava::CreateInfo& createInfo) override
    {
        createInfo.deviceFeatures.tessellationShader = true;
        createInfo.deviceFeatures.fillModeNonSolid = true; // wireframe
    }

    void init() override
    {
        const auto vao = ava::raii::VAO::create({ava::VertexAttribute::CreateVec3(), ava::VertexAttribute::CreateVec3(), ava::VertexAttribute::CreateVec2()}, vk::PrimitiveTopology::ePatchList);

        // Render pass
        {
            const ava::RenderPassCreationInfo renderPassCreationInfo{
                {
                    ava::createSimpleColorAttachmentInfo(surfaceFormat, true, true, 1),
                    ava::createSimpleDepthAttachmentInfo(depthFormat, true, 1),
                },
                1
            };
            renderPass = ava::raii::RenderPass::create(renderPassCreationInfo);
        }

        // Graphics pipeline
        {
            ava::GraphicsPipelineCreationInfo creationInfo{};
            const std::vector shaders = {
                ava::raii::Shader::create("tessellation.slang.spv", vk::ShaderStageFlagBits::eVertex, "vertex"),
                ava::raii::Shader::create("tessellation.slang.spv", vk::ShaderStageFlagBits::eTessellationControl, "hull"),
                ava::raii::Shader::create("tessellation.slang.spv", vk::ShaderStageFlagBits::eTessellationEvaluation, "domain"),
                ava::raii::Shader::create("tessellation.slang.spv", vk::ShaderStageFlagBits::eFragment, "fragment"),
            };

            ava::raii::populateGraphicsPipelineCreationInfo(creationInfo, shaders, renderPass, 0, vao, true, true);
            creationInfo.rasterizer.polygonMode = vk::PolygonMode::eLine; // Wireframe
            creationInfo.tessellation.patchControlPoints = 3; // We take in 3 patch control points (triangles)
            creationInfo.depthStencil.depthCompareOp = vk::CompareOp::eGreater; // reverse Z
            graphicsPipeline = ava::raii::GraphicsPipeline::create(creationInfo);
        }

        {
            std::vector<Vertex> vertices;

            const std::string inputFile = "suzanne_smooth.obj";
            tinyobj::ObjReaderConfig readerConfig;
            readerConfig.triangulate = true;

            tinyobj::ObjReader reader;
            if (!reader.ParseFromFile(inputFile, readerConfig))
            {
                if (!reader.Error().empty())
                {
                    std::cerr << reader.Error() << std::endl;
                }
                throw std::runtime_error("Failed to parse obj file " + inputFile);
            }

            auto& attrib = reader.GetAttrib();
            auto& shapes = reader.GetShapes();

            // Loop over shapes
            for (const auto& shape : shapes)
            {
                // Loop over faces(polygon)
                size_t indexOffset = 0;
                for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
                {
                    const auto fv = static_cast<size_t>(shape.mesh.num_face_vertices[f]);

                    // Loop over vertices in the face.
                    for (size_t v = 0; v < fv; v++)
                    {
                        vertices.emplace_back();
                        const auto [vertexIndex, normalIndex, texcoordIndex] = shape.mesh.indices[indexOffset + v];
                        const tinyobj::real_t vx = attrib.vertices[3 * static_cast<size_t>(vertexIndex) + 0];
                        const tinyobj::real_t vy = attrib.vertices[3 * static_cast<size_t>(vertexIndex) + 1];
                        const tinyobj::real_t vz = attrib.vertices[3 * static_cast<size_t>(vertexIndex) + 2];
                        vertices.back().position = glm::vec3(vx, vy, vz);

                        if (normalIndex >= 0)
                        {
                            const tinyobj::real_t nx = attrib.normals[3 * static_cast<size_t>(normalIndex) + 0];
                            const tinyobj::real_t ny = attrib.normals[3 * static_cast<size_t>(normalIndex) + 1];
                            const tinyobj::real_t nz = attrib.normals[3 * static_cast<size_t>(normalIndex) + 2];
                            vertices.back().normal = glm::vec3(nx, ny, nz);
                        }

                        if (texcoordIndex >= 0)
                        {
                            const tinyobj::real_t tx = attrib.texcoords[2 * static_cast<size_t>(texcoordIndex) + 0];
                            const tinyobj::real_t ty = attrib.texcoords[2 * static_cast<size_t>(texcoordIndex) + 1];
                            vertices.back().uv = glm::vec2(tx, ty);
                        }
                    }
                    indexOffset += fv;
                }
            }

            vertexCount = vertices.size();
            model = ava::raii::VBO::create(vao, vertices, 0);
        }

        ubo = ava::raii::Buffer::createUniform(sizeof(UBO));

        descriptorPool = ava::raii::DescriptorPool::create(graphicsPipeline);
        set0 = descriptorPool->allocateDescriptorSet(0);
        set0->bindBuffer(0, ubo);
    }

    void update() override
    {
        constexpr float revolutionsPerSecond = 0.0625f;
        constexpr float tessLevelMax = 3.0f;
        constexpr float tessLevelChangeRate = 0.125f;

        const auto view = glm::lookAt(glm::vec3{0.0f, 1.0f, -4.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
        auto projection = glm::perspective(glm::radians(45.0f), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.01f, 100.0f);
        projection[1][1] *= -1.0f; // Invert Y

        const auto viewProj = projection * view;
        const auto time = static_cast<float>(glfwGetTime());
        const auto model = glm::eulerAngleYXZ(std::fmod(time * revolutionsPerSecond, 1.0f) * 2 * glm::pi<float>(), 0.0f, 0.0f);

        UBO uboData{};
        uboData.viewProjection = viewProj;
        uboData.model = model;
        uboData.tessAlpha = 1.0f;
        uboData.tessLevel = (std::sin((time) * glm::pi<float>() * tessLevelChangeRate) * 0.5f + 0.5f) * tessLevelMax;
        ubo->update(uboData);
    }

    void draw(const ava::raii::CommandBuffer::Ptr& commandBuffer, uint32_t currentFrame, uint32_t imageIndex) override
    {
        vk::ClearValue colorClearValue{{0.0f, 0.0f, 0.0f, 1.0f}};
        vk::ClearValue depthClearValue{{0.0f, 0u}};
        commandBuffer->beginRenderPass(renderPass, framebuffers.at(imageIndex), {colorClearValue, depthClearValue});

        commandBuffer->bindGraphicsPipeline(graphicsPipeline);
        commandBuffer->bindVBO(model);
        commandBuffer->bindDescriptorSet(set0);
        commandBuffer->draw(vertexCount);

        commandBuffer->endRenderPass();
    }

    void cleanup() override
    {
        depthImageView.reset();
        depthImage.reset();
        set0.reset();
        descriptorPool.reset();
        ubo.reset();
        model.reset();
        framebuffers.clear();
        graphicsPipeline.reset();
        renderPass.reset();
    }

    void resize() override
    {
        AvaFramework::resize();

        depthImage = ava::raii::Image::create2D(ava::getSwapchainExtent(), depthFormat, ava::DEFAULT_IMAGE_DEPTH_ATTACHMENT_USAGE_FLAGS);
        depthImageView = depthImage->createImageView(vk::ImageAspectFlagBits::eDepth);

        const auto swapchainImageViews = ava::raii::getSwapchainImageViews();
        framebuffers.clear();
        for (const auto& swapchainImageView : swapchainImageViews)
        {
            framebuffers.push_back(ava::raii::Framebuffer::create(renderPass, {swapchainImageView, depthImageView}, ava::getSwapchainExtent(), 1));
        }
    }
};

int main()
{
    Tessellation tesselation;
    tesselation.run();
    return 0;
}
