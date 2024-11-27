#ifndef AVA_DETAIL_VBO_HPP
#define AVA_DETAIL_VBO_HPP

#include "../types.hpp"

namespace ava::detail
{
    struct VBO
    {
        ava::Buffer buffer;
        uint32_t vertexCount;
        uint32_t stride;
        uint32_t binding;
        vk::PrimitiveTopology topology;
    };
}

#endif
