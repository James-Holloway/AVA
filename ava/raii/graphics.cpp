#include "graphics.hpp"

#include "commandBuffer.hpp"
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
}
