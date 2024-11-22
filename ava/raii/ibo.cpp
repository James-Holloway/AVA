#include "ibo.hpp"
#include "ava/ibo.hpp"
#include "ava/detail/ibo.hpp"

#include "commandBuffer.hpp"
#include "ava/detail/detail.hpp"

namespace ava::raii
{
    IBO::IBO(const ava::IBO& existingIBO)
    {
        AVA_CHECK(existingIBO != nullptr && existingIBO->buffer, "Cannot create a RAII IBO from an invalid IBO");
        ibo = existingIBO;
    }

    IBO::~IBO()
    {
        destroyIBO(ibo);
    }

    void IBO::bind(const Pointer<CommandBuffer>& commandBuffer) const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot bind IBO to an invalid command buffer");
        ava::bindIBO(commandBuffer->commandBuffer, ibo);
    }

    Pointer<IBO> IBO::create(const uint32_t* indices, const uint32_t indexCount)
    {
        return std::make_shared<IBO>(ava::createIBO(indices, indexCount));
    }

    Pointer<IBO> IBO::create(const uint16_t* indices, const uint32_t indexCount)
    {
        return std::make_shared<IBO>(ava::createIBO(indices, indexCount));
    }

    Pointer<IBO> IBO::create(const std::span<const uint32_t> indices)
    {
        return std::make_shared<IBO>(ava::createIBO(indices));
    }

    Pointer<IBO> IBO::create(const std::span<const uint16_t> indices)
    {
        return std::make_shared<IBO>(ava::createIBO(indices));
    }

    Pointer<IBO> IBO::create(const std::vector<uint32_t>& indices)
    {
        return std::make_shared<IBO>(ava::createIBO(indices));
    }

    Pointer<IBO> IBO::create(const std::vector<uint16_t>& indices)
    {
        return std::make_shared<IBO>(ava::createIBO(indices));
    }
} // ava
