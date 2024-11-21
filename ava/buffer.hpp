#ifndef AVA_BUFFER_HPP
#define AVA_BUFFER_HPP

#include "detail/vulkan.hpp"
#include "types.hpp"
#include "bufferLocation.hpp"

namespace ava
{
    constexpr vk::BufferUsageFlags DEFAULT_TRANSFER_BUFFER_USAGE = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst;
    constexpr vk::BufferUsageFlags DEFAULT_UNIFORM_BUFFER_USAGE = vk::BufferUsageFlagBits::eUniformBuffer | DEFAULT_TRANSFER_BUFFER_USAGE;
    constexpr vk::BufferUsageFlags DEFAULT_STORAGE_BUFFER_USAGE = vk::BufferUsageFlagBits::eStorageBuffer | DEFAULT_TRANSFER_BUFFER_USAGE;
    constexpr vk::BufferUsageFlags DEFAULT_VERTEX_BUFFER_USAGE = vk::BufferUsageFlagBits::eVertexBuffer | DEFAULT_TRANSFER_BUFFER_USAGE;
    constexpr vk::BufferUsageFlags DEFAULT_INDEX_BUFFER_USAGE = vk::BufferUsageFlagBits::eIndexBuffer | DEFAULT_TRANSFER_BUFFER_USAGE;
    constexpr vk::BufferUsageFlags DEFAULT_COMBINED_VERTEX_INDEX_BUFFER_USAGE = DEFAULT_VERTEX_BUFFER_USAGE | DEFAULT_INDEX_BUFFER_USAGE;

    // General buffer creation
    [[nodiscard]] Buffer createBuffer(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, BufferLocation bufferLocation = BufferLocation::eGpuOnly, vk::DeviceSize alignment = 0);
    // Destroys any buffer
    void destroyBuffer(Buffer& buffer);

    // More specialized buffer creation
    [[nodiscard]] Buffer createUniformBuffer(vk::DeviceSize size, vk::BufferUsageFlags extraBufferUsage = {}, BufferLocation bufferLocation = BufferLocation::eCpuToGpu, vk::DeviceSize alignment = 0);
    [[nodiscard]] Buffer createVertexBuffer(vk::DeviceSize size, vk::BufferUsageFlags extraBufferUsage = {}, BufferLocation bufferLocation = BufferLocation::eGpuOnly, vk::DeviceSize alignment = 0);
    [[nodiscard]] Buffer createIndexBuffer(vk::DeviceSize size, vk::BufferUsageFlags extraBufferUsage = {}, BufferLocation bufferLocation = BufferLocation::eGpuOnly, vk::DeviceSize alignment = 0);

    // Updating buffer data
    // Updates CpuToGpu buffers using mapped data, otherwise creates a single time command buffer and staging buffer to update GpuOnly data (requires TransferSrc)
    void updateBuffer(const Buffer& buffer, const void* data, vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
    // Update buffer via a staging buffer
    void updateBuffer(const Buffer& buffer, const CommandBuffer& commandBuffer, const Buffer& stagingBuffer, vk::DeviceSize offset = 0);

    template <typename T>
    void updateBuffer(const Buffer& buffer, const T& data, const vk::DeviceSize offset = 0)
    {
        updateBuffer(buffer, &data, sizeof(T), offset);
    }

    template <typename T>
    void updateBuffer(const Buffer& buffer, const std::span<T> data, const vk::DeviceSize offset = 0)
    {
        updateBuffer(buffer, reinterpret_cast<void*>(data.data()), data.size() * sizeof(T), offset);
    }

    template <typename T>
    void updateBuffer(const Buffer& buffer, const std::vector<T>& data, const vk::DeviceSize offset = 0)
    {
        updateBuffer(buffer, reinterpret_cast<void*>(data.data()), data.size() * sizeof(T), offset);
    }

    void insertBufferMemoryBarrier(const CommandBuffer& commandBuffer, const Buffer& buffer, vk::PipelineStageFlags srcStage, vk::PipelineStageFlagBits dstStage, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
}

#endif
