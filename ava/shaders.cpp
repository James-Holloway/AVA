#include "shaders.hpp"

#include "detail/shaders.hpp"
#include "detail/detail.hpp"
#include "detail/reflection.hpp"
#include "detail/renderPass.hpp"
#include "detail/state.hpp"
#include "detail/vao.hpp"

namespace ava
{
    static std::vector<uint32_t> getShaderIndicesFromType(const std::vector<Shader>& shaders, const vk::ShaderStageFlagBits stage)
    {
        std::vector<uint32_t> shaderIndices;
        for (uint32_t i = 0; i < shaders.size(); i++)
        {
            const auto& shader = shaders[i];
            if ((shader->stage & stage) != vk::ShaderStageFlags{})
            {
                shaderIndices.push_back(i);
            }
        }
        return shaderIndices;
    }

    // ReSharper disable once CppDFAConstantParameter
    static bool hasShaderType(const std::vector<Shader>& shaders, const vk::ShaderStageFlagBits stage)
    {
        for (const auto& shader : shaders)
        {
            if ((shader->stage & stage) != vk::ShaderStageFlags{})
            {
                return true;
            }
        }
        return false;
    }

    Shader createShader(const std::string& shaderPath, const vk::ShaderStageFlagBits stage, const std::string& entry)
    {
        std::vector<char> spirv;
        auto module = detail::loadShaderModule(shaderPath, spirv);
        auto outShader = new detail::Shader();
        outShader->module = module;
        outShader->stage = stage;
        outShader->entry = entry;
        outShader->spriv = spirv;
        return outShader;
    }

    Shader createShader(const std::vector<char>& shaderSpirv, const vk::ShaderStageFlagBits stage, const std::string& entry)
    {
        AVA_CHECK(!shaderSpirv.empty(), "Cannot create shader from empty shader SPIR-V vector");

        auto module = detail::createShaderModule(shaderSpirv);
        auto outShader = new detail::Shader();
        outShader->module = module;
        outShader->stage = stage;
        outShader->entry = entry;
        outShader->spriv = shaderSpirv;
        return outShader;
    }

    void destroyShader(Shader& shader)
    {
        AVA_CHECK(shader != nullptr, "Cannot destroy an invalid shader");
        if (shader->module)
        {
            detail::State.device.destroyShaderModule(shader->module);
        }

        delete shader;
        shader = nullptr;
    }

    void populateGraphicsPipelineCreationInfo(GraphicsPipelineCreationInfo& pipelineCreationInfo, const std::vector<Shader>& shaders, const RenderPass& renderPass, uint32_t subpass, const VAO& vao, const bool depthTest, const bool depthWrite)
    {
        if (renderPass != nullptr)
        {
            AVA_CHECK(subpass < renderPass->subpasses, "Cannot populate graphics pipeline creation info with a subpass not less than the number of subpasses in the provided render pass");
            AVA_CHECK(renderPass->subpassColorAttachmentCounts.size() >= renderPass->subpasses, "Cannot populate graphics pipeline creation info with an invalid render pass (subpassColorAttachmentCounts incorrectly sized)");

            uint32_t colorAttachmentCount = renderPass->subpassColorAttachmentCounts[subpass];

            constexpr static vk::PipelineColorBlendAttachmentState defaultColorBlend{
                true, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
                vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
            };

            pipelineCreationInfo.colorBlendAttachments.resize(colorAttachmentCount, defaultColorBlend);
        }

        pipelineCreationInfo.shaders = shaders;
        pipelineCreationInfo.renderPass = renderPass;
        pipelineCreationInfo.subpass = subpass;
        pipelineCreationInfo.vao = vao;
        pipelineCreationInfo.depthStencil.depthTestEnable = depthTest;
        pipelineCreationInfo.depthStencil.depthWriteEnable = depthWrite;
    }

    GraphicsPipeline createGraphicsPipeline(const GraphicsPipelineCreationInfo& pipelineCreationInfo)
    {
        AVA_CHECK(detail::State.device, "Cannot create a graphics pipeline with an invalid State device")
        AVA_CHECK(pipelineCreationInfo.renderPass != nullptr && pipelineCreationInfo.renderPass->renderPass, "Cannot create a graphics pipeline with an invalid RenderPass");
        AVA_CHECK(!pipelineCreationInfo.shaders.empty(), "Cannot create a graphics pipeline with no shaders");

        const auto vertexShaderIndices = getShaderIndicesFromType(pipelineCreationInfo.shaders, vk::ShaderStageFlagBits::eVertex);
        AVA_CHECK(!vertexShaderIndices.empty(), "Cannot create a graphics pipeline with no vertex shader");
        AVA_CHECK(vertexShaderIndices.size() == 1, "Cannot create a graphics pipeline with more than 1 vertex shader");
        const auto fragmentShaderIndices = getShaderIndicesFromType(pipelineCreationInfo.shaders, vk::ShaderStageFlagBits::eFragment);
        AVA_CHECK(fragmentShaderIndices.size() <= 1, "Cannot create a graphics pipeline with more than 1 fragment shader"); // Fragment shader can be omitted if depth only
        AVA_CHECK(!hasShaderType(pipelineCreationInfo.shaders, vk::ShaderStageFlagBits::eCompute), "Cannot create a graphics pipeline with more than 0 compute shaders, create a compute pipeline instead");

        const auto [layoutBindings, pushConstants] = detail::reflect(pipelineCreationInfo.shaders);

        // Create descriptor set layouts
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        descriptorSetLayouts.reserve(layoutBindings.size());
        for (auto& setLayouts : layoutBindings)
        {
            vk::DescriptorSetLayoutCreateInfo layoutInfo{{}, setLayouts};
            descriptorSetLayouts.push_back(detail::State.device.createDescriptorSetLayout(layoutInfo));
        }

        // Shader stages
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
        shaderStages.reserve(pipelineCreationInfo.shaders.size());
        vk::ShaderStageFlags allStages{};
        for (auto& shader : pipelineCreationInfo.shaders)
        {
            shaderStages.emplace_back(vk::PipelineShaderStageCreateFlags{}, shader->stage, shader->module, shader->entry.c_str(), nullptr, nullptr); // TODO: Allow specialization constants
            allStages |= shader->stage;
        }

        // Vertex input assembly info
        vk::PipelineVertexInputStateCreateInfo vertexInput{};
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
        if (pipelineCreationInfo.vao != nullptr && !pipelineCreationInfo.vao->bindingDescriptions.empty() && !pipelineCreationInfo.vao->attributeDescriptions.empty())
        {
            vertexInput.setVertexAttributeDescriptions(pipelineCreationInfo.vao->attributeDescriptions);;
            vertexInput.setVertexBindingDescriptions(pipelineCreationInfo.vao->bindingDescriptions);

            inputAssembly.topology = pipelineCreationInfo.vao->topology;
            inputAssembly.primitiveRestartEnable = pipelineCreationInfo.vao->primitiveRestartEnable;
        }
        else
        {
            inputAssembly.topology = pipelineCreationInfo.fallbackTopology;
        }

        // Dynamic states
        vk::PipelineDynamicStateCreateInfo dynamicState;
        dynamicState.setDynamicStates(pipelineCreationInfo.dynamicStates);

        // Color blend
        auto colorBlend = pipelineCreationInfo.colorBlend;
        if (colorBlend.attachmentCount == 0 && colorBlend.pAttachments == nullptr)
        {
            colorBlend.setAttachments(pipelineCreationInfo.colorBlendAttachments);
        }

        // Pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setSetLayouts(descriptorSetLayouts);
        pipelineLayoutInfo.setPushConstantRanges(pushConstants);

        auto pipelineLayout = detail::State.device.createPipelineLayout(pipelineLayoutInfo);

        // Create the pipeline
        vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
        graphicsPipelineCreateInfo
            .setStages(shaderStages)
            .setLayout(pipelineLayout)
            .setRenderPass(pipelineCreationInfo.renderPass->renderPass)
            .setSubpass(pipelineCreationInfo.subpass)
            .setPVertexInputState(&vertexInput)
            .setPInputAssemblyState(&inputAssembly)
            .setPDynamicState(&dynamicState)
            .setPViewportState(&pipelineCreationInfo.viewport)
            .setPRasterizationState(&pipelineCreationInfo.rasterizer)
            .setPMultisampleState(&pipelineCreationInfo.multisampling)
            .setPDepthStencilState(&pipelineCreationInfo.depthStencil)
            .setPColorBlendState(&colorBlend)
            .setPTessellationState(&pipelineCreationInfo.tessellation)
            .setBasePipelineHandle(nullptr)
            .setBasePipelineIndex(-1);

        auto pipeline = detail::State.device.createGraphicsPipeline(nullptr, graphicsPipelineCreateInfo);
        vk::detail::resultCheck(pipeline.result, "Failed to create a graphics pipeline");

        // Create ava graphics pipeline
        auto outGraphicsPipeline = new detail::GraphicsPipeline();
        outGraphicsPipeline->layout = pipelineLayout;
        outGraphicsPipeline->pipeline = pipeline.value;
        outGraphicsPipeline->stages = allStages;
        outGraphicsPipeline->depthTest = pipelineCreationInfo.depthStencil.depthTestEnable;
        outGraphicsPipeline->depthWrite = pipelineCreationInfo.depthStencil.depthWriteEnable;
        outGraphicsPipeline->dynamicStates = pipelineCreationInfo.dynamicStates;
        outGraphicsPipeline->layoutBindings = layoutBindings;
        outGraphicsPipeline->pushConstants = pushConstants;
        outGraphicsPipeline->minDepth = pipelineCreationInfo.depthStencil.minDepthBounds;
        outGraphicsPipeline->maxDepth = pipelineCreationInfo.depthStencil.maxDepthBounds;
        return outGraphicsPipeline;
    }

    void destroyGraphicsPipeline(GraphicsPipeline& pipeline)
    {
        AVA_CHECK(pipeline != nullptr, "Cannot destroy invalid graphics pipeline");
        AVA_CHECK(detail::State.device, "Cannot destroy graphics pipeline when State's device is invalid")
        if (pipeline->pipeline)
        {
            detail::State.device.destroyPipeline(pipeline->pipeline);
        }

        if (pipeline->layout)
        {
            detail::State.device.destroyPipelineLayout(pipeline->layout);
        }

        delete pipeline;
        pipeline = nullptr;
    }

    static void setupDefaultDynamicState(const vk::CommandBuffer& commandBuffer, const GraphicsPipeline& pipeline, const vk::DynamicState dynamicState)
    {
        const auto extent = detail::State.currentRenderPassExtent;
        AVA_CHECK(extent.width != 0 && extent.height != 0, "Cannot setup default dynamic state as a render pass has not been started properly")

        switch (dynamicState)
        {
        case vk::DynamicState::eScissor:
            {
                commandBuffer.setScissor(0, vk::Rect2D{{0, 0}, extent});
                return;
            }
        case vk::DynamicState::eViewport:
            {
                commandBuffer.setViewport(0, vk::Viewport{0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), pipeline->minDepth, pipeline->maxDepth});
                return;
            }
        default:
            return;
        }
    }

    void bindGraphicsPipeline(const vk::CommandBuffer& commandBuffer, const GraphicsPipeline& pipeline)
    {
        AVA_CHECK(pipeline != nullptr && pipeline->pipeline, "Cannot bind invalid graphics pipeline");
        AVA_CHECK(commandBuffer, "Cannot bind a pipeline to an invalid command buffer");

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->pipeline);

        for (const auto dynamicState : pipeline->dynamicStates)
        {
            setupDefaultDynamicState(commandBuffer, pipeline, dynamicState);
        }
    }
}
