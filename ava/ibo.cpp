#include "ibo.hpp"

#include "detail/detail.hpp"
#include "detail/ibo.hpp"

#include "buffer.hpp"
#include "detail/buffer.hpp"
#include "detail/commandBuffer.hpp"

namespace ava
{
    IBO createIBO(const uint32_t* indices, const uint32_t indexCount)
    {
        AVA_CHECK(indices != nullptr, "Cannot create IBO when indices is nullptr");
        AVA_CHECK(indexCount > 0, "Cannot create IBO when indexCount is 0");

        constexpr auto indexType = vk::IndexType::eUint32;
        const vk::DeviceSize size = indexCount * sizeof(uint32_t);

        const auto buffer = createIndexBuffer(size);
        updateBuffer(buffer, indices, size, 0);

        const auto outIBO = new detail::IBO;
        outIBO->buffer = buffer;
        outIBO->indexType = indexType;
        outIBO->indexCount = indexCount;
        return outIBO;
    }

    IBO createIBO(const uint16_t* indices, const uint32_t indexCount)
    {
        AVA_CHECK(indices != nullptr, "Cannot create IBO when indices is nullptr");
        AVA_CHECK(indexCount > 0, "Cannot create IBO when indexCount is 0");

        constexpr auto indexType = vk::IndexType::eUint16;
        const vk::DeviceSize size = indexCount * sizeof(uint16_t);

        const auto buffer = createIndexBuffer(size);
        updateBuffer(buffer, indices, size, 0);

        const auto outIBO = new detail::IBO;
        outIBO->buffer = buffer;
        outIBO->indexType = indexType;
        outIBO->indexCount = indexCount;
        return outIBO;
    }

    IBO createIBO(const std::span<const uint32_t> indices)
    {
        return createIBO(indices.data(), indices.size());
    }

    IBO createIBO(const std::span<const uint16_t> indices)
    {
        return createIBO(indices.data(), indices.size());
    }

    IBO createIBO(const std::vector<uint32_t>& indices)
    {
        return createIBO(indices.data(), indices.size());
    }

    IBO createIBO(const std::vector<uint16_t>& indices)
    {
        return createIBO(indices.data(), indices.size());
    }

    void destroyIBO(IBO& ibo)
    {
        AVA_CHECK(ibo != nullptr, "Cannot destroy invalid IBO");

        if (ibo->buffer != nullptr)
        {
            destroyBuffer(ibo->buffer);
        }

        delete ibo;
        ibo = nullptr;
    }

    void bindIBO(const CommandBuffer& commandBuffer, const IBO& ibo)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot bind IBO to an invalid command buffer");
        AVA_CHECK(ibo != nullptr && ibo->buffer && ibo->buffer->buffer, "Cannot bind an invalid IBO");
        AVA_CHECK(commandBuffer->pipelineCurrentlyBound, "Cannot bind an IBO when a pipeline has not yet been bound");

        commandBuffer->commandBuffer.bindIndexBuffer(ibo->buffer->buffer, 0u, ibo->indexType);
        commandBuffer->lastBoundIndexBufferIndexCount = ibo->indexCount;
    }
}
