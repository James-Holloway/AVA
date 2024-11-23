#ifndef AVA_RAII_BUFFER_HPP
#define AVA_RAII_BUFFER_HPP

#include "types.hpp"
#include "../memoryLocation.hpp"

namespace ava::raii
{
    class Buffer
    {
    public:
        using Ptr = Pointer<Buffer>;

        Buffer(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, MemoryLocation bufferLocation = MemoryLocation::eGpuOnly, vk::DeviceSize alignment = 0);
        explicit Buffer(const ava::Buffer& existingBuffer);
        virtual ~Buffer();

        ava::Buffer buffer;

        Buffer(const Buffer& other) = delete;
        Buffer& operator=(Buffer& other) = delete;
        Buffer(Buffer&& other) noexcept;
        Buffer& operator=(Buffer&& other) noexcept;

        [[nodiscard]] vk::DeviceSize getBufferSize() const;
        [[nodiscard]] vk::BufferUsageFlags getBufferUsage() const;

        void update(const void* data, vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0) const;
        // Update buffer via a staging buffer
        void update(const Pointer<CommandBuffer>& commandBuffer, const Pointer<Buffer>& stagingBuffer, vk::DeviceSize offset = 0) const;

        template <typename T>
        void update(const T& data, const vk::DeviceSize offset = 0) const
        {
            update(&data, sizeof(T), offset);
        }

        template <typename T>
        void update(const std::span<T> data, const vk::DeviceSize offset = 0) const
        {
            update(reinterpret_cast<const void*>(data.data()), data.size() * sizeof(T), offset);
        }

        template <typename T>
        void update(const std::vector<T>& data, const vk::DeviceSize offset = 0) const
        {
            update(reinterpret_cast<const void*>(data.data()), data.size() * sizeof(T), offset);
        }

        static Pointer<Buffer> create(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, MemoryLocation bufferLocation = MemoryLocation::eGpuOnly, vk::DeviceSize alignment = 0);
        static Pointer<Buffer> createUniform(vk::DeviceSize size, vk::BufferUsageFlags extraBufferUsage = {}, MemoryLocation = MemoryLocation::eCpuToGpu, vk::DeviceSize alignment = 0);
    };
}

#endif
