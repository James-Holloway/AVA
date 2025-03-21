#include "buffer.hpp"

#include "detail/buffer.hpp"

#include "commandBuffer.hpp"
#include "detail/commandBuffer.hpp"
#include "detail/detail.hpp"
#include "detail/state.hpp"

namespace ava
{
    Buffer createBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags bufferUsage, const MemoryLocation bufferLocation, const vk::DeviceSize alignment)
    {
        AVA_CHECK(size > 0, "Cannot create a buffer with a size of 0");
        AVA_CHECK(detail::State.device, "Cannot create buffer when State's device is invalid");
        AVA_CHECK(detail::State.allocator, "Cannot create buffer when State's allocator is invalid");

        vk::BufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.size = size;
        bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
        bufferCreateInfo.usage = bufferUsage;

        // If shader device address is enabled then every buffer allocation gets the flag
        if (detail::State.shaderDeviceAddressEnabled)
        {
            bufferCreateInfo.usage |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
        }

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
        if (bufferLocation == MemoryLocation::eCpuToGpu)
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
        outBuffer->size = size;

        return outBuffer;
    }

    void destroyBuffer(Buffer& buffer)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(buffer != nullptr, "Cannot destroy invalid buffer");
        AVA_CHECK_NO_EXCEPT_RETURN(detail::State.device, "Cannot destroy buffer when State's device is invalid");

        if (buffer->buffer)
        {
            if (buffer->allocation)
            {
                AVA_CHECK_NO_EXCEPT_RETURN(detail::State.allocator, "Cannot destroy buffer when State's allocator is invalid");

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

    Buffer createUniformBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags extraBufferUsage, const MemoryLocation bufferLocation, const vk::DeviceSize alignment)
    {
        return createBuffer(size, DEFAULT_UNIFORM_BUFFER_USAGE | extraBufferUsage, bufferLocation, alignment);
    }

    Buffer createVertexBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags extraBufferUsage, const MemoryLocation bufferLocation, const vk::DeviceSize alignment)
    {
        return createBuffer(size, DEFAULT_VERTEX_BUFFER_USAGE | extraBufferUsage, bufferLocation, alignment);
    }

    Buffer createIndexBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags extraBufferUsage, const MemoryLocation bufferLocation, const vk::DeviceSize alignment)
    {
        return createBuffer(size, DEFAULT_INDEX_BUFFER_USAGE | extraBufferUsage, bufferLocation, alignment);
    }

    void updateBuffer(const Buffer& buffer, const void* data, vk::DeviceSize size, const vk::DeviceSize offset)
    {
        AVA_CHECK(buffer != nullptr && buffer->buffer, "Cannot update an invalid buffer");
        AVA_CHECK((size + offset) <= buffer->size, "Cannot update buffer where size + offset (" + std::to_string((size + offset)) +") exceeds buffer size (" + std::to_string(buffer->size) + ")");
        AVA_CHECK(data != nullptr, "Cannot update buffer when buffer data is nullptr");

        // If CPU mapped then write to such mapped pointer
        if (buffer->bufferLocation == MemoryLocation::eCpuToGpu && buffer->mapped != nullptr)
        {
            AVA_CHECK(detail::State.allocator, "Cannot update CpuToGpu buffer when State's allocator is invalid");

            if (size == vk::WholeSize)
            {
                size = buffer->size - offset;
            }
            std::memcpy(reinterpret_cast<void*>(reinterpret_cast<size_t>(buffer->mapped) + offset), data, size);
            detail::State.allocator.flushAllocation(buffer->allocation, offset, size);
            return;
        }

        AVA_CHECK((buffer->bufferUsage & vk::BufferUsageFlagBits::eTransferDst) != vk::BufferUsageFlags{}, "Cannot update Gpu Only buffer with a staging buffer & single time command when buffer does not have any TransferDst BufferUsage");

        // Create staging buffer
        auto stagingBuffer = createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, MemoryLocation::eCpuToGpu, 0);
        updateBuffer(stagingBuffer, data, size, 0);

        // Update via single time command-buffer buffer-copy
        auto commandBuffer = beginSingleTimeCommands(vk::QueueFlagBits::eTransfer);
        updateBuffer(buffer, commandBuffer, stagingBuffer, offset, size);
        endSingleTimeCommands(commandBuffer);

        // Destroy staging buffer
        destroyBuffer(stagingBuffer);
    }

    void updateBuffer(const Buffer& buffer, const CommandBuffer& commandBuffer, const Buffer& stagingBuffer, const vk::DeviceSize offset, vk::DeviceSize size)
    {
        AVA_CHECK(buffer != nullptr && buffer->buffer, "Cannot update an invalid buffer");
        AVA_CHECK(stagingBuffer != nullptr && stagingBuffer->buffer, "Cannot update a buffer with an invalid staging buffer");
        AVA_CHECK((stagingBuffer->size + offset) <= buffer->size, "Cannot update buffer where size + offset (" + std::to_string(stagingBuffer->size) + ") exceeds buffer size (" + std::to_string(buffer->size) + ")");
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot update buffer with an invalid command buffer");
        AVA_CHECK(size > 0, "Cannot update buffer when size is 0");

        if (size == vk::WholeSize)
        {
            size = stagingBuffer->size;
        }

        const vk::BufferCopy copyRegion{0, offset, size};
        commandBuffer->commandBuffer.copyBuffer(stagingBuffer->buffer, buffer->buffer, copyRegion);
    }

    void insertBufferMemoryBarrier(const CommandBuffer& commandBuffer, const Buffer& buffer, const vk::PipelineStageFlags srcStage, const vk::PipelineStageFlags dstStage, const vk::AccessFlags srcAccessMask, const vk::AccessFlags dstAccessMask, const vk::DeviceSize size, const vk::DeviceSize offset)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot insert buffer memory barrier when command buffer is invalid")
        AVA_CHECK(buffer != nullptr && buffer->buffer, "Cannot insert buffer memory barrier when buffer is invalid")

        vk::BufferMemoryBarrier barrier{};
        barrier.buffer = buffer->buffer;
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;
        barrier.size = size;
        barrier.offset = offset;
        barrier.srcQueueFamilyIndex = commandBuffer->familyQueueIndex;
        barrier.dstQueueFamilyIndex = commandBuffer->familyQueueIndex;

        constexpr vk::DependencyFlags dependencyFlags{};

        commandBuffer->commandBuffer.pipelineBarrier(srcStage, dstStage, dependencyFlags, nullptr, barrier, nullptr);
    }
}
