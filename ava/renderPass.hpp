#ifndef AVA_RENDERPASS_HPP
#define AVA_RENDERPASS_HPP

#include "image.hpp"
#include "detail/vulkan.hpp"

namespace ava
{
    enum class SubPassAttachmentTypeFlagBits : uint32_t
    {
        eNone = 0,
        eColor = 1 << 0,
        eDepthStencil = 1 << 1,
        eInputAttachment = 1 << 2,
        eResolve = 1 << 3,
        ePreserve = 1 << 4,
    };

    using SubPassAttachmentTypeFlags = vk::Flags<SubPassAttachmentTypeFlagBits>;
    inline SubPassAttachmentTypeFlags operator|(SubPassAttachmentTypeFlagBits lhs, SubPassAttachmentTypeFlagBits rhs);

    struct SubpassAttachmentInfo
    {
        constexpr static uint32_t IGNORE_ATTACHMENT = ~0u;
        constexpr static uint32_t AUTO_ATTACHMENT_ORDER = IGNORE_ATTACHMENT - 1;

        // Use IGNORE_ATTACHMENT as a subpassAttachment to not use it in that subpass
        // Use AUTO_ATTACHMENT_ORDER to use the declared order
        uint32_t attachmentOrder = AUTO_ATTACHMENT_ORDER;
        vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal;
        SubPassAttachmentTypeFlags attachmentType = SubPassAttachmentTypeFlagBits::eColor;
        uint32_t resolveAttachmentIndex = IGNORE_ATTACHMENT;
    };

    struct RenderPassAttachmentInfo
    {
        // Subpass infos. Each element is a different subpass
        std::vector<SubpassAttachmentInfo> subpassInfos{{}};

        // Format of the attachment
        vk::Format format = vk::Format::eUndefined;
        vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;

        // Initial and final layouts of the attachment
        vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined; // Undefined means don't care about previous
        vk::ImageLayout finalLayout = vk::ImageLayout::eColorAttachmentOptimal; // Set to ePresentSrcKHR if final

        // Store/load operations
        vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear;
        vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;
        vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

        // Other
        vk::AttachmentDescriptionFlags flags = {};
    };

    struct RenderPassCreationInfo
    {
        // Attachments
        std::vector<RenderPassAttachmentInfo> attachments{};
        uint32_t subPasses = 1;
        std::vector<vk::SubpassDependency> subpassDependencies{}; // Optional, will use defaults if empty
    };

    [[nodiscard]] RenderPass createRenderPass(const RenderPassCreationInfo& createInfo);
    [[nodiscard]] RenderPass createRenderPass(const vk::RenderPassCreateInfo& renderPassCreateInfo);
    void destroyRenderPass(RenderPass& renderPass);

    // Simple functions to create a one render pass attachment with one subpass
    RenderPassAttachmentInfo createSimpleColorAttachmentInfo(vk::Format colorFormat, bool isFirst, bool isFinal, uint32_t subpasses = 1);
    RenderPassAttachmentInfo createSimpleDepthAttachmentInfo(vk::Format depthFormat, bool isFirst, uint32_t subpasses = 1);
    RenderPassAttachmentInfo createSimpleResolveAttachmentInfo(vk::Format colorFormat, bool isFinal, uint32_t subpasses = 1);
}

#endif
