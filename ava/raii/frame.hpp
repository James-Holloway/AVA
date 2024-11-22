#ifndef AVA_RAII_FRAME_HPP
#define AVA_RAII_FRAME_HPP

#include "types.hpp"

namespace ava::raii
{
    // Returns nullptr on failure
    [[nodiscard]] Pointer<CommandBuffer> startFrame(uint32_t* currentFrame = nullptr, uint32_t* imageIndex = nullptr);
}

#endif
