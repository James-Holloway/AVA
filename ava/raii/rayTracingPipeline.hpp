#ifndef AVA_RAII_RAYTRACINGPIPELINE_HPP
#define AVA_RAII_RAYTRACINGPIPELINE_HPP

#include "types.hpp"
#include "../types.hpp"
#include "ava/rayTracingPipeline.hpp"

namespace ava::raii
{
    class RayTracingPipeline
    {
    public:
        using Ptr = std::shared_ptr<RayTracingPipeline>;

        explicit RayTracingPipeline(ava::RayTracingPipeline existingRayTracingPipeline);
        ~RayTracingPipeline();

        ava::RayTracingPipeline pipeline;

        RayTracingPipeline(const RayTracingPipeline& other) = delete;
        RayTracingPipeline& operator=(RayTracingPipeline& other) = delete;
        RayTracingPipeline(RayTracingPipeline&& other) noexcept;
        RayTracingPipeline& operator=(RayTracingPipeline&& other) noexcept;

        static Pointer<RayTracingPipeline> create(const RayTracingPipelineCreationInfo& creationInfo);
    };

    void populateRayTracingPipelineCreationInfo(RayTracingPipelineCreationInfo& rayTracingPipelineCreationInfo, const std::vector<Pointer<Shader>>& shaders);
}

#endif
