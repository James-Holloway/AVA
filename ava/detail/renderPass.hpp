#ifndef AVA_DETAIL_RENDERPASS_HPP
#define AVA_DETAIL_RENDERPASS_HPP

#include "./vulkan.hpp"

namespace ava::detail
{
    struct RenderPass
    {
        vk::RenderPass renderPass;
        uint32_t attachmentCount = 0;
        uint32_t subpasses = 0;
        std::vector<uint32_t> subpassColorAttachmentCounts;
    };

    struct Framebuffer
    {
        std::vector<vk::Framebuffer> framebuffers;
        vk::Extent2D extent;
        uint32_t layers = 0;
        uint32_t attachmentCount = 0;
    };
}


#endif
