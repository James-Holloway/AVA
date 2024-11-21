#ifndef AVA_VAO_HPP
#define AVA_VAO_HPP

#include <vector>
#include "detail/vulkan.hpp"
#include "vertexAttribute.hpp"
#include "types.hpp"

namespace ava
{
    [[nodiscard]] VAO createVAO(const std::vector<VertexAttribute>& vertexAttributes, const std::vector<uint32_t>& strides, vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList, bool primitiveRestartEnable = false);
    [[nodiscard]] VAO createVAO(const std::vector<VertexAttribute>& vertexAttributes, vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList, bool primitiveRestartEnable = false);
    void destroyVAO(VAO& vao);
}


#endif
