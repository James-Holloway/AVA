#ifndef AVA_IBO_HPP
#define AVA_IBO_HPP

#include "types.hpp"

namespace ava
{
    [[nodiscard]] IBO createIBO(const uint32_t* indices, uint32_t indexCount);
    [[nodiscard]] IBO createIBO(const uint16_t* indices, uint32_t indexCount);

    [[nodiscard]] IBO createIBO(std::span<const uint32_t> indices);
    [[nodiscard]] IBO createIBO(std::span<const uint16_t> indices);

    [[nodiscard]] IBO createIBO(const std::vector<uint32_t>& indices);
    [[nodiscard]] IBO createIBO(const std::vector<uint16_t>& indices);

    void destroyIBO(IBO& ibo);

    void bindIBO(const CommandBuffer& commandBuffer, const IBO& ibo);
}

#endif
