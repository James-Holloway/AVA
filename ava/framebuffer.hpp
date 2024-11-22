#ifndef AVA_FRAMEBUFFER_HPP
#define AVA_FRAMEBUFFER_HPP

#include "types.hpp"
#include "renderPass.hpp"

namespace ava
{
    // Top level attachment vector is for each framebuffer to create. Next level attachment vector is for each attachment in the framebuffer
    [[nodiscard]] Framebuffer createFramebuffer(const RenderPass& renderPass, std::vector<std::vector<vk::ImageView>> attachments, vk::Extent2D extent, int layers = 1);
    [[nodiscard]] Framebuffer createFramebuffer(const RenderPass& renderPass, const std::vector<std::vector<ava::ImageView>>& attachments, vk::Extent2D extent, int layers = 1);
    [[nodiscard]] Framebuffer createSwapchainFramebuffer(const RenderPass& renderPass);
    void destroyFramebuffer(Framebuffer& frameBuffer);
}


#endif