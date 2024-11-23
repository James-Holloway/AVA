#ifndef AVA_RAII_FRAMEBUFFER_HPP
#define AVA_RAII_FRAMEBUFFER_HPP

#include "types.hpp"

namespace ava::raii
{
    class Framebuffer
    {
    public:
        using Ptr = Pointer<Framebuffer>;

        explicit Framebuffer(const ava::Framebuffer& existingFramebuffer);
        ~Framebuffer();

        ava::Framebuffer framebuffer;

        Framebuffer(const Framebuffer& other) = delete;
        Framebuffer& operator=(Framebuffer& other) = delete;
        Framebuffer(Framebuffer&& other) noexcept;
        Framebuffer& operator=(Framebuffer&& other) noexcept;

        static Pointer<Framebuffer> create(const Pointer<RenderPass>& renderPass, const std::vector<vk::ImageView>& attachments, vk::Extent2D extent, int layers = 1);
        static Pointer<Framebuffer> create(const Pointer<RenderPass>& renderPass, const std::vector<Pointer<ImageView>>& attachments, vk::Extent2D extent, int layers = 1);
        static std::vector<Pointer<Framebuffer>> create(std::vector<ava::Framebuffer> framebuffers);
        static std::vector<Pointer<Framebuffer>> createSwapchain(const Pointer<RenderPass>& renderPass);
    };
}
#endif
