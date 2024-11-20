#ifndef AVA_FRAMEBUFFER_HPP
#define AVA_FRAMEBUFFER_HPP

#include "types.hpp"
#include "renderPass.hpp"

namespace ava
{
    // Top level attachment vector is for each framebuffer to create. Next level attachment vector is for each attachment in the framebuffer
    Framebuffer createFramebuffer(const RenderPass& renderPass, std::vector<std::vector<vk::ImageView>> attachments, vk::Extent2D extent, int layers = 1);
    Framebuffer createSwapchainFramebuffer(const RenderPass& renderPass);
    void destroyFramebuffer(Framebuffer& frameBuffer);
}


#endif
