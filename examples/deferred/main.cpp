#include <framework.hpp>
#include <iostream>
#include <random>
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    int textureIndex;
};

struct UBO
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 model;
};

constexpr auto depthFormat = vk::Format::eD32Sfloat;
constexpr auto albedoFormat = vk::Format::eR16G16B16A16Sfloat;
constexpr auto normalFormat = vk::Format::eR16G16B16A16Sfloat;

class Deferred final : public AvaFramework
{
public:
    Deferred() : AvaFramework("AVA Deferred")
    {
    }

    ~Deferred() override = default;

    ava::raii::RenderPass::Ptr renderPass;
    ava::raii::GraphicsPipeline::Ptr deferredPipeline;
    ava::raii::GraphicsPipeline::Ptr finalPipeline;
    std::vector<ava::raii::Framebuffer::Ptr> framebuffers;

    ava::raii::VBO::Ptr vbo;
    uint32_t vertexCount = 0;
    ava::raii::Buffer::Ptr ubo;

    ava::raii::Sampler::Ptr sampler;
    std::vector<ava::raii::Image::Ptr> textures;
    std::vector<ava::raii::ImageView::Ptr> textureViews;
    ava::raii::Image::Ptr whitePixel;
    ava::raii::ImageView::Ptr whitePixelImageView;

    ava::raii::DescriptorPool::Ptr deferredPool;
    ava::raii::DescriptorSet::Ptr deferredSet0;

    ava::raii::DescriptorPool::Ptr finalPool;
    ava::raii::DescriptorSet::Ptr finalSet0;

    ava::raii::Image::Ptr albedoImage;
    ava::raii::ImageView::Ptr albedoImageView;

    ava::raii::Image::Ptr normalImage;
    ava::raii::ImageView::Ptr normalImageView;

    ava::raii::Image::Ptr depthImage;
    ava::raii::ImageView::Ptr depthImageView;

    void init() override
    {
        const auto vao = ava::raii::VAO::create({ava::VertexAttribute::CreateVec3(), ava::VertexAttribute::CreateVec3(), ava::VertexAttribute::CreateVec2(), ava::VertexAttribute::CreateInt()}); // Vertex

        // Render pass
        {
            ava::RenderPassCreationInfo creationInfo{
                {
                    ava::createSimpleColorAttachmentInfo(surfaceFormat, true, true, 2), // Swapchain (0, target 0)
                    ava::createSimpleDepthAttachmentInfo(depthFormat, true, 2), // Depth (attach 1)
                    ava::createSimpleColorAttachmentInfo(albedoFormat, true, false, 2), // Albedo (attach 2, target 1)
                    ava::createSimpleColorAttachmentInfo(normalFormat, true, false, 2), // Normal (attach 3, target 2)
                },
                2
            };
            // Subpass 0
            creationInfo.attachments[0].subpassInfos[0].attachmentType = ava::SubPassAttachmentTypeFlagBits::ePreserve;
            // Subpass 1
            creationInfo.attachments[1].subpassInfos[1].attachmentType = ava::SubPassAttachmentTypeFlagBits::eInputAttachment;
            creationInfo.attachments[1].subpassInfos[1].layout = vk::ImageLayout::eShaderReadOnlyOptimal;
            creationInfo.attachments[2].subpassInfos[1].attachmentType = ava::SubPassAttachmentTypeFlagBits::eInputAttachment;
            creationInfo.attachments[2].subpassInfos[1].layout = vk::ImageLayout::eShaderReadOnlyOptimal;
            creationInfo.attachments[3].subpassInfos[1].attachmentType = ava::SubPassAttachmentTypeFlagBits::eInputAttachment;
            creationInfo.attachments[3].subpassInfos[1].layout = vk::ImageLayout::eShaderReadOnlyOptimal;

            renderPass = ava::raii::RenderPass::create(creationInfo);
        }

        // Deferred graphics pipeline
        {
            const auto shaders = std::vector{
                ava::raii::Shader::create("deferred.slang.spv", vk::ShaderStageFlagBits::eVertex, "vertex"),
                ava::raii::Shader::create("deferred.slang.spv", vk::ShaderStageFlagBits::eFragment, "fragment"),
            };

            ava::GraphicsPipelineCreationInfo creationInfo{};
            ava::raii::populateGraphicsPipelineCreationInfo(creationInfo, shaders, renderPass, 0, vao, true, true);
            creationInfo.rasterizer.cullMode = vk::CullModeFlagBits::eNone;
            creationInfo.rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
            creationInfo.depthStencil.depthCompareOp = vk::CompareOp::eGreater;
            deferredPipeline = ava::raii::GraphicsPipeline::create(creationInfo);
        }

        // Final graphics pipeline
        {
            const auto shaders = std::vector{
                ava::raii::Shader::create("final.slang.spv", vk::ShaderStageFlagBits::eVertex, "vertex"),
                ava::raii::Shader::create("final.slang.spv", vk::ShaderStageFlagBits::eFragment, "fragment"),
            };

            ava::GraphicsPipelineCreationInfo creationInfo{};
            ava::raii::populateGraphicsPipelineCreationInfo(creationInfo, shaders, renderPass, 1, nullptr, false, false);
            finalPipeline = ava::raii::GraphicsPipeline::create(creationInfo);
        }

        // Descriptors
        deferredPool = ava::raii::DescriptorPool::create(deferredPipeline);
        deferredSet0 = deferredPool->allocateDescriptorSet(0);

        finalPool = ava::raii::DescriptorPool::create(finalPipeline);
        finalSet0 = finalPool->allocateDescriptorSet(0);

        // Model loading
        {
            const std::string inputFile = "apollo_11_command_module.obj";
            tinyobj::ObjReaderConfig readerConfig;
            readerConfig.triangulate = true;
            readerConfig.mtl_search_path = ".";

            tinyobj::ObjReader reader;
            if (!reader.ParseFromFile(inputFile, readerConfig))
            {
                if (!reader.Error().empty())
                {
                    std::cerr << reader.Error() << std::endl;
                }
                throw std::runtime_error("Failed to parse obj file " + inputFile);
            }

            if (!reader.Warning().empty())
            {
                std::cerr << reader.Warning() << std::endl;
            }

            auto& attrib = reader.GetAttrib();
            auto& shapes = reader.GetShapes();
            auto& materials = reader.GetMaterials();

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
                        const auto materialIndex = shape.mesh.material_ids[f];
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

                        vertices.back().textureIndex = materialIndex;
                    }
                    indexOffset += fv;
                }
            }

            vertexCount = vertices.size();
            vbo = ava::raii::VBO::create(vao, vertices);

            // Load material textures
            for (const auto& material : materials)
            {
                if (material.diffuse_texname.empty())
                {
                    textures.push_back(nullptr);
                    textureViews.push_back(nullptr);
                    continue;
                }

                int width, height, channels;
                const auto pixels = stbi_load(material.diffuse_texname.c_str(), &width, &height, &channels, STBI_rgb_alpha);
                if (pixels == nullptr)
                {
                    std::cerr << "Failed to load texture " << material.diffuse_texname << std::endl;
                    continue;
                }
                const auto size = width * height * STBI_rgb_alpha * sizeof(stbi_uc);
                constexpr auto textureFormat = vk::Format::eR8G8B8A8Unorm;
                textures.push_back(ava::raii::Image::create2D({static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, textureFormat));
                textures.back()->update(pixels, size, vk::ImageAspectFlagBits::eColor);
                textureViews.push_back(textures.back()->createImageView(vk::ImageAspectFlagBits::eColor));
                stbi_image_free(pixels);
            }
        }

        // Sampler
        sampler = ava::raii::Sampler::create(vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear);

        // White pixel
        whitePixel = ava::raii::Image::create2D({1, 1}, vk::Format::eR8G8B8A8Unorm);
        whitePixel->update(std::vector{0xFFFFFFFFu});
        whitePixelImageView = whitePixel->createImageView(vk::ImageAspectFlagBits::eColor);

        // Descriptor sets
        ubo = ava::raii::Buffer::createUniform(sizeof(UBO));
        deferredSet0->bindBuffer(0, ubo);
        for (uint32_t i = 0; i < textures.size(); i++)
        {
            if (textures[i] != nullptr)
            {
                deferredSet0->bindImage(1, textures[i], textureViews[i], sampler, i);
            }
            else
            {
                deferredSet0->bindImage(1, whitePixel, whitePixelImageView, sampler, i);
            }
        }
        finalSet0->bindBuffer(0, ubo);
    }

    void update() override
    {
        const auto view = glm::lookAt(glm::vec3{0.0f, -2.0f, -16.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
        auto projection = glm::perspective(glm::radians(25.0f), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.01f, 100.0f);
        projection[1][1] *= -1.0f; // Invert Y

        const auto time = static_cast<float>(glfwGetTime());
        const auto model = glm::eulerAngleYXZ(std::fmod(time, glm::pi<float>() * 2.0f), 0.0f, 0.0f);

        UBO uboData{};
        uboData.view = view;
        uboData.projection = projection;
        uboData.model = model;
        ubo->update(uboData);
    }

    void draw(const ava::raii::CommandBuffer::Ptr& commandBuffer, const uint32_t currentFrame, const uint32_t imageIndex) override
    {
        vk::ClearValue colorClearValue{{0.0f, 0.0f, 0.0f, 1.0f}};
        vk::ClearValue depthClearValue{{0.0f, 0u}};
        commandBuffer->beginRenderPass(renderPass, framebuffers.at(imageIndex), {colorClearValue, depthClearValue, colorClearValue, colorClearValue});
        // Deferred pass
        {
            commandBuffer->bindGraphicsPipeline(deferredPipeline);
            commandBuffer->bindVBO(vbo);
            commandBuffer->bindDescriptorSet(deferredSet0);
            commandBuffer->draw(vertexCount);
        }

        commandBuffer->nextSubpass();

        // Final pass
        {
            commandBuffer->bindGraphicsPipeline(finalPipeline);
            commandBuffer->bindDescriptorSet(finalSet0);
            commandBuffer->draw(3);
        }

        commandBuffer->endRenderPass();
    }

    void cleanup() override
    {
        whitePixelImageView.reset();
        whitePixel.reset();
        sampler.reset();
        textureViews.clear();
        textures.clear();
        normalImageView.reset();
        normalImage.reset();
        albedoImageView.reset();
        albedoImage.reset();
        depthImageView.reset();
        depthImage.reset();
        renderPass.reset();
        finalPipeline.reset();
        deferredPipeline.reset();
        framebuffers.clear();
        finalSet0.reset();
        finalPool.reset();
        deferredSet0.reset();
        deferredPool.reset();
        vbo.reset();
        ubo.reset();
    }

    void resize() override
    {
        AvaFramework::resize();

        const auto extent = ava::getSwapchainExtent();

        const auto commandBuffer = ava::raii::CommandBuffer::beginSingleTime(vk::QueueFlagBits::eTransfer);

        depthImage = ava::raii::Image::create2D(extent, depthFormat, ava::DEFAULT_IMAGE_DEPTH_ATTACHMENT_USAGE_FLAGS | vk::ImageUsageFlagBits::eInputAttachment);
        depthImageView = depthImage->createImageView(vk::ImageAspectFlagBits::eDepth);

        commandBuffer->transitionImageLayout(depthImage, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eDepth);

        albedoImage = ava::raii::Image::create2D(extent, albedoFormat, ava::DEFAULT_IMAGE_COLOR_ATTACHMENT_USAGE_FLAGS | vk::ImageUsageFlagBits::eInputAttachment);
        albedoImageView = albedoImage->createImageView(vk::ImageAspectFlagBits::eColor);

        commandBuffer->transitionImageLayout(albedoImage, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);

        normalImage = ava::raii::Image::create2D(extent, normalFormat, ava::DEFAULT_IMAGE_COLOR_ATTACHMENT_USAGE_FLAGS | vk::ImageUsageFlagBits::eInputAttachment);
        normalImageView = normalImage->createImageView(vk::ImageAspectFlagBits::eColor);

        commandBuffer->transitionImageLayout(normalImage, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);

        const auto swapchainImageViews = ava::raii::getSwapchainImageViews();
        framebuffers.clear();
        for (const auto& swapchainImageView : swapchainImageViews)
        {
            framebuffers.push_back(ava::raii::Framebuffer::create(renderPass, {swapchainImageView, depthImageView, albedoImageView, normalImageView}, extent, 1));
        }

        commandBuffer->endSingleTime();

        finalSet0->bindImage(1, depthImage, depthImageView, sampler);
        finalSet0->bindImage(2, albedoImage, albedoImageView, sampler);
        finalSet0->bindImage(3, normalImage, normalImageView, sampler);
    }
};

int main()
{
    Deferred monkeys;
    monkeys.run();
    return 0;
}
