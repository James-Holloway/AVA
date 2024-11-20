#ifndef AVA_DETAIL_BUFFER_HPP
#define AVA_DETAIL_BUFFER_HPP

#include "./vulkan.hpp"
#include "../bufferLocation.hpp"

namespace ava::detail
{
    struct Buffer
    {
        vk::Buffer buffer;
        vma::Allocation allocation;
        vma::AllocationInfo allocationInfo;
        vk::BufferUsageFlags bufferUsage;
        BufferLocation bufferLocation;
        vk::DeviceSize alignment;
        void* mapped;
    };
}

#endif