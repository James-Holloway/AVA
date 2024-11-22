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

    Pointer<Framebuffer> Framebuffer::create(const Pointer<RenderPass>& renderPass, const std::vector<std::vector<vk::ImageView>>& attachments, const vk::Extent2D extent, const int layers)
    {
        AVA_CHECK(renderPass != nullptr && renderPass->renderPass != nullptr, "Cannot create a framebuffer with an invalid render pass");
        return std::make_shared<Framebuffer>(ava::createFramebuffer(renderPass->renderPass, attachments, extent, layers));
    }

    Pointer<Framebuffer> Framebuffer::create(const Pointer<RenderPass>& renderPass, const std::vector<std::vector<Pointer<ImageView>>>& attachments, vk::Extent2D extent, int layers)
    {
        AVA_CHECK(renderPass != nullptr && renderPass->renderPass != nullptr, "Cannot create a framebuffer with an invalid render pass");

        std::vector<std::vector<ava::ImageView>> imageViews;
        imageViews.reserve(attachments.size());
        for (auto& frame : attachments)
        {
            imageViews.push_back(std::vector<ava::ImageView>());
            auto& outFrame = imageViews.back();
            outFrame.reserve(frame.size());

            for (auto& attachment : frame)
            {
                if (attachment != nullptr)
                {
                    outFrame.push_back(attachment->imageView);
                }
                else
                {
                    outFrame.push_back(nullptr);
                }
            }
        }

        return std::make_shared<Framebuffer>(ava::createFramebuffer(renderPass->renderPass, imageViews, extent, layers));
    }

    Pointer<Framebuffer> Framebuffer::createSwapchain(const Pointer<RenderPass>& renderPass)
    {
        AVA_CHECK(renderPass != nullptr && renderPass->renderPass != nullptr, "Cannot create a framebuffer with an invalid render pass");
        return std::make_shared<Framebuffer>(ava::createSwapchainFramebuffer(renderPass->renderPass));
    }
}
