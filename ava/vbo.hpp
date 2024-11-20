#ifndef AVA_VBO_HPP
#define AVA_VBO_HPP

#include "detail/vulkan.hpp"
#include "types.hpp"

namespace ava
{
    VBO createVBO(VAO vao, const void* data, size_t size, uint32_t binding = 0);

    template <typename T>
    VBO createVBO(const VAO vao, const std::span<T> data, const uint32_t binding = 0)
    {
        return createVBO(vao, data.data(), data.size() * sizeof(T), binding);
    }

    template <typename T>
    VBO createVBO(const VAO vao, const std::vector<T>& data, const uint32_t binding = 0)
    {
        return createVBO(vao, data.data(), data.size() * sizeof(T), binding);
    }

    void destroyVBO(VBO& vbo);

    void bindVBO(const CommandBuffer& commandBuffer, const VBO& vbo);
}

#endif
