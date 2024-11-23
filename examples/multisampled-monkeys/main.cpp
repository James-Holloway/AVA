#include <framework.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include <iostream>
#include <random>
#include <tiny_obj_loader.h>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct UBO
{
    glm::mat4 mvp;
};

constexpr uint32_t MAX_MONKEYS = 16;
constexpr auto depthFormat = vk::Format::eD32Sfloat;
constexpr auto sampleCount = vk::SampleCountFlagBits::e4; // MSAA 4x

class MultisampledMonkeys final : public AvaFramework
{
public:
    MultisampledMonkeys() : AvaFramework("AVA Multisampled Monkeys")
    {
    }

    ~MultisampledMonkeys() override = default;

    ava::raii::RenderPass::Ptr renderPass;
    ava::raii::GraphicsPipeline::Ptr graphicsPipeline;
    std::vector<ava::raii::Framebuffer::Ptr> framebuffers;

    ava::raii::VBO::Ptr monkeyModel;
    uint32_t monkeyVertexCount = 0;
    ava::raii::Buffer::Ptr ubo;

    ava::raii::DescriptorPool::Ptr pool;
    std::vector<ava::raii::DescriptorSet::Ptr> monkeySets;
    std::vector<glm::mat4> monkeyStartMatrix;

    ava::raii::Image::Ptr msaaImage;
    ava::raii::ImageView::Ptr msaaImageView;

    ava::raii::Image::Ptr depthImage;
    ava::raii::ImageView::Ptr depthImageView;

    void init() override
    {
        const auto vao = ava::raii::VAO::create({ava::VertexAttribute::CreateVec3(), ava::VertexAttribute::CreateVec3(), ava::VertexAttribute::CreateVec2()}); // Vertex

        // Render pass
        {
            ava::RenderPassCreationInfo creationInfo{
                {
                    ava::createSimpleColorAttachmentInfo(surfaceFormat, true, false), // Target MSAA
                    ava::createSimpleResolveAttachmentInfo(surfaceFormat, true), // Resolve to swapchain
                    ava::createSimpleDepthAttachmentInfo(depthFormat, true), // Depth MSAA
                },
                1
            };

            // Target MSAA
            creationInfo.attachments[0].sampleCount = sampleCount;
            creationInfo.attachments[0].subpassInfos[0].attachmentType |= ava::SubPassAttachmentTypeFlagBits::eResolve; // Add resolve flag
            creationInfo.attachments[0].subpassInfos[0].resolveAttachmentIndex = 1; // Resolve to attachment 1
            // Depth MSAA
            creationInfo.attachments[2].sampleCount = sampleCount;
            renderPass = ava::raii::RenderPass::create(creationInfo);
        }

        // Graphics pipeline
        {
            const auto shaders = std::vector{
                ava::raii::Shader::create("multisampled-monkeys.slang.spv", vk::ShaderStageFlagBits::eVertex, "vertex"),
                ava::raii::Shader::create("multisampled-monkeys.slang.spv", vk::ShaderStageFlagBits::eFragment, "pixel"),
            };

            ava::GraphicsPipelineCreationInfo creationInfo{};
            ava::raii::populateGraphicsPipelineCreationInfo(creationInfo, shaders, renderPass, 0, vao, true, true);
            creationInfo.multisampling.rasterizationSamples = sampleCount;
            graphicsPipeline = ava::raii::GraphicsPipeline::create(creationInfo);
        }

        // Descriptors
        pool = ava::raii::DescriptorPool::create(graphicsPipeline);
        monkeySets = pool->allocateDescriptorSets(0, MAX_MONKEYS);

        // Monkey model
        {
            const std::string inputFile = "suzanne.obj";
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

            std::vector<Vertex> vertices;

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

            monkeyVertexCount = vertices.size();
            monkeyModel = ava::raii::VBO::create(vao, vertices);
        }

        {
            std::default_random_engine generator(std::random_device{}());
            std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
            for (uint32_t i = 0; i < MAX_MONKEYS; i++)
            {
                auto position = glm::vec3(distribution(generator), distribution(generator), distribution(generator)) * 2.0f - 1.0f; // [0,1] to [-1,1]
                position *= 4.0f; // [-4,4]
                position.z += 4.0f; // [0,8]
                glm::vec3 scale{distribution(generator) * 0.5f + 0.5f}; // [0.5,1.0]
                glm::vec3 eulerRotation{distribution(generator) * glm::pi<float>() * 2.0f, distribution(generator) * glm::pi<float>() * 2.0f, 0.0f};

                auto model = glm::translate(glm::mat4{1.0f}, position) * glm::eulerAngleYXZ(eulerRotation.y, eulerRotation.x, eulerRotation.z) * glm::scale(glm::mat4{1.0f}, scale);

                monkeyStartMatrix.push_back(model);
            }
        }

        // Monkey descriptor sets
        ubo = ava::raii::Buffer::createUniform(sizeof(UBO) * MAX_MONKEYS);
        for (uint32_t i = 0; i < MAX_MONKEYS; i++)
        {
            monkeySets[i]->bindBuffer(0, ubo, sizeof(UBO), sizeof(UBO) * i);
        }
    }

    void update() override
    {
        const auto time = static_cast<float>(glfwGetTime()) * 0.25f;

        const auto view = glm::lookAt(glm::vec3{0.0f, 0.0f, -12.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
        const auto projection = glm::perspective(glm::radians(45.0f), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.01f, 100.0f);

        std::vector<UBO> ubos(MAX_MONKEYS, UBO{});
        for (uint32_t i = 0; i < MAX_MONKEYS; i++)
        {
            const auto t = time + static_cast<float>(i) * 0.5f;
            const auto rotation = glm::eulerAngleYXZ(std::sin(t + 0.125f) * glm::pi<float>() * 2.0f, std::cos(t * 0.7025f) * glm::pi<float>() * 2.0f, 0.0f);
            const auto model = monkeyStartMatrix[i] * rotation;

            ubos[i].mvp = projection * view * model;
        }
        ubo->update(ubos);
    }

    void draw(const ava::raii::CommandBuffer::Ptr& commandBuffer, const uint32_t currentFrame, const uint32_t imageIndex) override
    {
        vk::ClearValue colorClearValue{{0.0f, 0.0f, 0.0f, 1.0f}};
        vk::ClearValue depthClearValue{{1.0f, 0u}};
        commandBuffer->beginRenderPass(renderPass, framebuffers.at(imageIndex), {colorClearValue, colorClearValue, depthClearValue});
        commandBuffer->bindGraphicsPipeline(graphicsPipeline);
        commandBuffer->bindVBO(monkeyModel);
        for (uint32_t i = 0; i < MAX_MONKEYS; i++)
        {
            commandBuffer->bindDescriptorSet(monkeySets[i]);
            commandBuffer->draw(monkeyVertexCount);
        }
        commandBuffer->endRenderPass();
    }

    void cleanup() override
    {
        msaaImageView.reset();
        msaaImage.reset();
        depthImageView.reset();
        depthImage.reset();
        renderPass.reset();
        graphicsPipeline.reset();
        framebuffers.clear();
        monkeySets.clear();
        pool.reset();
        monkeyModel.reset();
        ubo.reset();
    }

    void resize() override
    {
        AvaFramework::resize();

        const auto extent = vk::Extent3D{ava::getSwapchainExtent(), 1};
        msaaImage = ava::raii::Image::create(extent, surfaceFormat, ava::DEFAULT_IMAGE_COLOR_ATTACHMENT_USAGE_FLAGS | vk::ImageUsageFlagBits::eTransientAttachment, vk::ImageType::e2D, vk::ImageTiling::eOptimal, 1, 1, sampleCount, ava::MemoryLocation::eLazyGpu);
        msaaImageView = msaaImage->createImageView(vk::ImageAspectFlagBits::eColor);

        depthImage = ava::raii::Image::create(extent, depthFormat, ava::DEFAULT_IMAGE_DEPTH_ATTACHMENT_USAGE_FLAGS | vk::ImageUsageFlagBits::eTransientAttachment, vk::ImageType::e2D, vk::ImageTiling::eOptimal, 1, 1, sampleCount, ava::MemoryLocation::eLazyGpu);
        depthImageView = depthImage->createImageView(vk::ImageAspectFlagBits::eDepth);

        const auto swapchainImageViews = ava::raii::getSwapchainImageViews();
        framebuffers.clear();
        for (const auto& swapchainImageView : swapchainImageViews)
        {
            framebuffers.push_back(ava::raii::Framebuffer::create(renderPass, {msaaImageView, swapchainImageView, depthImageView}, {extent.width, extent.height}, 1));
        }
    }
};

int main()
{
    MultisampledMonkeys monkeys;
    monkeys.run();
    return 0;
}
