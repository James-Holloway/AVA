#ifndef AVA_RAII_TYPES_HPP
#define AVA_RAII_TYPES_HPP

#include "../types.hpp"

namespace ava::raii
{
    class Buffer;
    class CommandBuffer;

    template <typename T>
    using Pointer = std::shared_ptr<T>;

    template <typename T>
    using WeakPointer = std::weak_ptr<T>;
}

#endif