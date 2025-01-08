#include "vibo.hpp"
#include "ava/vibo.hpp"
#include "ava/detail/vibo.hpp"

#include "commandBuffer.hpp"
#include "vao.hpp"
#include "ava/detail/buffer.hpp"
#include "ava/detail/detail.hpp"


namespace ava::raii
{
    VIBO::VIBO(const ava::VIBO existingVIBO)
    {
        AVA_CHECK(existingVIBO != nullptr && existingVIBO->buffer != nullptr && existingVIBO->buffer->buffer != nullptr, "Cannot create a RAII VIBO from an invalid VIBO");

        vibo = existingVIBO;
    }

    VIBO::~VIBO()
    {
        if (vibo != nullptr)
        {
            ava::destroyVIBO(vibo);
        }
    }

    VIBO::VIBO(VIBO&& other) noexcept
    {
        vibo = other.vibo;
        other.vibo = nullptr;
    }

    VIBO& VIBO::operator=(VIBO&& other) noexcept
    {
        if (this != &other)
        {
            vibo = other.vibo;
            other.vibo = nullptr;
        }
        return *this;
    }

    void VIBO::bind(const Pointer<CommandBuffer>& commandBuffer) const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot bind a VIBO to an invalid command buffer");

        ava::bindVIBO(commandBuffer->commandBuffer, vibo);
    }

    Pointer<VIBO> VIBO::create(const Pointer<VAO>& vao, const void* vertexData, const size_t vertexDataSize, const uint16_t* indices, const uint32_t indexCount, const uint32_t vertexBinding)
    {
        AVA_CHECK(vao != nullptr && vao->vao != nullptr, "Cannot create a VIBO with an invalid VAO");

        return std::make_shared<VIBO>(ava::createVIBO(vao->vao, vertexData, vertexDataSize, indices, indexCount, vertexBinding));
    }

    Pointer<VIBO> VIBO::create(const Pointer<VAO>& vao, const void* vertexData, const size_t vertexDataSize, const uint32_t* indices, const uint32_t indexCount, const uint32_t vertexBinding)
    {
        AVA_CHECK(vao != nullptr && vao->vao != nullptr, "Cannot create a VIBO with an invalid VAO");

        return std::make_shared<VIBO>(ava::createVIBO(vao->vao, vertexData, vertexDataSize, indices, indexCount, vertexBinding));
    }
}
