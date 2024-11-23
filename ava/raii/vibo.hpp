#ifndef AVA_RAII_VIBO_HPP
#define AVA_RAII_VIBO_HPP

#include "types.hpp"

namespace ava::raii
{
    class VIBO
    {
    public:
        using Ptr = Pointer<VIBO>;

        VIBO(ava::VIBO existingVIBO);
        ~VIBO();

        ava::VIBO vibo;

        VIBO(const VIBO& other) = delete;
        VIBO& operator=(VIBO& other) = delete;
        VIBO(VIBO&& other) noexcept;
        VIBO& operator=(VIBO&& other) noexcept;

        void bind(const Pointer<CommandBuffer>& commandBuffer) const;

        static Pointer<VIBO> create(const Pointer<VAO>& vao, const void* vertexData, size_t vertexDataSize, uint16_t* indices, uint32_t indexCount, uint32_t vertexBinding = 0);
        static Pointer<VIBO> create(const Pointer<VAO>& vao, const void* vertexData, size_t vertexDataSize, uint32_t* indices, uint32_t indexCount, uint32_t vertexBinding = 0);

        template <typename T>
        static Pointer<VIBO> create(const Pointer<VAO>& vao, std::span<T> vertexData, std::span<uint16_t> indices, const uint32_t vertexBinding = 0)
        {
            return create(vao, vertexData.data(), vertexData.size() * sizeof(T), indices.data(), indices.size(), vertexBinding);
        }

        template <typename T>
        static Pointer<VIBO> create(const Pointer<VAO>& vao, std::span<T> vertexData, std::span<uint32_t> indices, const uint32_t vertexBinding = 0)
        {
            return create(vao, vertexData.data(), vertexData.size() * sizeof(T), indices.data(), indices.size(), vertexBinding);
        }

        template <typename T>
        static Pointer<VIBO> create(const Pointer<VAO>& vao, const std::vector<T>& vertexData, const std::vector<uint16_t>& indices, const uint32_t vertexBinding = 0)
        {
            return create(vao, vertexData.data(), vertexData.size() * sizeof(T), indices.data(), indices.size(), vertexBinding);
        }

        template <typename T>
        static Pointer<VIBO> create(const Pointer<VAO>& vao, const std::vector<T>& vertexData, const std::vector<uint32_t>& indices, const uint32_t vertexBinding = 0)
        {
            return create(vao, vertexData.data(), vertexData.size() * sizeof(T), indices.data(), indices.size(), vertexBinding);
        }
    };
}


#endif
