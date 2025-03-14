#include "framebuffer.hpp"
#include "ava/framebuffer.hpp"

#include "image.hpp"
#include "renderPass.hpp"
#include "ava/detail/detail.hpp"
#include "ava/detail/renderPass.hpp"

namespace ava::raii
{
    Framebuffer::Framebuffer(const ava::Framebuffer& existingFramebuffer)
    {
        AVA_CHECK(existingFramebuffer != nullptr, "Cannot create RAII framebuffer from an invalid framebuffer");
        framebuffer = existingFramebuffer;
    }

    Framebuffer::~Framebuffer()
    {
        if (framebuffer != nullptr)
        {
            ava::destroyFramebuffer(framebuffer);
        }
    }

    Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    {
        framebuffer = other.framebuffer;
        other.framebuffer = nullptr;
    }

    Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
    {
        if (this != &other)
        {
            framebuffer = other.framebuffer;
            other.framebuffer = nullptr;
        }
        return *this;
    }

    Pointer<Framebuffer> Framebuffer::create(const Pointer<RenderPass>& renderPass, const std::vector<vk::ImageView>& attachments, const vk::Extent2D extent, const int layers)
    {
        AVA_CHECK(renderPass != nullptr && renderPass->renderPass != nullptr, "Cannot create a framebuffer with an invalid render pass");
        return std::make_shared<Framebuffer>(ava::createFramebuffer(renderPass->renderPass, attachments, extent, layers));
    }

    Pointer<Framebuffer> Framebuffer::create(const Pointer<RenderPass>& renderPass, const std::vector<Pointer<ImageView>>& attachments, vk::Extent2D extent, int layers)
    {
        AVA_CHECK(renderPass != nullptr && renderPass->renderPass != nullptr, "Cannot create a framebuffer with an invalid render pass");

        std::vector<ava::ImageView> imageViews;
        imageViews.reserve(attachments.size());
        for (auto& attachment : attachments)
        {
            if (attachment != nullptr)
            {
                imageViews.push_back(attachment->imageView);
            }
            else
            {
                imageViews.push_back(nullptr);
            }
        }

        return std::make_shared<Framebuffer>(ava::createFramebuffer(renderPass->renderPass, imageViews, extent, layers));
    }

    std::vector<Pointer<Framebuffer>> Framebuffer::create(std::vector<ava::Framebuffer> framebuffers)
    {
        std::vector<Pointer<Framebuffer>> outFramebuffers;
        outFramebuffers.reserve(framebuffers.size());
        for (auto& framebuffer : framebuffers)
        {
            outFramebuffers.push_back(std::make_shared<Framebuffer>(framebuffer));
        }

        return outFramebuffers;
    }

    std::vector<Pointer<Framebuffer>> Framebuffer::createSwapchain(const Pointer<RenderPass>& renderPass)
    {
        AVA_CHECK(renderPass != nullptr && renderPass->renderPass != nullptr, "Cannot create a framebuffer with an invalid render pass");

        auto framebuffers = ava::createSwapchainFramebuffers(renderPass->renderPass);

        std::vector<Pointer<Framebuffer>> outFramebuffers;
        outFramebuffers.reserve(framebuffers.size());
        for (auto& framebuffer : framebuffers)
        {
            outFramebuffers.push_back(std::make_shared<Framebuffer>(framebuffer));
        }

        return outFramebuffers;
    }
}
