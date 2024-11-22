#include "frame.hpp"

#include "ava/frame.hpp"
#include "commandBuffer.hpp"

namespace ava::raii
{
    Pointer<CommandBuffer> startFrame(uint32_t* currentFrame, uint32_t* imageIndex)
    {
        auto cbRet = ava::startFrame(currentFrame, imageIndex);
        return cbRet.has_value() ? std::make_shared<CommandBuffer>(cbRet.value()) : nullptr;
    }
}
