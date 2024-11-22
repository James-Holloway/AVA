#include "framebuffer.hpp"

#include "detail/detail.hpp"
#include "detail/image.hpp"
#include "detail/renderPass.hpp"
#include "detail/state.hpp"

namespace ava
{
    Framebuffer createFramebuffer(const RenderPass& renderPass, const std::vector<vk::ImageView>& attachments, const vk::Extent2D extent, const int layers)
    {
        AVA_CHECK(detail::State.device != nullptr, "Cannot create a framebuffer without a State device");
        AVA_CHECK(renderPass != nullptr && renderPass->renderPass, "Cannot create a framebuffer using an invalid render pass");
        AVA_CHECK(!attachments.empty(), "Cannot create a framebuffer using an empty attachments vector");
        AVA_CHECK(extent.width > 0 && extent.height > 0, "Cannot create a framebuffer using an invalid extent");

        vk::FramebufferCreateInfo framebufferCreateInfo{};

        // ReSharper disable once CppDFANullDereference
        framebufferCreateInfo.renderPass = renderPass->renderPass;
        framebufferCreateInfo.setAttachments(attachments);
        framebufferCreateInfo.width = extent.width;
        framebufferCreateInfo.height = extent.height;
        framebufferCreateInfo.layers = layers;
        framebufferCreateInfo.flags = {};

        const auto framebuffer = detail::State.device.createFramebuffer(framebufferCreateInfo);

        // ReSharper disable once CppDFAMemoryLeak
        const auto outFramebuffer = new detail::Framebuffer();
        outFramebuffer->framebuffer = framebuffer;
        outFramebuffer->extent = extent;
        outFramebuffer->layers = layers;
        outFramebuffer->attachmentCount = static_cast<uint32_t>(attachments.size());

        return outFramebuffer;
    }

    Framebuffer createFramebuffer(const RenderPass& renderPass, const std::vector<ava::ImageView>& attachments, const vk::Extent2D extent, const int layers)
    {
        std::vector<vk::ImageView> imageViews;
        imageViews.reserve(attachments.size());
        for (auto& attachment : attachments)
        {
            if (attachment != nullptr)
            {
                imageViews.push_back(attachment->imageView);
            }
            else
            {
                imageViews.emplace_back(nullptr);
            }
        }

        return createFramebuffer(renderPass, imageViews, extent, layers);
    }

    std::vector<Framebuffer> createSwapchainFramebuffers(const RenderPass& renderPass)
    {
        AVA_CHECK(detail::State.swapchain, "Cannot create a swapchain framebuffer without a valid State swapchain");

        std::vector<Framebuffer> framebuffers;
        framebuffers.reserve(detail::State.swapchainImageViews.size());
        for (auto& imageView : detail::State.swapchainImageViews)
        {
            framebuffers.push_back(createFramebuffer(renderPass, {imageView}, detail::State.swapchainExtent));
        }

        // ReSharper disable once CppDFAMemoryLeak
        return framebuffers;
    }

    void destroyFramebuffer(Framebuffer& framebuffer)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(framebuffer != nullptr, "Cannot destroy an invalid framebuffer");

        if (framebuffer->framebuffer)
        {
            detail::State.device.destroyFramebuffer(framebuffer->framebuffer);
        }

        delete framebuffer;
        framebuffer = nullptr;
    }

    void destroyFramebuffers(std::vector<Framebuffer>& framebuffers)
    {
        for (auto& framebuffer : framebuffers)
        {
            destroyFramebuffer(framebuffer);
        }
        framebuffers.clear();
    }
}
