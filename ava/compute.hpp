#ifndef AVA_COMPUTE_HPP
#define AVA_COMPUTE_HPP

#include "types.hpp"

namespace ava
{
    struct ComputePipelineCreationInfo
    {
        Shader shader = nullptr;
    };

    [[nodiscard]] ComputePipeline createComputePipeline(const ComputePipelineCreationInfo& pipelineCreationInfo);
    void destroyComputePipeline(ComputePipeline& pipeline);

    void bindComputePipeline(const CommandBuffer& commandBuffer, const ComputePipeline& pipeline);

    void dispatch(const CommandBuffer& commandBuffer, uint32_t groupCountX, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
}

#endif
