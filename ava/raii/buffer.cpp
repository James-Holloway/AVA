#include "buffer.hpp"
#include "ava/buffer.hpp"
#include "ava/detail/buffer.hpp"

#include "commandBuffer.hpp"
#include "ava/detail/detail.hpp"

namespace ava::raii
{
    Buffer::Buffer(const vk::DeviceSize size, const vk::BufferUsageFlags bufferUsage, const BufferLocation bufferLocation, const vk::DeviceSize alignment)
    {
        buffer = ava::createBuffer(size, bufferUsage, bufferLocation, alignment);
    }

    Buffer::Buffer(const ava::Buffer& existingBuffer)
    {
        AVA_CHECK(existingBuffer != nullptr && existingBuffer->buffer, "Cannot create a RAII buffer from an invalid buffer");
        buffer = existingBuffer;
    }

    Buffer::~Buffer()
    {
        if (buffer != nullptr)
        {
            destroyBuffer(buffer);
        }
    }

    vk::DeviceSize Buffer::getBufferSize() const
    {
        AVA_CHECK(buffer != nullptr, "Cannot get buffer size when buffer is invali");

        return buffer->allocationInfo.size;
    }

    vk::BufferUsageFlags Buffer::getBufferUsage() const
    {
        AVA_CHECK(buffer != nullptr, "Cannot get buffer usage when buffer is invalid");

        return buffer->bufferUsage;
    }

    void Buffer::update(const void* data, const vk::DeviceSize size, const vk::DeviceSize offset) const
    {
        ava::updateBuffer(buffer, data, size, offset);
    }

    void Buffer::update(const Pointer<CommandBuffer>& commandBuffer, const Pointer<Buffer>& stagingBuffer, const vk::DeviceSize offset) const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot update buffer with staging buffer when command buffer is invalid");
        ava::updateBuffer(buffer, commandBuffer->commandBuffer, stagingBuffer->buffer, offset);
    }

    Pointer<Buffer> Buffer::create(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, BufferLocation bufferLocation, vk::DeviceSize alignment)
    {
        return std::make_shared<Buffer>(size, bufferUsage, bufferLocation, alignment);
    }

    Pointer<Buffer> Buffer::createUniform(vk::DeviceSize size, const vk::BufferUsageFlags extraBufferUsage, BufferLocation bufferLocation, vk::DeviceSize alignment)
    {
        return std::make_shared<Buffer>(size, DEFAULT_UNIFORM_BUFFER_USAGE | extraBufferUsage, bufferLocation, alignment);
    }
}
