#ifndef AVA_HPP
#define AVA_HPP

#include "creation.hpp"
#include "renderPass.hpp"
#include "./vulkan.hpp"
#include "shaders.hpp"
#include "graphics.hpp"
#include "compute.hpp"
#include "framebuffer.hpp"
#include "commandBuffer.hpp"
#include "buffer.hpp"
#include "image.hpp"
#include "sampler.hpp"
#include "frame.hpp"
#include "vao.hpp"
#include "descriptors.hpp"
#include "ibo.hpp"
#include "vbo.hpp"

namespace ava
{
    constexpr inline Version AVAVersion{0, 1, 0};

    uint32_t getCurrentFrame();
    uint32_t getImageIndex();
    uint32_t getFramesInFlight();
    void deviceWaitIdle();
    vk::Extent2D getSwapchainExtent();
    uint32_t getSwapchainImageCount();
}

#endif
