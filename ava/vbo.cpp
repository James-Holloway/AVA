#include "vbo.hpp"
#include "detail/vbo.hpp"

#include "buffer.hpp"
#include "detail/buffer.hpp"
#include "detail/commandBuffer.hpp"
#include "detail/detail.hpp"
#include "detail/vao.hpp"
#include "detail/state.hpp"

namespace ava
{
    VBO createVBO(const VAO vao, const void* data, size_t size, const uint32_t binding)
    {
        AVA_CHECK(vao != nullptr && !vao->strides.empty(), "Cannot create VBO from invalid VAO")
        AVA_CHECK(data != nullptr, "Cannot create VBO when data is nullptr")
        AVA_CHECK(size > 0, "Cannot create VBO when data size is 0")
        AVA_CHECK(binding < vao->strides.size(), "Cannot create VBO when binding is out of range of VAO bindings")
        const auto stride = vao->strides.at(binding);
        AVA_CHECK((size % stride) == 0, "Cannot create VBO when size is not a multiple of VAO's binding's stride (" + std::to_string(stride) + ")");

        auto buffer = createVertexBuffer(size);
        updateBuffer(buffer, data, size, 0);

        auto outVBO = new detail::VBO();
        outVBO->buffer = buffer;
        outVBO->vertexCount = size / stride;
        outVBO->stride = stride;
        outVBO->binding = binding;
        outVBO->topology = vao->topology;
        return outVBO;
    }

    void destroyVBO(VBO& vbo)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(vbo != nullptr, "Cannot destroy invalid VBO");
        AVA_CHECK_NO_EXCEPT_RETURN(detail::State.device, "Cannot destroy VBO when State's device is invalid")

        if (vbo->buffer != nullptr)
        {
            destroyBuffer(vbo->buffer);
        }

        delete vbo;
        vbo = nullptr;
    }

    void bindVBO(const CommandBuffer& commandBuffer, const VBO& vbo)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot bind VBO to an invalid command buffer");
        AVA_CHECK(vbo != nullptr && vbo->buffer && vbo->buffer->buffer, "Cannot bind an invalid VBO");
        AVA_CHECK(commandBuffer->pipelineCurrentlyBound, "Cannot bind a VBO when a pipeline has not yet been bound");

        commandBuffer->commandBuffer.bindVertexBuffers(vbo->binding, vbo->buffer->buffer, {0u});
    }
}
