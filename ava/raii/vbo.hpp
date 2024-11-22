#ifndef AVA_RAII_VBO_HPP
#define AVA_RAII_VBO_HPP

#include "types.hpp"

namespace ava::raii
{
    class VBO
    {
    public:
        explicit VBO(const ava::VBO& existingVBO);
        ~VBO();

        ava::VBO vbo;

        void bind(const Pointer<CommandBuffer>& commandBuffer) const;

        static Pointer<VBO> create(const Pointer<VAO>& vao, const void* data, size_t size, uint32_t binding = 0);

        template <typename T>
        static Pointer<VBO> create(const Pointer<VAO> vao, const std::span<T> data, const uint32_t binding = 0)
        {
            return create(vao, data.data(), data.size() * sizeof(T), binding);
        }

        template <typename T>
        static Pointer<VBO> create(const Pointer<VAO> vao, const std::vector<T>& data, const uint32_t binding = 0)
        {
            return create(vao, data.data(), data.size() * sizeof(T), binding);
        }
    };
} // ava

#endif