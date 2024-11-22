#ifndef AVA_VIBO_HPP
#define AVA_VIBO_HPP

#include "types.hpp"

namespace ava
{
    VIBO createVIBO(VAO vao, const void* vertexData, size_t vertexDataSize, uint16_t* indices, uint32_t indexCount, uint32_t vertexBinding = 0);
    VIBO createVIBO(VAO vao, const void* vertexData, size_t vertexDataSize, uint32_t* indices, uint32_t indexCount, uint32_t vertexBinding = 0);

    template <typename T>
    VIBO createVIBO(VAO vao, std::span<T> vertexData, std::span<uint16_t> indices, const uint32_t vertexBinding = 0)
    {
        return createVIBO(vao, vertexData.data(), vertexData.size() * sizeof(T), indices.data(), indices.size(), vertexBinding);
    }

    template <typename T>
    VIBO createVIBO(VAO vao, std::span<T> vertexData, std::span<uint32_t> indices, const uint32_t vertexBinding = 0)
    {
        return createVIBO(vao, vertexData.data(), vertexData.size() * sizeof(T), indices.data(), indices.size(), vertexBinding);
    }

    template <typename T>
    VIBO createVIBO(VAO vao, const std::vector<T>& vertexData, const std::vector<uint16_t>& indices, const uint32_t vertexBinding = 0)
    {
        return createVIBO(vao, vertexData.data(), vertexData.size() * sizeof(T), indices.data(), indices.size(), vertexBinding);
    }

    template <typename T>
    VIBO createVIBO(VAO vao, const std::vector<T>& vertexData, const std::vector<uint32_t>& indices, const uint32_t vertexBinding = 0)
    {
        return createVIBO(vao, vertexData.data(), vertexData.size() * sizeof(T), indices.data(), indices.size(), vertexBinding);
    }

    void destroyVIBO(VIBO& vibo);

    void bindVIBO(const CommandBuffer& commandBuffer, const VIBO& vibo);
}

#endif
