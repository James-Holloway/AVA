#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>

#include "ava.hpp"

#include "detail/detail.hpp"
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
        AVA_CHECK(State.device, "Cannot wait idle on an invalid device");
        State.device.waitIdle();
    }

    vk::Extent2D getSwapchainExtent()
    {
        return State.swapchainExtent;
    }

    uint32_t getSwapchainImageCount()
    {
        return State.swapchainImageCount;
    }
}
