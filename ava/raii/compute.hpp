#ifndef AVA_RAII_COMPUTE_HPP
#define AVA_RAII_COMPUTE_HPP

#include "types.hpp"
#include "ava/compute.hpp"

namespace ava::raii
{
    class ComputePipeline
    {
    public:
        explicit ComputePipeline(const ava::ComputePipeline& existingPipeline);
        ~ComputePipeline();

        ava::ComputePipeline pipeline;

        ComputePipeline(const ComputePipeline& other) = delete;
        ComputePipeline& operator=(ComputePipeline& other) = delete;

        void bind(const Pointer<CommandBuffer>& commandBuffer) const;

        static Pointer<ComputePipeline> create(const ComputePipelineCreationInfo& creationInfo);
    };
}

#endif
