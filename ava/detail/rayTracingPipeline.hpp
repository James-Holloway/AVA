#ifndef AVA_DETAIL_RAYTRACINGPIPELINE_HPP
#define AVA_DETAIL_RAYTRACINGPIPELINE_HPP

#include "./vulkan.hpp"
#include "../types.hpp"

namespace ava::detail
{
    struct ShaderBindingTable
    {
        vk::StridedDeviceAddressRegionKHR rayGenRegion;
        vk::StridedDeviceAddressRegionKHR missRegion;
        vk::StridedDeviceAddressRegionKHR hitRegion;
        vk::StridedDeviceAddressRegionKHR callableRegion;
        uint32_t handleCount;
        uint32_t missShaderCount;
        uint32_t hitShaderCount;
        uint32_t callbackShaderCount;
        ava::Buffer buffer;
    };

    struct RayTracingPipeline
    {
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;

        std::vector<std::vector<vk::DescriptorSetLayoutBinding>> layoutBindings;
        std::vector<vk::PushConstantRange> pushConstants;

        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

        ShaderBindingTable* shaderBindingTable;
    };
}

#endif
