#include <framework.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include <tiny_obj_loader.h>
#include <iostream>

struct Vertex
{
    glm::vec4 position;
    glm::vec4 normal;
};

struct Object
{
    vk::DeviceAddress vertexBuffer;
    vk::DeviceAddress indexBuffer;
    uint32_t indexWidth;
};

struct UBO
{
    float screenWidth;
    float screenHeight;
    int frameIndex;
    int padding;
    glm::mat4 viewProjection;
    glm::mat4 viewInverse;
    glm::mat4 projectionInverse;
};

class RayTracing : public AvaFramework
{
public:
    RayTracing() : AvaFramework("AVA Ray Tracing")
    {
    }

    ~RayTracing() override = default;

    ava::raii::BLAS::Ptr blas;
    ava::raii::BLASInstance::Ptr instance;
    ava::raii::TLAS::Ptr tlas;
    ava::raii::RayTracingPipeline::Ptr rayTracingPipeline;

    ava::raii::Buffer::Ptr objectBuffer;

    ava::raii::DescriptorPool::Ptr rayTracingDescriptorPool;
    std::vector<ava::raii::DescriptorSet::Ptr> rayTracingSet0s;

    std::vector<ava::raii::Image::Ptr> resultImages;
    std::vector<ava::raii::ImageView::Ptr> resultImageViews;
    ava::raii::Sampler::Ptr sampler;
    ava::raii::Buffer::Ptr ubo;

    ava::raii::RenderPass::Ptr finalRenderPass;
    std::vector<ava::raii::Framebuffer::Ptr> finalFramebuffers;
    ava::raii::GraphicsPipeline::Ptr finalPipeline;

    ava::raii::DescriptorPool::Ptr finalDescriptorPool;
    std::vector<ava::raii::DescriptorSet::Ptr> finalSet0s;

    void configure(ava::CreateInfo& createInfo) override
    {
        if (!ava::queryRayTracingSupport(apiVersion))
        {
            throw std::runtime_error("Ray Tracing not supported, cannot run sample");
        }

        createInfo.enableRayTracing = true;
        createInfo.deviceFeatures.shaderInt16 = true;
        createInfo.deviceVulkan11Features.variablePointers = true;
        createInfo.deviceVulkan11Features.variablePointersStorageBuffer = true;
    }

    void init() override
    {
        const auto vao = ava::raii::VAO::create({ava::VertexAttribute::CreateVec4(), ava::VertexAttribute::CreateVec4()});

        {
            const ava::RenderPassCreationInfo creationInfo{{ava::createSimpleColorAttachmentInfo(surfaceFormat, true, true)}, 1};
            finalRenderPass = ava::raii::RenderPass::create(creationInfo);
        }

        {
            const std::vector shaders{
                ava::raii::Shader::create("final.slang.spv", vk::ShaderStageFlagBits::eVertex, "vertex"),
                ava::raii::Shader::create("final.slang.spv", vk::ShaderStageFlagBits::eFragment, "fragment"),
            };
            ava::GraphicsPipelineCreationInfo creationInfo{};
            ava::raii::populateGraphicsPipelineCreationInfo(creationInfo, shaders, finalRenderPass, 0, nullptr, false, false);
            finalPipeline = ava::raii::GraphicsPipeline::create(creationInfo);
        }

        {
            const std::vector shaders{
                ava::raii::Shader::create("raytracing.slang.spv", vk::ShaderStageFlagBits::eRaygenKHR, "raygen"),
                ava::raii::Shader::create("raytracing.slang.spv", vk::ShaderStageFlagBits::eMissKHR, "miss"),
                ava::raii::Shader::create("raytracing.slang.spv", vk::ShaderStageFlagBits::eClosestHitKHR, "closestHit"),
            };
            ava::RayTracingPipelineCreationInfo creationInfo{};
            ava::raii::populateRayTracingPipelineCreationInfo(creationInfo, shaders);
            rayTracingPipeline = ava::raii::RayTracingPipeline::create(creationInfo);
        }

        {
            std::vector<Vertex> vertices;

            const std::string inputFile = "sponza_nomat.obj";
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
                        vertices.back().position = glm::vec4(vx, vy, vz, 1);

                        if (normalIndex >= 0)
                        {
                            const tinyobj::real_t nx = attrib.normals[3 * static_cast<size_t>(normalIndex) + 0];
                            const tinyobj::real_t ny = attrib.normals[3 * static_cast<size_t>(normalIndex) + 1];
                            const tinyobj::real_t nz = attrib.normals[3 * static_cast<size_t>(normalIndex) + 2];
                            vertices.back().normal = glm::vec4(nx, ny, nz, 0);
                        }
                    }
                    indexOffset += fv;
                }
            }

            const auto model = ava::raii::VBO::create(vao, vertices, 0);

            blas = ava::raii::BLAS::create(model);
            blas->rebuild();
        }

        objectBuffer = ava::raii::Buffer::create(sizeof(Object), vk::BufferUsageFlagBits::eStorageBuffer | ava::DEFAULT_TRANSFER_BUFFER_USAGE);
        Object objectBufferData{};
        objectBufferData.vertexBuffer = blas->getVertexBufferAddress();
        objectBufferData.indexBuffer = blas->getIndexBufferAddress();
        objectBufferData.indexWidth = (blas->getIndexBufferType() == vk::IndexType::eUint16) ? 16 : 32;
        objectBuffer->update(objectBufferData);

        instance = blas->createInstance(0);
        tlas = ava::raii::TLAS::create();
        tlas->rebuild({instance});

        ubo = ava::raii::Buffer::createUniform(sizeof(UBO));

        rayTracingDescriptorPool = ava::raii::DescriptorPool::create(rayTracingPipeline);
        rayTracingSet0s = rayTracingDescriptorPool->allocateDescriptorSets(0, ava::getFramesInFlight());
        for (const auto& set : rayTracingSet0s)
        {
            set->bindTLAS(0, tlas);
            set->bindBuffer(2, ubo);
            set->bindBuffer(3, objectBuffer);
        }

        finalDescriptorPool = ava::raii::DescriptorPool::create(finalPipeline);
        finalSet0s = finalDescriptorPool->allocateDescriptorSets(0, ava::getFramesInFlight());

        sampler = ava::raii::Sampler::create();
    }

    void update() override
    {
        const auto view = glm::lookAt(glm::vec3{5.5f, 2.0f, 0.0f}, glm::vec3{0.0f, 2.2f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
        auto projection = glm::perspective(glm::radians(70.0f), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.01f, 100.0f);
        projection[1][1] *= -1.0f; // Invert Y
        UBO uboData{};
        uboData.screenWidth = static_cast<float>(windowWidth);
        uboData.screenHeight = static_cast<float>(windowHeight);
        uboData.frameIndex = 0;
        uboData.viewProjection = projection * view;
        uboData.viewInverse = glm::inverse(view);
        uboData.projectionInverse = glm::inverse(projection);

        ubo->update(uboData);
    }

    void draw(const ava::raii::CommandBuffer::Ptr& commandBuffer, const uint32_t currentFrame, const uint32_t imageIndex) override
    {
        const auto& resultImage = resultImages.at(currentFrame);
        // Transition result image for writing
        commandBuffer->transitionImageLayout(resultImage, vk::ImageLayout::eGeneral);

        // Ray tracing
        {
            commandBuffer->bindRayTracingPipeline(rayTracingPipeline);
            commandBuffer->bindDescriptorSet(rayTracingSet0s.at(currentFrame));
            commandBuffer->traceRays(windowWidth, windowHeight);
        }

        // Transition result image for reading
        commandBuffer->transitionImageLayout(resultImage, vk::ImageLayout::eShaderReadOnlyOptimal);

        // Final screen
        {
            vk::ClearValue colorClearValue{{0.0f, 0.0f, 0.0f, 1.0f}};
            commandBuffer->beginRenderPass(finalRenderPass, finalFramebuffers.at(imageIndex), {colorClearValue});

            commandBuffer->bindGraphicsPipeline(finalPipeline);
            commandBuffer->bindDescriptorSet(finalSet0s.at(currentFrame));
            commandBuffer->draw(3);

            commandBuffer->endRenderPass();
        }
    }

    void cleanup() override
    {
        objectBuffer.reset();
        rayTracingSet0s.clear();
        rayTracingDescriptorPool.reset();
        finalSet0s.clear();
        finalDescriptorPool.reset();
        sampler.reset();
        resultImageViews.clear();
        resultImages.clear();
        tlas.reset();
        instance.reset();
        blas.reset();
        rayTracingPipeline.reset();
        ubo.reset();
        finalFramebuffers.clear();
        finalRenderPass.reset();
        finalPipeline.reset();
    }

    void resize() override
    {
        AvaFramework::resize();

        finalFramebuffers = ava::raii::Framebuffer::createSwapchain(finalRenderPass);

        const auto extent = ava::getSwapchainExtent();
        resultImageViews.clear();
        resultImages.clear();
        for (uint32_t i = 0; i < ava::getFramesInFlight(); i++)
        {
            resultImages.push_back(ava::raii::Image::create2D(extent, vk::Format::eR16G16B16A16Sfloat, ava::DEFAULT_IMAGE_SAMPLED_USAGE_FLAGS | ava::DEFAULT_IMAGE_STORAGE_USAGE_FLAGS));
            resultImageViews.push_back(resultImages.back()->createImageView());
            rayTracingSet0s[i]->bindImage(1, resultImages.back(), resultImageViews.back(), nullptr, vk::ImageLayout::eGeneral);
            finalSet0s[i]->bindImage(0, resultImages.back(), resultImageViews.back(), sampler, vk::ImageLayout::eShaderReadOnlyOptimal);
        }
    }
};

int main()
{
    RayTracing rayTracing;
    rayTracing.run();
    return 0;
}
