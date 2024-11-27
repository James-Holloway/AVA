#include <framework.hpp>
#include <iostream>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

struct UBO
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 model;
    glm::vec4 lightPosition;
};

Vertex planeVertices[] = {
    {{-1.0f, 0.0f, -1.0f}, {0, 1, 0}},
    {{1.0f, 0.0f, -1.0f}, {0, 1, 0}},
    {{-1.0f, 0.0f, 1.0f}, {0, 1, 0}},
    {{1.0f, 0.0f, 1.0f}, {0, 1, 0}},
};

uint16_t planeIndices[] = {
    3, 1, 0, 0, 2, 3
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

constexpr auto depthFormat = vk::Format::eD16Unorm;
constexpr auto cubeCount = 7;
constexpr auto TAU = glm::pi<float>() * 2.0f;

class RayQuery : public AvaFramework
{
public:
    RayQuery() : AvaFramework("AVA Ray Query")
    {
        apiVersion = {1, 2, 0};
    }

    ~RayQuery() override = default;

    ava::raii::RenderPass::Ptr renderPass;
    ava::raii::GraphicsPipeline::Ptr graphicsPipeline;
    std::vector<ava::raii::Framebuffer::Ptr> framebuffers;

    ava::raii::VIBO::Ptr planeModel;
    ava::raii::VIBO::Ptr cubeModel;

    ava::raii::BLAS::Ptr planeBLAS;
    ava::raii::BLAS::Ptr cubeBLAS;
    ava::raii::BLASInstance::Ptr planeInstance;
    std::vector<ava::raii::BLASInstance::Ptr> cubeInstances;
    std::vector<ava::raii::TLAS::Ptr> tlases;

    ava::raii::Buffer::Ptr planeUBO;
    std::vector<ava::raii::Buffer::Ptr> cubeUBOs;

    ava::raii::DescriptorPool::Ptr descriptorPool;
    ava::raii::DescriptorSet::Ptr planeSet;
    std::vector<ava::raii::DescriptorSet::Ptr> cubeSets;

    ava::raii::Image::Ptr depthImage;
    ava::raii::ImageView::Ptr depthImageView;

    void configure(ava::CreateInfo& createInfo) override
    {
        if (!ava::queryRayTracingSupport(apiVersion))
        {
            throw std::runtime_error("Ray Tracing not supported, cannot run sample");
        }

        createInfo.enableRayTracing = true;
    }

    void init() override
    {
        const auto vao = ava::raii::VAO::create({ava::VertexAttribute::CreateVec3(), ava::VertexAttribute::CreateVec3()});

        {
            const ava::RenderPassCreationInfo creationInfo{
                {
                    ava::createSimpleColorAttachmentInfo(surfaceFormat, true, true, 1),
                    ava::createSimpleDepthAttachmentInfo(depthFormat, true, 1),
                },
                1
            };
            renderPass = ava::raii::RenderPass::create(creationInfo);
        }

        {
            const std::vector shaders{
                ava::raii::Shader::create("rayquery.slang.spv", vk::ShaderStageFlagBits::eVertex, "vertex"),
                ava::raii::Shader::create("rayquery.slang.spv", vk::ShaderStageFlagBits::eFragment, "fragment"),
            };
            ava::GraphicsPipelineCreationInfo creationInfo{};
            ava::raii::populateGraphicsPipelineCreationInfo(creationInfo, shaders, renderPass, 0, vao, true, true);
            creationInfo.depthStencil.depthCompareOp = vk::CompareOp::eLess; // Reverse Z
            creationInfo.rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
            graphicsPipeline = ava::raii::GraphicsPipeline::create(creationInfo);
        }

        {
            planeUBO = ava::raii::Buffer::createUniform(sizeof(UBO));
            for (int i = 0; i < cubeCount; i++)
            {
                cubeUBOs.push_back(ava::raii::Buffer::createUniform(sizeof(UBO)));
            }
        }

        {
            descriptorPool = ava::raii::DescriptorPool::create(graphicsPipeline, (cubeCount + 1) * 8);
            planeSet = descriptorPool->allocateDescriptorSet(0);
            planeSet->bindBuffer(0, planeUBO);
            cubeSets = descriptorPool->allocateDescriptorSets(0, cubeCount);
            for (int i = 0; i < cubeCount; i++)
            {
                cubeSets[i]->bindBuffer(0, cubeUBOs[i]);
            }
        }

        {
            planeModel = ava::raii::VIBO::create(vao, planeVertices, sizeof(planeVertices), planeIndices, std::size(planeIndices));
            cubeModel = ava::raii::VIBO::create(vao, cubeVertices, sizeof(cubeVertices), cubeIndices, std::size(cubeIndices));

            planeBLAS = ava::raii::BLAS::create(planeModel);
            cubeBLAS = ava::raii::BLAS::create(cubeModel);
            planeBLAS->rebuild();
            cubeBLAS->rebuild();

            planeInstance = planeBLAS->createInstance();
            cubeInstances.reserve(cubeCount);
            for (int i = 0; i < cubeCount; i++)
            {
                cubeInstances.push_back(cubeBLAS->createInstance());
            }

            auto blasInstances = cubeInstances;
            blasInstances.push_back(planeInstance);
            for (int i = 0; i < ava::getFramesInFlight(); i++)
            {
                tlases.push_back(ava::raii::TLAS::create());
                tlases.back()->rebuild(blasInstances);
            }
        }
    }

    void update() override
    {
        const auto view = glm::lookAt(glm::vec3{0.0f, 32.0f, -40.0f}, glm::vec3{0.0f, 2.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f});
        auto projection = glm::perspective(glm::radians(60.0f), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.01f, 100.0f);
        projection[1][1] *= -1;

        const auto time = static_cast<float>(glfwGetTime());
        constexpr auto lightDistance = 18.0f;
        constexpr auto lightSpeed = 0.125f;

        const glm::vec4 lightPosition{std::sin(time * TAU * lightSpeed) * lightDistance, 22.0f, std::cos(time * TAU * lightSpeed) * lightDistance, 1.0f};

        UBO planeUBOData{};
        planeUBOData.view = view;
        planeUBOData.projection = projection;
        planeUBOData.model = glm::scale(glm::mat4{1.0f}, glm::vec3{64.0f});
        planeUBOData.lightPosition = lightPosition;
        planeUBO->update(planeUBOData);
        planeInstance->updateTransformMatrix(ava::getTransformMatrix44(glm::value_ptr(glm::transpose(planeUBOData.model))));

        for (int i = 0; i < cubeCount; i++)
        {
            constexpr auto rotationsPerSecond = 1.0f / 5.0f;
            constexpr auto cubeDistance = 16.0f;
            const auto percent = static_cast<float>(i) / static_cast<float>(cubeCount);
            auto model = glm::translate(glm::mat4{1.0f}, glm::vec3{std::sin(percent * TAU) * cubeDistance, 16.0f, std::cos(percent * TAU) * cubeDistance});
            model = model * glm::eulerAngleYXZ(percent * TAU, std::fmod(time * rotationsPerSecond, 1.0f) * TAU, 0.0f);

            UBO uboData{};
            uboData.view = view;
            uboData.projection = projection;
            uboData.model = model;
            uboData.lightPosition = lightPosition;

            cubeUBOs[i]->update(uboData);

            cubeInstances[i]->updateTransformMatrix(ava::getTransformMatrix44(glm::value_ptr(glm::transpose(model))));
        }

        auto blasInstances = cubeInstances;
        blasInstances.push_back(planeInstance);

        const auto currentFrame = ava::getCurrentFrame();
        if (!tlases[currentFrame]->update(blasInstances))
        {
            std::cerr << "Failed to update TLAS, rebuilding\n";
            tlases[currentFrame]->rebuild(blasInstances);
        }

        for (int i = 0; i < cubeCount; i++)
        {
            cubeSets[i]->bindTLAS(1, tlases[currentFrame]);
        }
        planeSet->bindTLAS(1, tlases[currentFrame]);
    }

    void draw(const ava::raii::CommandBuffer::Ptr& commandBuffer, const uint32_t currentFrame, const uint32_t imageIndex) override
    {
        vk::ClearValue colorClearValue{{0.0f, 0.0f, 0.0f, 1.0f}};
        vk::ClearValue depthClearValue{{1.0f, 0u}};
        commandBuffer->beginRenderPass(renderPass, framebuffers.at(imageIndex), {colorClearValue, depthClearValue});

        commandBuffer->bindGraphicsPipeline(graphicsPipeline);
        commandBuffer->bindVIBO(planeModel);
        commandBuffer->bindDescriptorSet(planeSet);
        commandBuffer->drawIndexed();

        commandBuffer->bindVIBO(cubeModel);
        for (int i = 0; i < cubeCount; i++)
        {
            commandBuffer->bindDescriptorSet(cubeSets[i]);
            commandBuffer->drawIndexed();
        }

        commandBuffer->endRenderPass();
    }

    void cleanup() override
    {
        tlases.clear();
        cubeInstances.clear();
        planeInstance.reset();
        cubeBLAS.reset();
        planeBLAS.reset();

        cubeSets.clear();
        planeSet.reset();
        descriptorPool.reset();
        planeUBO.reset();
        cubeUBOs.clear();
        planeModel.reset();
        cubeModel.reset();
        depthImageView.reset();
        depthImage.reset();
        renderPass.reset();
        graphicsPipeline.reset();
        framebuffers.clear();
    }

    void resize() override
    {
        AvaFramework::resize();
        const auto extent = ava::getSwapchainExtent();

        depthImage = ava::raii::Image::create2D(extent, depthFormat, ava::DEFAULT_IMAGE_DEPTH_ATTACHMENT_USAGE_FLAGS);
        depthImageView = depthImage->createImageView(vk::ImageAspectFlagBits::eDepth);

        const auto swapchainImageViews = ava::raii::getSwapchainImageViews();
        framebuffers.clear();
        for (const auto& swapchainImageView : swapchainImageViews)
        {
            framebuffers.push_back(ava::raii::Framebuffer::create(renderPass, {swapchainImageView, depthImageView}, extent, 1));
        }
    }
};

int main()
{
    RayQuery rayQuery;
    rayQuery.run();
    return 0;
}
