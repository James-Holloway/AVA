#include "framebuffer.hpp"

#include "detail/detail.hpp"
#include "detail/image.hpp"
#include "detail/renderPass.hpp"
#include "detail/state.hpp"

namespace ava
{
    // Framebuffer
    Framebuffer createFramebuffer(const RenderPass& renderPass, std::vector<std::vector<vk::ImageView>> attachments, const vk::Extent2D extent, const int layers)
    {
        AVA_CHECK(detail::State.device != nullptr, "Cannot create a framebuffer without a State device");
        AVA_CHECK(renderPass != nullptr && renderPass->renderPass, "Cannot create a framebuffer using an invalid render pass");
        AVA_CHECK(!attachments.empty(), "Cannot create a framebuffer using an empty attachments vector");
        AVA_CHECK(extent.width > 0 && extent.height > 0, "Cannot create a framebuffer using an invalid extent");

        std::vector<vk::Framebuffer> framebuffers;
        framebuffers.resize(attachments.size());
        for (uint32_t i = 0; i < framebuffers.size(); i++)
        {
            vk::FramebufferCreateInfo framebufferCreateInfo{};

            // ReSharper disable once CppDFANullDereference
            framebufferCreateInfo.renderPass = renderPass->renderPass;
            framebufferCreateInfo.setAttachments(attachments[i]);
            framebufferCreateInfo.width = extent.width;
            framebufferCreateInfo.height = extent.height;
            framebufferCreateInfo.layers = layers;
            framebufferCreateInfo.flags = {};

            framebuffers[i] = detail::State.device.createFramebuffer(framebufferCreateInfo);
        }

        const auto outFramebuffer = new detail::Framebuffer();
        outFramebuffer->framebuffers = framebuffers;
        outFramebuffer->extent = extent;
        outFramebuffer->layers = layers;
        outFramebuffer->attachmentCount = static_cast<uint32_t>(attachments.size());

        return outFramebuffer;
    }

    Framebuffer createFramebuffer(const RenderPass& renderPass, const std::vector<std::vector<ava::ImageView>>& attachments, const vk::Extent2D extent, const int layers)
    {
        std::vector<std::vector<vk::ImageView>> imageViews;
        imageViews.reserve(attachments.size());
        for (auto& frame : attachments)
        {
            imageViews.push_back(std::vector<vk::ImageView>());
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

        return createFramebuffer(renderPass, imageViews, extent, layers);
    }

    Framebuffer createSwapchainFramebuffer(const RenderPass& renderPass)
    {
        AVA_CHECK(detail::State.swapchain, "Cannot create a swapchain framebuffer without a valid State swapchain");

        std::vector<std::vector<vk::ImageView>> attachments;
        attachments.reserve(detail::State.swapchainImageViews.size());
        for (auto& imageView : detail::State.swapchainImageViews)
        {
            attachments.push_back({imageView});
        }
        return createFramebuffer(renderPass, attachments, detail::State.swapchainExtent);
    }

    void destroyFramebuffer(Framebuffer& frameBuffer)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(frameBuffer != nullptr, "Cannot destroy an invalid framebuffer");

        for (auto& framebuffer : frameBuffer->framebuffers)
        {
            if (framebuffer)
            {
                detail::State.device.destroyFramebuffer(framebuffer);
            }
        }

        delete frameBuffer;
        frameBuffer = nullptr;
    }
}
