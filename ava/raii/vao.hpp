#ifndef AVA_RAII_VAO_HPP
#define AVA_RAII_VAO_HPP

#include "types.hpp"
#include "../vertexAttribute.hpp"

namespace ava::raii
{
    class VAO
    {
    public:
        using Ptr = Pointer<VAO>;

        explicit VAO(const ava::VAO& existingVAO);
        ~VAO();

        ava::VAO vao;

        VAO(const VAO& other) = delete;
        VAO& operator=(VAO& other) = delete;
        VAO(VAO&& other) noexcept;
        VAO& operator=(VAO&& other) noexcept;

        // ReSharper disable once CppNonExplicitConversionOperator
        operator ava::VAO() const;

        static Pointer<VAO> create(const std::vector<VertexAttribute>& vertexAttributes, const std::vector<uint32_t>& strides, vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList, bool primitiveRestartEnable = false);
        static Pointer<VAO> create(const std::vector<VertexAttribute>& vertexAttributes, vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList, bool primitiveRestartEnable = false);
    };
}

#endif
