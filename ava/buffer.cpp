#include "buffer.hpp"

#include "detail/buffer.hpp"

#include "commandBuffer.hpp"
#include "detail/commandBuffer.hpp"
#include "detail/detail.hpp"
#include "detail/state.hpp"

namespace ava
{
    vma::MemoryUsage getMemoryUsageFromBufferLocation(const BufferLocation bufferLocation)
    {
        switch (bufferLocation)
        {
        default:
        case BufferLocation::eGpuOnly:
            return vma::MemoryUsage::eGpuOnly;
        case BufferLocation::eCpuToGpu:
            return vma::MemoryUsage::eCpuToGpu;
        }
    }

    Buffer createBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags bufferUsage, const BufferLocation bufferLocation, const vk::DeviceSize alignment)
    {
        AVA_CHECK(size > 0, "Cannot create a buffer with a size of 0");
        AVA_CHECK(detail::State.device, "Cannot create buffer when State's device is invalid");
        AVA_CHECK(detail::State.allocator, "Cannot create buffer when State's allocator is invalid");

        vk::BufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.size = size;
        bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
        bufferCreateInfo.usage = bufferUsage;

        vma::AllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = getMemoryUsageFromBufferLocation(bufferLocation);
        vk::Buffer buffer;
        vma::Allocation allocation;
        if (alignment == 0)
        {
            const auto [first, second] = detail::State.allocator.createBuffer(bufferCreateInfo, allocationCreateInfo);
            buffer = first;
            allocation = second;
        }
        else
        {
            const auto [first, second] = detail::State.allocator.createBufferWithAlignment(bufferCreateInfo, allocationCreateInfo, alignment);
            buffer = first;
            allocation = second;
        }
        const auto allocationInfo = detail::State.allocator.getAllocationInfo(allocation);
        void* mapped = nullptr;
        if (bufferLocation == BufferLocation::eCpuToGpu)
        {
            mapped = detail::State.allocator.mapMemory(allocation);
            std::memset(mapped, 0, size);
        }

        const auto outBuffer = new detail::Buffer();
        outBuffer->buffer = buffer;
        outBuffer->allocation = allocation;
        outBuffer->allocationInfo = allocationInfo;
        outBuffer->bufferUsage = bufferUsage;
        outBuffer->bufferLocation = bufferLocation;
        outBuffer->alignment = alignment;
        outBuffer->mapped = mapped;

        return outBuffer;
    }

    void destroyBuffer(Buffer& buffer)
    {
        AVA_CHECK(buffer != nullptr, "Cannot destroy invalid buffer");
        AVA_CHECK(detail::State.device, "Cannot destroy buffer when State's device is invalid");

        if (buffer->buffer)
        {
            if (buffer->allocation)
            {
                AVA_CHECK(detail::State.allocator, "Cannot destroy buffer when State's allocator is invalid");

                if (buffer->mapped != nullptr)
                {
                    detail::State.allocator.unmapMemory(buffer->allocation);
                }
                detail::State.allocator.destroyBuffer(buffer->buffer, buffer->allocation);
            }
            else
            {
                detail::State.device.destroyBuffer(buffer->buffer);
            }
        }

        delete buffer;
        buffer = nullptr;
    }

    Buffer createUniformBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags extraBufferUsage, const BufferLocation bufferLocation, const vk::DeviceSize alignment)
    {
        return createBuffer(size, DEFAULT_UNIFORM_BUFFER_USAGE | extraBufferUsage, bufferLocation, alignment);
    }

    Buffer createVertexBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags extraBufferUsage, const BufferLocation bufferLocation, const vk::DeviceSize alignment)
    {
        return createBuffer(size, DEFAULT_VERTEX_BUFFER_USAGE | extraBufferUsage, bufferLocation, alignment);
    }

    Buffer createIndexBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags extraBufferUsage, const BufferLocation bufferLocation, const vk::DeviceSize alignment)
    {
        return createBuffer(size, DEFAULT_INDEX_BUFFER_USAGE | extraBufferUsage, bufferLocation, alignment);
    }

    void updateBuffer(const Buffer& buffer, const void* data, vk::DeviceSize size, const vk::DeviceSize offset)
    {
        AVA_CHECK(buffer != nullptr && buffer->buffer, "Cannot update an invalid buffer");
        AVA_CHECK((size + offset) <= buffer->allocationInfo.size, "Cannot update buffer where size + offset (" + std::to_string((size + offset)) +") exceeds buffer size (" + std::to_string(buffer->allocationInfo.size) + ")");
        AVA_CHECK(data != nullptr, "Cannot update buffer when buffer data is nullptr");

        // If CPU mapped then write to such mapped pointer
        if (buffer->bufferLocation == BufferLocation::eCpuToGpu && buffer->mapped != nullptr)
        {
            AVA_CHECK(detail::State.allocator, "Cannot update CpuToGpu buffer when State's allocator is invalid");

            if (size == vk::WholeSize)
            {
                size = buffer->allocationInfo.size - offset;
            }
            std::memcpy(reinterpret_cast<void*>(reinterpret_cast<size_t>(buffer->mapped) + offset), data, size);
            detail::State.allocator.flushAllocation(buffer->allocation, offset, size);
            return;
        }

        AVA_CHECK((buffer->bufferUsage & vk::BufferUsageFlagBits::eTransferDst) != vk::BufferUsageFlags{}, "Cannot update Gpu Only buffer with a staging buffer & single time command when buffer does not have any TransferDst BufferUsage");

        // Create staging buffer
        auto stagingBuffer = createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, BufferLocation::eCpuToGpu, 0);
        updateBuffer(stagingBuffer, data, size, offset);

        // Update via single time command-buffer buffer-copy
        auto commandBuffer = beginSingleTimeCommands(vk::QueueFlagBits::eTransfer);
        updateBuffer(buffer, commandBuffer, stagingBuffer, offset);
        endSingleTimeCommands(commandBuffer);

        // Destroy staging buffer
        destroyBuffer(stagingBuffer);
    }

    void updateBuffer(const Buffer& buffer, const CommandBuffer commandBuffer, const Buffer& stagingBuffer, const vk::DeviceSize offset)
    {
        AVA_CHECK(buffer != nullptr && buffer->buffer, "Cannot update an invalid buffer");
        AVA_CHECK(stagingBuffer != nullptr && stagingBuffer->buffer, "Cannot update a buffer with an invalid staging buffer");
        AVA_CHECK((stagingBuffer->allocationInfo.size + offset) <= buffer->allocationInfo.size, "Cannot update buffer where size + offset (" + std::to_string(stagingBuffer->allocationInfo.size) + ") exceeds buffer size (" + std::to_string(buffer->allocationInfo.size) + ")");
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot update buffer with an invalid command buffer");

        const vk::BufferCopy copyRegion{0, offset, stagingBuffer->allocationInfo.size};
        commandBuffer->commandBuffer.copyBuffer(stagingBuffer->buffer, buffer->buffer, copyRegion);
    }
}
