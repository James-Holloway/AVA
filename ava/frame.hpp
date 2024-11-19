#ifndef AVA_FRAME_HPP
#define AVA_FRAME_HPP

#include <cstdint>

#include "detail/vulkan.hpp"

namespace ava
{
    // Returns an un-started & un-reset graphics command buffer when the operation was successful, otherwise it will have no value
    // currentFrame and imageIndex are optional outs
    std::optional<vk::CommandBuffer> startFrame(uint32_t* currentFrame = nullptr, uint32_t* imageIndex = nullptr);

    // Presents the frame using the current frame's graphics command buffer
    void presentFrame();

    // Returns if a swapchain resize is required. Check before starting and after presenting a frame. Recreate any swapchain-sized images if true
    bool resizeNeeded();
}

#endif
