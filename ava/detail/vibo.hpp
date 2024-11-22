#ifndef AVA_DETAIL_VIBO_HPP
#define AVA_DETAIL_VIBO_HPP

#include "./vulkan.hpp"
#include "../types.hpp"

namespace ava::detail
{
    struct VIBO
    {
        ava::Buffer buffer;
        uint32_t vertexCount;
        uint32_t stride;
        uint32_t binding;
        vk::IndexType indexType;
        uint32_t indexCount;
        vk::DeviceSize vertexOffset;
        vk::DeviceSize indexOffset;
    };
}

#endif
