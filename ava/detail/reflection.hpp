#ifndef AVA_DETAIL_REFLECTION_HPP
#define AVA_DETAIL_REFLECTION_HPP

#include "./vulkan.hpp"
#include "./shaders.hpp"

namespace ava::detail
{
    struct ReflectedShaderInfo
    {
        std::vector<std::vector<vk::DescriptorSetLayoutBinding>> layoutBindings;
        std::vector<vk::PushConstantRange> pushConstants;
    };

    ReflectedShaderInfo reflect(const std::vector<Shader*>& shaders);
}


#endif
