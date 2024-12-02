#include "rayTracingPipeline.hpp"

#include "shaders.hpp"
#include "ava/detail/detail.hpp"

namespace ava::raii
{
    RayTracingPipeline::RayTracingPipeline(const ava::RayTracingPipeline existingRayTracingPipeline)
    {
        AVA_CHECK(existingRayTracingPipeline != nullptr, "Cannot create RAII ray tracing pipeline when existing ray tracing pipeline is invalid");

        pipeline = existingRayTracingPipeline;
    }

    RayTracingPipeline::~RayTracingPipeline()
    {
        if (pipeline != nullptr)
        {
            ava::destroyRayTracingPipeline(pipeline);
        }
    }

    RayTracingPipeline::RayTracingPipeline(RayTracingPipeline&& other) noexcept
    {
        pipeline = other.pipeline;
        other.pipeline = nullptr;
    }

    RayTracingPipeline& RayTracingPipeline::operator=(RayTracingPipeline&& other) noexcept
    {
        if (this != &other)
        {
            pipeline = other.pipeline;
            other.pipeline = nullptr;
        }
        return *this;
    }

    Pointer<RayTracingPipeline> RayTracingPipeline::create(const RayTracingPipelineCreationInfo& creationInfo)
    {
        return std::make_shared<RayTracingPipeline>(ava::createRayTracingPipeline(creationInfo));
    }

    void populateRayTracingPipelineCreationInfo(RayTracingPipelineCreationInfo& rayTracingPipelineCreationInfo, const std::vector<Pointer<Shader>>& shaders)
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

        rayTracingPipelineCreationInfo.shaders = avaShaders;
    }
}
