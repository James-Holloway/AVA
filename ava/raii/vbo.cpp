#include "vbo.hpp"
#include "ava/vbo.hpp"
#include "ava/detail/vbo.hpp"

#include "commandBuffer.hpp"
#include "vao.hpp"
#include "ava/detail/detail.hpp"

namespace ava::raii
{
    VBO::VBO(const ava::VBO& existingVBO)
    {
        AVA_CHECK(existingVBO != nullptr && existingVBO->buffer != nullptr, "Cannot create RAII VBO from an invalid VBO");
        vbo = existingVBO;
    }

    VBO::~VBO()
    {
        ava::destroyVBO(vbo);
    }

    void VBO::bind(const Pointer<CommandBuffer>& commandBuffer) const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot bind VBO to an invalid command buffer");
        ava::bindVBO(commandBuffer->commandBuffer, vbo);
    }

    Pointer<VBO> VBO::create(const Pointer<VAO>& vao, const void* data, const size_t size, const uint32_t binding)
    {
        AVA_CHECK(vao != nullptr && vao->vao != nullptr, "Cannot create a VBO with an invalid vao");

        return std::make_shared<VBO>(ava::createVBO(vao->vao, data, size, binding));
    }
} // ava
