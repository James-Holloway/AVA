#ifndef AVA_DETAIL_IBO_HPP
#define AVA_DETAIL_IBO_HPP

#include "../types.hpp"

namespace ava::detail
{
    struct IBO
    {
        ava::Buffer buffer;
        uint32_t indexCount;
        vk::IndexType indexType;
    };
}

#endif //IBO_HPP
