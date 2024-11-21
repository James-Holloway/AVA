#ifndef AVA_RAII_BUFFER_HPP
#define AVA_RAII_BUFFER_HPP

#include "types.hpp"
#include "../bufferLocation.hpp"

namespace ava::raii
{
    class Buffer
    {
    public:
        Buffer(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, BufferLocation bufferLocation = BufferLocation::eGpuOnly, vk::DeviceSize alignment = 0);
        explicit Buffer(const ava::Buffer& existingBuffer);
        virtual ~Buffer();

        ava::Buffer buffer;

        [[nodiscard]] ava::Buffer getBuffer() const;
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
            update(reinterpret_cast<void*>(data.data()), data.size() * sizeof(T), offset);
        }

        template <typename T>
        void update(const std::vector<T>& data, const vk::DeviceSize offset = 0) const
        {
            update(reinterpret_cast<void*>(data.data()), data.size() * sizeof(T), offset);
        }

        static Pointer<Buffer> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags bufferUsage, BufferLocation bufferLocation = BufferLocation::eGpuOnly, vk::DeviceSize alignment = 0);
        static Pointer<Buffer> createUniformBuffer(vk::DeviceSize size, vk::BufferUsageFlags extraBufferUsage = {}, BufferLocation = BufferLocation::eCpuToGpu, vk::DeviceSize alignment = 0);
    };
}

#endif
