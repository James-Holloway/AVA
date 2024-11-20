#ifndef AVA_IBO_HPP
#define AVA_IBO_HPP

#include "types.hpp"

namespace ava
{
    IBO createIBO(const uint32_t* indices, uint32_t indexCount);
    IBO createIBO(const uint16_t* indices, uint32_t indexCount);

    IBO createIBO(std::span<const uint32_t> indices);
    IBO createIBO(std::span<const uint16_t> indices);

    void destroyIBO(IBO& ibo);

    void bindIBO(const CommandBuffer& commandBuffer, const IBO& ibo);
}

#endif
