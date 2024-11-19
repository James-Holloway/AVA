#ifndef AVA_VERTEXATTRIBUTE_HPP
#define AVA_VERTEXATTRIBUTE_HPP

#include "detail/vulkan.hpp"

namespace ava
{
    struct VertexAttribute
    {
        constexpr static uint32_t AUTO_LOCATION = 0xFFFF'FFFF;
        constexpr static uint32_t AUTO_OFFSET = 0xFFFF'FFFF;

        vk::Format format = vk::Format::eUndefined;
        uint32_t location = AUTO_LOCATION;
        uint32_t offset = AUTO_OFFSET;
        uint32_t binding = 0;

        constexpr static VertexAttribute CreateFloat(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR32Sfloat, location, offset, binding};
        }

        constexpr static VertexAttribute CreateVec2(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR32G32Sfloat, location, offset, binding};
        }

        constexpr static VertexAttribute CreateVec3(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR32G32B32Sfloat, location, offset, binding};
        }

        constexpr static VertexAttribute CreateVec4(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR32G32B32A32Sfloat, location, offset, binding};
        }

        constexpr static VertexAttribute CreateUint(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR32Uint, location, offset, binding};
        }

        constexpr static VertexAttribute CreateUshort(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR16Uint, location, offset, binding};
        }

        constexpr static VertexAttribute CreateByte(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR8Uint, location, offset, binding};
        }

        constexpr static VertexAttribute CreateByteVec2(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR8G8Uint, location, offset, binding};
        }

        constexpr static VertexAttribute CreateByteVec3(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR8G8B8Uint, location, offset, binding};
        }

        constexpr static VertexAttribute CreateByteVec4(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR8G8B8A8Uint, location, offset, binding};
        }

        constexpr static VertexAttribute CreateByteUNorm(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR8Unorm, location, offset, binding};
        }

        constexpr static VertexAttribute CreateByteVec2UNorm(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR8G8Unorm, location, offset, binding};
        }

        constexpr static VertexAttribute CreateByteVec3UNorm(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR8G8B8Unorm, location, offset, binding};
        }

        constexpr static VertexAttribute CreateByteVec4UNorm(const uint32_t location = AUTO_LOCATION, const uint32_t offset = AUTO_OFFSET, const uint32_t binding = 0)
        {
            return VertexAttribute{vk::Format::eR8G8B8A8Unorm, location, offset, binding};
        }
    };
}

#endif
