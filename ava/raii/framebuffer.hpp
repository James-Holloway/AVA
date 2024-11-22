#ifndef AVA_RAII_FRAMEBUFFER_HPP
#define AVA_RAII_FRAMEBUFFER_HPP

#include "types.hpp"

namespace ava::raii
{
    class Framebuffer
    {
    public:
        explicit Framebuffer(const ava::Framebuffer& existingFramebuffer);
        ~Framebuffer();

        ava::Framebuffer framebuffer;

        static Pointer<Framebuffer> create(const Pointer<RenderPass>& renderPass, const std::vector<std::vector<vk::ImageView>>& attachments, vk::Extent2D extent, int layers = 1);
        static Pointer<Framebuffer> create(const Pointer<RenderPass>& renderPass, const std::vector<std::vector<Pointer<ImageView>>>& attachments, vk::Extent2D extent, int layers = 1);
        static Pointer<Framebuffer> createSwapchain(const Pointer<RenderPass>& renderPass);
    };
}
#endif
