#ifndef AVA_SHADERS_HPP
#define AVA_SHADERS_HPP

#include <vector>

#include "commandBuffer.hpp"
#include "detail/vulkan.hpp"
#include "vao.hpp"
#include "renderPass.hpp"
#include "types.hpp"

namespace ava
{
    struct GraphicsPipelineCreationInfo
    {
        std::vector<Shader> shaders;

        RenderPass renderPass; // Render pass to use
        uint32_t subpass = 0; // Subpass of the render pass to use
        VAO vao = nullptr; // Optional if you create vertices in the vertex shader

        // All below are pipeline state initialized with reasonable defaults. The one exception is depth stencil, but populateGraphicsPipelineCreationInfo can be used for simplicity
        vk::PipelineRasterizationStateCreateInfo rasterizer{{}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f};
        vk::PipelineMultisampleStateCreateInfo multisampling{{}, vk::SampleCountFlagBits::e1, false, 1.0f, nullptr, false, false};
        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments{};
        vk::PipelineColorBlendStateCreateInfo colorBlend{{}, false, vk::LogicOp::eCopy, {}, {0.0f, 0.0f, 0.0f, 0.0f}};
        vk::PipelineDepthStencilStateCreateInfo depthStencil{{}, false, false, vk::CompareOp::eLess, false, false, {}, {}, 0.0f, 1.0f};
        vk::PipelineTessellationStateCreateInfo tessellation{{}, 0};
        vk::PipelineViewportStateCreateInfo viewport{{}, 1, nullptr, 1, nullptr};
        std::vector<vk::DynamicState> dynamicStates{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        vk::PrimitiveTopology fallbackTopology = vk::PrimitiveTopology::eTriangleList; // Topology to be used if VAO is not provided
    };

    [[nodiscard]] Shader createShader(const std::string& shaderPath, vk::ShaderStageFlagBits stage, const std::string& entry = "main");
    [[nodiscard]] Shader createShader(const std::vector<char>& shaderSpirv, vk::ShaderStageFlagBits stage, const std::string& entry = "main");
    void destroyShader(Shader& shader);

    // Graphics pipeline population
    void populateGraphicsPipelineCreationInfo(GraphicsPipelineCreationInfo& pipelineCreationInfo, const std::vector<Shader>& shaders, const RenderPass& renderPass, uint32_t subpass, const VAO& vao, bool depthTest, bool depthWrite);
    // Graphics pipeline creation
    [[nodiscard]] GraphicsPipeline createGraphicsPipeline(const GraphicsPipelineCreationInfo& pipelineCreationInfo);
    void destroyGraphicsPipeline(GraphicsPipeline& pipeline);

    void bindGraphicsPipeline(const CommandBuffer& commandBuffer, const GraphicsPipeline& pipeline);
}

#endif
