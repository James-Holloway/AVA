#include "compute.hpp"
#include "ava/compute.hpp"

#include "commandBuffer.hpp"
#include "ava/detail/detail.hpp"
#include "ava/detail/shaders.hpp"

namespace ava::raii
{
    ComputePipeline::ComputePipeline(const ava::ComputePipeline& existingPipeline)
    {
        AVA_CHECK(existingPipeline != nullptr && existingPipeline->pipeline, "Cannot create a RAII compute pipeline from an invalid compute pipeline");

        pipeline = existingPipeline;
    }

    ComputePipeline::~ComputePipeline()
    {
        ava::destroyComputePipeline(pipeline);
    }

    void ComputePipeline::bind(const Pointer<CommandBuffer>& commandBuffer) const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot bind compute pipeline to an invalid command buffer");
        ava::bindComputePipeline(commandBuffer->commandBuffer, pipeline);
    }

    Pointer<ComputePipeline> ComputePipeline::create(const ComputePipelineCreationInfo& creationInfo)
    {
        return std::make_shared<ComputePipeline>(ava::createComputePipeline(creationInfo));
    }
}
