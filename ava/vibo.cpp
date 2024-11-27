#include "vibo.hpp"
#include "detail/vibo.hpp"

#include "buffer.hpp"
#include "detail/buffer.hpp"
#include "detail/commandBuffer.hpp"
#include "detail/detail.hpp"
#include "detail/vao.hpp"

namespace ava
{
    template <typename Ti>
    VIBO createVIBOMain(const VAO vao, const void* vertexData, const size_t vertexDataSize, Ti* indices, const uint32_t indexCount, const uint32_t vertexBinding, vk::IndexType indexType)
    {
        AVA_CHECK(vao != nullptr && !vao->strides.empty(), "Cannot create VIBO from invalid VAO")
        AVA_CHECK(vertexData != nullptr, "Cannot create VIBO when data is nullptr")
        AVA_CHECK(vertexDataSize > 0, "Cannot create VIBO when vertex data size is 0")
        AVA_CHECK(vertexBinding < vao->strides.size(), "Cannot create VIBO when vertex binding is out of range of VAO bindings")
        const auto stride = vao->strides.at(vertexBinding);
        AVA_CHECK((vertexDataSize % stride) == 0, "Cannot create VIBO when vertex data size is not a multiple of VAO's binding's stride (" + std::to_string(stride) + ")");
        AVA_CHECK(indices != nullptr, "Cannot create VIBO when indices is nullptr");
        AVA_CHECK(indexCount > 0, "Cannot create VIBO when indexCount is 0");

        const vk::DeviceSize indexSize = indexCount * sizeof(Ti);
        const vk::DeviceSize size = vertexDataSize + indexSize;

        const auto buffer = createBuffer(size, DEFAULT_VERTEX_BUFFER_USAGE | DEFAULT_INDEX_BUFFER_USAGE);
        updateBuffer(buffer, vertexData, vertexDataSize, 0);
        updateBuffer(buffer, indices, indexSize, vertexDataSize);

        auto outVIBO = new detail::VIBO();
        outVIBO->buffer = buffer;
        outVIBO->vertexCount = size / stride;
        outVIBO->stride = stride;
        outVIBO->binding = vertexBinding;
        outVIBO->indexCount = indexCount;
        outVIBO->vertexOffset = 0;
        outVIBO->indexOffset = vertexDataSize;
        outVIBO->indexType = indexType;
        outVIBO->topology = vao->topology;
        return outVIBO;
    }

    VIBO createVIBO(const VAO vao, const void* vertexData, const size_t vertexDataSize, uint16_t* indices, const uint32_t indexCount, const uint32_t vertexBinding)
    {
        return createVIBOMain(vao, vertexData, vertexDataSize, indices, indexCount, vertexBinding, vk::IndexType::eUint16);
    }

    VIBO createVIBO(const VAO vao, const void* vertexData, const size_t vertexDataSize, uint32_t* indices, const uint32_t indexCount, const uint32_t vertexBinding)
    {
        return createVIBOMain(vao, vertexData, vertexDataSize, indices, indexCount, vertexBinding, vk::IndexType::eUint32);
    }

    void destroyVIBO(VIBO& vibo)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(vibo != nullptr, "Cannot destroy invalid VIBO");

        if (vibo->buffer != nullptr)
        {
            destroyBuffer(vibo->buffer);
        }

        delete vibo;
        vibo = nullptr;
    }

    void bindVIBO(const CommandBuffer& commandBuffer, const VIBO& vibo)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot bind VIBO to an invalid command buffer");
        AVA_CHECK(vibo != nullptr && vibo->buffer && vibo->buffer->buffer, "Cannot bind an invalid VIBO");
        AVA_CHECK(commandBuffer->pipelineCurrentlyBound, "Cannot bind an VIBO when a pipeline has not yet been bound");

        commandBuffer->commandBuffer.bindVertexBuffers(vibo->binding, vibo->buffer->buffer, vibo->vertexOffset);
        commandBuffer->commandBuffer.bindIndexBuffer(vibo->buffer->buffer, vibo->indexOffset, vibo->indexType);
        commandBuffer->lastBoundIndexBufferIndexCount = vibo->indexCount;
    }
} // ava
