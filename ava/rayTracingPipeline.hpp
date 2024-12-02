#ifndef AVA_RAYTRACINGPIPELINE_HPP
#define AVA_RAYTRACINGPIPELINE_HPP

#include "types.hpp"

namespace ava
{
    struct RayTracingPipelineCreationInfo
    {
        // When providing shaders, any hit and intersection shaders must come after the corresponding closest hit shader
        std::vector<ava::Shader> shaders{};
        uint32_t maxRayRecursionDepth = 1; // AMD's limits specify 1
    };

    RayTracingPipeline createRayTracingPipeline(const RayTracingPipelineCreationInfo& creationInfo);
    void destroyRayTracingPipeline(RayTracingPipeline& rayTracingPipeline);

    void populateRayTracingPipelineCreationInfo(RayTracingPipelineCreationInfo& rayTracingPipelineCreationInfo, const std::vector<Shader>& shaders);

    void bindRayTracingPipeline(const CommandBuffer& commandBuffer, const RayTracingPipeline& rayTracingPipeline);
    void traceRays(const CommandBuffer& commandBuffer, uint32_t width, uint32_t height, uint32_t depth = 1);
}

#endif
