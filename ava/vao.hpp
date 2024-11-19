#ifndef AVA_VAO_HPP
#define AVA_VAO_HPP

#include <vector>
#include "detail/vulkan.hpp"
#include "vertexAttribute.hpp"

namespace ava
{
    namespace detail
    {
        struct VAO;
    }

    using VAO = detail::VAO*;

    VAO createVAO(const std::vector<VertexAttribute>& vertexAttributes, const std::vector<uint32_t>& strides, vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList, bool primitiveRestartEnable = false);
    VAO createVAO(const std::vector<VertexAttribute>& vertexAttributes, vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList, bool primitiveRestartEnable = false);
}


#endif
