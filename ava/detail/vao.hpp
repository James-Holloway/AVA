#ifndef AVA_DETAIL_VAO_HPP
#define AVA_DETAIL_VAO_HPP

#include <vector>
#include "./vulkan.hpp"

namespace ava::detail
{
    struct VAO
    {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
        std::vector<vk::VertexInputBindingDescription> bindingDescriptions;
        vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
        std::vector<uint32_t> strides;
        bool primitiveRestartEnable;
    };
}

#endif
