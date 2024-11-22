#include "graphics.hpp"

#include "commandBuffer.hpp"
#include "renderPass.hpp"
#include "shaders.hpp"
#include "vao.hpp"
#include "ava/detail/detail.hpp"
#include "ava/detail/shaders.hpp"

namespace ava::raii
{
    GraphicsPipeline::GraphicsPipeline(const ava::GraphicsPipeline& existingPipeline)
    {
        AVA_CHECK(existingPipeline != nullptr && existingPipeline->pipeline, "Cannot create RAII graphics pipeline from an invalid graphics pipeline");
        pipeline = existingPipeline;
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        ava::destroyGraphicsPipeline(pipeline);
    }

    void GraphicsPipeline::bind(const Pointer<CommandBuffer>& commandBuffer) const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot bind graphics pipeline to an invalid command buffer");
        ava::bindGraphicsPipeline(commandBuffer->commandBuffer, pipeline);
    }

    Pointer<GraphicsPipeline> GraphicsPipeline::create(const GraphicsPipelineCreationInfo& creationInfo)
    {
        return std::make_shared<GraphicsPipeline>(ava::createGraphicsPipeline(creationInfo));
    }

    void populateGraphicsPipelineCreationInfo(GraphicsPipelineCreationInfo& pipelineCreationInfo, const std::vector<Pointer<Shader>>& shaders, const Pointer<RenderPass>& renderPass, uint32_t subpass, const Pointer<VAO>& vao, bool depthTest, bool depthWrite)
    {
        std::vector<ava::Shader> avaShaders;
        avaShaders.reserve(shaders.size());
        for (const auto& shader : shaders)
        {
            if (shader != nullptr)
            {
                avaShaders.push_back(shader->shader);
            }
            else
            {
                avaShaders.push_back(nullptr);
            }
        }
        const ava::RenderPass avaRenderPass = renderPass != nullptr ? renderPass->renderPass : nullptr;
        const ava::VAO avaVAO = vao != nullptr ? vao->vao : nullptr;

        ava::populateGraphicsPipelineCreationInfo(pipelineCreationInfo, avaShaders, avaRenderPass, subpass, avaVAO, depthTest, depthWrite);
    }
}
