#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.hpp>

#include "ava.hpp"

#include "detail/state.hpp"

namespace ava
{
    using namespace detail;

    uint32_t getCurrentFrame()
    {
        return State.currentFrame;
    }

    uint32_t getImageIndex()
    {
        return State.imageIndex;
    }

    uint32_t getFramesInFlight()
    {
        return State.framesInFlight;
    }

    void deviceWaitIdle()
    {
        State.device.waitIdle();
    }
}
