#include "vao.hpp"
#include "ava/vao.hpp"
#include "ava/detail/vao.hpp"

#include "ava/detail/detail.hpp"

namespace ava::raii
{
    VAO::VAO(const ava::VAO& existingVAO)
    {
        AVA_CHECK(existingVAO != nullptr, "Cannot create RAII VAO when VAO is invalid");
        vao = existingVAO;
    }

    VAO::~VAO()
    {
        if (vao != nullptr)
        {
            ava::destroyVAO(vao);
        }
    }

    VAO::VAO(VAO&& other) noexcept
    {
        vao = other.vao;
        other.vao = nullptr;
    }

    VAO& VAO::operator=(VAO&& other) noexcept
    {
        if (this != &other)
        {
            vao = other.vao;
            other.vao = nullptr;
        }
        return *this;
    }

    VAO::operator ava::VAO() const
    {
        return vao;
    }

    Pointer<VAO> VAO::create(const std::vector<VertexAttribute>& vertexAttributes, const std::vector<uint32_t>& strides, const vk::PrimitiveTopology topology, const bool primitiveRestartEnable)
    {
        return std::make_shared<VAO>(ava::createVAO(vertexAttributes, strides, topology, primitiveRestartEnable));
    }

    Pointer<VAO> VAO::create(const std::vector<VertexAttribute>& vertexAttributes, const vk::PrimitiveTopology topology, const bool primitiveRestartEnable)
    {
        return std::make_shared<VAO>(ava::createVAO(vertexAttributes, topology, primitiveRestartEnable));
    }
}
