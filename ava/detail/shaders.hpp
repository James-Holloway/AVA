#ifndef AVA_DETAIL_SHADERS_HPP
#define AVA_DETAIL_SHADERS_HPP

#include "./vulkan.hpp"

namespace ava::detail
{
    struct Shader
    {
        vk::ShaderModule module;
        vk::ShaderStageFlagBits stage;
        std::vector<char> spriv;
        std::string entry;
    };

    struct GraphicsPipeline
    {
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        vk::ShaderStageFlags stages;

        std::vector<std::vector<vk::DescriptorSetLayoutBinding>> layoutBindings;
        std::vector<vk::PushConstantRange> pushConstants;

        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;

        std::vector<vk::DynamicState> dynamicStates;
        bool depthTest;
        bool depthWrite;
        float minDepth;
        float maxDepth;
    };

    vk::ShaderModule createShaderModule(const std::vector<char>& spirv);
    vk::ShaderModule loadShaderModule(const std::string& filePath, std::vector<char>& spirv);
}

#endif
