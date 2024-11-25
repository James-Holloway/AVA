#include "renderPass.hpp"

#include "detail/renderPass.hpp"

#include <map>

#include "detail/detail.hpp"
#include "detail/state.hpp"

namespace ava
{
    struct IntermediateRenderPassAttachmentInfo
    {
        RenderPassAttachmentInfo attachment{};
        uint32_t attachmentIndex{};
        vk::AttachmentDescription attachmentDescription;

        // If attachment has a subpass matching the subpass and subpass attachment type
        [[nodiscard]] bool hasAttachmentType(const uint32_t subpass, const SubPassAttachmentTypeFlags type) const
        {
            if (subpass >= attachment.subpassInfos.size()) return false;

            return (attachment.subpassInfos[subpass].attachmentType & type) == type;
        }
    };

    struct IntermediateSubPassInfo
    {
        std::vector<vk::AttachmentReference> colorAttachments{};
        std::vector<vk::AttachmentReference> depthAttachments{};
        std::vector<vk::AttachmentReference> inputAttachments{};
        std::vector<vk::AttachmentReference> resolveAttachments{};
        std::vector<uint32_t> preserveAttachments{};

        vk::SubpassDescription subpassDescription{};
    };

    SubPassAttachmentTypeFlags operator|(const SubPassAttachmentTypeFlagBits lhs, const SubPassAttachmentTypeFlagBits rhs)
    {
        return SubPassAttachmentTypeFlags{lhs} | rhs;
    }

    [[nodiscard]] static vk::AttachmentDescription createAttachmentDescription(const RenderPassAttachmentInfo& info)
    {
        vk::AttachmentDescription attachmentDescription;
        // Format and samples
        attachmentDescription.format = info.format;
        attachmentDescription.samples = info.sampleCount;
        // Layouts
        attachmentDescription.initialLayout = info.initialLayout;
        attachmentDescription.finalLayout = info.finalLayout;
        // Store/load ops
        attachmentDescription.loadOp = info.loadOp;
        attachmentDescription.storeOp = info.storeOp;
        attachmentDescription.stencilLoadOp = info.stencilLoadOp;
        attachmentDescription.stencilStoreOp = info.stencilStoreOp;

        // Other
        attachmentDescription.flags = info.flags;

        return attachmentDescription;
    }

    [[nodiscard]] static std::vector<IntermediateRenderPassAttachmentInfo> createIntermediateDescriptions(const std::vector<RenderPassAttachmentInfo>& attachments)
    {
        std::vector<IntermediateRenderPassAttachmentInfo> intermediateDescriptions;
        intermediateDescriptions.reserve(attachments.size());

        for (auto& attachment : attachments)
        {
            IntermediateRenderPassAttachmentInfo intermediateInfo;;
            intermediateInfo.attachment = attachment;
            intermediateInfo.attachmentDescription = createAttachmentDescription(attachment);
            intermediateInfo.attachmentIndex = intermediateDescriptions.size();
            intermediateDescriptions.push_back(intermediateInfo);
        }

        return intermediateDescriptions;
    }

    [[nodiscard]] static std::vector<vk::AttachmentReference> createSubpassAttachmentReferences(const std::vector<IntermediateRenderPassAttachmentInfo>& attachments, const uint32_t subpass, const SubPassAttachmentTypeFlagBits subPassAttachmentType)
    {
        if (attachments.empty())
        {
            return {};
        }

        std::map<uint32_t, const IntermediateRenderPassAttachmentInfo*> attachmentOrderMap;
        uint32_t attachmentIndex = 0;
        for (auto& attachment : attachments)
        {
            if (attachment.hasAttachmentType(subpass, subPassAttachmentType))
            {
                auto order = attachment.attachment.subpassInfos[subpass].attachmentOrder;
                if (order == SubpassAttachmentInfo::AUTO_ATTACHMENT_ORDER)
                {
                    order = attachmentIndex;
                }

                if (order != SubpassAttachmentInfo::IGNORE_ATTACHMENT)
                {
                    while (attachmentOrderMap.contains(order))
                    {
                        order++;
                    }

                    attachmentOrderMap[order] = &attachment;
                }
            }
            attachmentIndex++;
        }

        std::vector<vk::AttachmentReference> subpassAttachments;
        subpassAttachments.reserve(attachmentOrderMap.size());
        for (auto& [location, attachment] : attachmentOrderMap)
        {
            subpassAttachments.emplace_back(attachment->attachmentIndex, attachment->attachment.subpassInfos[subpass].layout);
        }

        return subpassAttachments;
    }

    // No ignores
    [[nodiscard]] static std::vector<vk::AttachmentReference> filterAttachmentReferencesToNonIgnore(const std::vector<vk::AttachmentReference>& attachments)
    {
        std::vector<vk::AttachmentReference> filteredAttachments;
        for (auto& attachment : attachments)
        {
            if (attachment.attachment != vk::AttachmentUnused)
            {
                filteredAttachments.push_back(attachment);
            }
        }
        return filteredAttachments;
    }

    static std::vector<vk::AttachmentReference> createResolveSubpassAttachmentReferences(const std::vector<IntermediateRenderPassAttachmentInfo>& attachments, const uint32_t subpass, const SubPassAttachmentTypeFlags subPassAttachment, const uint32_t vectorSize)
    {
        std::vector resolveSubpassAttachments(vectorSize, vk::AttachmentReference{vk::AttachmentUnused});
        uint32_t attachmentIndex = 0;
        for (auto& attachment : attachments)
        {
            if (attachment.hasAttachmentType(subpass, subPassAttachment))
            {
                auto location = attachment.attachment.subpassInfos[subpass].attachmentOrder;
                if (location == SubpassAttachmentInfo::AUTO_ATTACHMENT_ORDER)
                {
                    location = attachmentIndex;
                }

                auto resolveLocation = attachment.attachment.subpassInfos[subpass].resolveAttachmentIndex;
                if (resolveLocation == SubpassAttachmentInfo::AUTO_ATTACHMENT_ORDER)
                {
                    resolveLocation = attachmentIndex;
                }

                if (resolveLocation != SubpassAttachmentInfo::IGNORE_ATTACHMENT)
                {
                    AVA_CHECK(resolveLocation < attachments.size(), "Resolve attachment index " + std::to_string(resolveLocation) + " is out of range of attachments size (" + std::to_string(attachments.size()) + ") on attachment " + std::to_string(attachmentIndex) + " and subpass " + std::to_string(subpass));
                    const auto layout = attachments[resolveLocation].attachment.subpassInfos[subpass].layout;

                    resolveSubpassAttachments[location] = vk::AttachmentReference{resolveLocation, layout};
                }
            }

            attachmentIndex++;
        }

        return resolveSubpassAttachments;
    }

    static void populateSubpassInfo(IntermediateSubPassInfo& subPassInfo, const std::vector<IntermediateRenderPassAttachmentInfo>& intermediateAttachments, const uint32_t subpass)
    {
        subPassInfo.colorAttachments = createSubpassAttachmentReferences(intermediateAttachments, subpass, SubPassAttachmentTypeFlagBits::eColor);
        subPassInfo.depthAttachments = filterAttachmentReferencesToNonIgnore(createSubpassAttachmentReferences(intermediateAttachments, subpass, SubPassAttachmentTypeFlagBits::eDepthStencil));
        subPassInfo.inputAttachments = createSubpassAttachmentReferences(intermediateAttachments, subpass, SubPassAttachmentTypeFlagBits::eInputAttachment);
        auto preserveAttachmentReferences = createSubpassAttachmentReferences(intermediateAttachments, subpass, SubPassAttachmentTypeFlagBits::ePreserve);
        if (!preserveAttachmentReferences.empty())
        {
            subPassInfo.preserveAttachments.reserve(preserveAttachmentReferences.size());
            for (auto& preserveAttachment : preserveAttachmentReferences)
            {
                subPassInfo.preserveAttachments.push_back(preserveAttachment.attachment);
            }
        }
        // Resolve shares colorAttachmentCount
        subPassInfo.resolveAttachments = createResolveSubpassAttachmentReferences(intermediateAttachments, subpass, SubPassAttachmentTypeFlagBits::eColor | SubPassAttachmentTypeFlagBits::eResolve, subPassInfo.colorAttachments.size());

        AVA_CHECK(subPassInfo.depthAttachments.size() <= 1, "Cannot have more than one depth attachment for subpass " + std::to_string(subpass));
        AVA_CHECK(!subPassInfo.depthAttachments.empty() || !subPassInfo.colorAttachments.empty(), "Cannot have a subpass with 0 color attachments and no depth attachment");

        subPassInfo.subpassDescription.flags = {};
        subPassInfo.subpassDescription.setResolveAttachments(subPassInfo.resolveAttachments); // Set resolves before color so that colorAttachmentCount doesn't get reset to 0 if no resolves are present
        subPassInfo.subpassDescription.setColorAttachments(subPassInfo.colorAttachments);
        if (!subPassInfo.depthAttachments.empty())
        {
            subPassInfo.subpassDescription.setPDepthStencilAttachment(&subPassInfo.depthAttachments[0]);
        }
        subPassInfo.subpassDescription.setInputAttachments(subPassInfo.inputAttachments);
        subPassInfo.subpassDescription.setPreserveAttachments(subPassInfo.preserveAttachments);
    }

    static uint32_t getSubpassColorAttachmentCount(const std::vector<vk::AttachmentReference>& colorAttachmentReferences) noexcept
    {
        uint32_t colorAttachmentCount = 0;
        for (const auto& colorAttachment : colorAttachmentReferences)
        {
            if (colorAttachment.attachment != vk::AttachmentUnused)
            {
                colorAttachmentCount++;
            }
        }

        return colorAttachmentCount;
    }

    static uint32_t getSubpassColorAttachmentCount(const vk::AttachmentReference* colorAttachmentReferences, const uint32_t colorAttachmentCount) noexcept
    {
        return getSubpassColorAttachmentCount(std::vector(colorAttachmentReferences, &colorAttachmentReferences[colorAttachmentCount]));
    }

    RenderPass createRenderPass(const RenderPassCreationInfo& createInfo)
    {
        AVA_CHECK(!createInfo.attachments.empty(), "Cannot create RenderPass without any attachments");
        AVA_CHECK(createInfo.subPasses != 0, "Cannot create a RenderPass with 0 subpasses");
        AVA_CHECK(detail::State.device, "Cannot create a RenderPass without a valid State device");

        // Create intermediate information
        auto intermediateAttachments = createIntermediateDescriptions(createInfo.attachments);

        // Create render pass attachment descriptions
        std::vector<vk::AttachmentDescription> attachmentDescriptions;
        attachmentDescriptions.reserve(intermediateAttachments.size());
        for (auto& intermediate : intermediateAttachments)
        {
            attachmentDescriptions.push_back(intermediate.attachmentDescription);
        }

        // Creation of each subpass
        std::vector<vk::SubpassDescription> subpassDescriptions;
        subpassDescriptions.reserve(createInfo.subPasses);
        std::vector<IntermediateSubPassInfo> intermediateSubpasses;
        intermediateSubpasses.resize(createInfo.subPasses);
        std::vector colorAttachmentCounts(createInfo.subPasses, 0u);
        for (uint32_t subpass = 0; subpass < createInfo.subPasses; subpass++)
        {
            populateSubpassInfo(intermediateSubpasses[subpass], intermediateAttachments, subpass);
            subpassDescriptions.push_back(intermediateSubpasses[subpass].subpassDescription);
            colorAttachmentCounts[subpass] = getSubpassColorAttachmentCount(intermediateSubpasses[subpass].colorAttachments);
        }

        std::vector<vk::SubpassDependency> subpassDependencies;
        subpassDependencies.resize(createInfo.subPasses + 1);

        auto& firstDependency = subpassDependencies[0];
        firstDependency.srcSubpass = vk::SubpassExternal;
        firstDependency.dstSubpass = 0;
        firstDependency.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        firstDependency.dstStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
        firstDependency.srcAccessMask = vk::AccessFlagBits::eNone;
        firstDependency.dstAccessMask = vk::AccessFlagBits::eNone;
        firstDependency.dependencyFlags = vk::DependencyFlagBits::eByRegion;

        for (uint32_t subpass = 0; subpass < createInfo.subPasses - 1; subpass++)
        {
            auto& subpassDependency = subpassDependencies[subpass + 1];
            subpassDependency.srcSubpass = subpass;
            subpassDependency.dstSubpass = subpass + 1;
            subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
            subpassDependency.dstStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
            subpassDependency.srcAccessMask = vk::AccessFlagBits::eNone;
            subpassDependency.dstAccessMask = vk::AccessFlagBits::eNone;
            subpassDependency.dependencyFlags = vk::DependencyFlagBits::eByRegion;
        }

        auto& lastDependency = subpassDependencies[subpassDependencies.size() - 1];
        lastDependency.srcSubpass = createInfo.subPasses - 1;
        lastDependency.dstSubpass = vk::SubpassExternal;
        lastDependency.srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
        lastDependency.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        lastDependency.srcAccessMask = vk::AccessFlagBits::eNone;
        lastDependency.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
        lastDependency.dependencyFlags = vk::DependencyFlagBits::eByRegion;

        vk::RenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.flags = {};
        renderPassCreateInfo.setAttachments(attachmentDescriptions);
        renderPassCreateInfo.setSubpasses(subpassDescriptions);
        renderPassCreateInfo.setDependencies(subpassDependencies);

        const auto renderPass = detail::State.device.createRenderPass(renderPassCreateInfo);
        AVA_CHECK(renderPass, "Failed to create RenderPass");

        const auto outRenderPass = new detail::RenderPass();
        outRenderPass->renderPass = renderPass;
        outRenderPass->attachmentCount = renderPassCreateInfo.attachmentCount;
        outRenderPass->subpasses = renderPassCreateInfo.subpassCount;
        outRenderPass->subpassColorAttachmentCounts = colorAttachmentCounts;
        return outRenderPass;
    }

    RenderPass createRenderPass(const vk::RenderPassCreateInfo& renderPassCreateInfo)
    {
        AVA_CHECK(detail::State.device, "Cannot create a RenderPass without a valid State device");

        const auto renderPass = detail::State.device.createRenderPass(renderPassCreateInfo);
        AVA_CHECK(renderPass, "Failed to create RenderPass");

        const auto outRenderPass = new detail::RenderPass();
        outRenderPass->renderPass = renderPass;
        outRenderPass->attachmentCount = renderPassCreateInfo.attachmentCount;
        outRenderPass->subpasses = renderPassCreateInfo.subpassCount;

        // Subpass color attachment counts, reverse the render pass create info to get the attachments
        outRenderPass->subpassColorAttachmentCounts.reserve(outRenderPass->subpasses);
        for (uint32_t subpass = 0; subpass < renderPassCreateInfo.subpassCount; subpass++)
        {
            const auto& subpassInfo = renderPassCreateInfo.pSubpasses[subpass];
            outRenderPass->subpassColorAttachmentCounts.push_back(getSubpassColorAttachmentCount(subpassInfo.pColorAttachments, subpassInfo.colorAttachmentCount));
        }

        return outRenderPass;
    }

    void destroyRenderPass(RenderPass& renderPass)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(renderPass != nullptr, "Cannot destroy an invalid render pass");

        if (renderPass->renderPass)
        {
            detail::State.device.destroyRenderPass(renderPass->renderPass);
        }
        delete renderPass;
        renderPass = nullptr;
    }

    RenderPassAttachmentInfo createSimpleColorAttachmentInfo(const vk::Format colorFormat, const bool isFirst, const bool isFinal, const uint32_t subpasses)
    {
        RenderPassAttachmentInfo renderPassAttachmentInfo;
        renderPassAttachmentInfo.subpassInfos = std::vector<SubpassAttachmentInfo>(subpasses, {SubpassAttachmentInfo::AUTO_ATTACHMENT_ORDER, vk::ImageLayout::eColorAttachmentOptimal, SubPassAttachmentTypeFlagBits::eColor});
        renderPassAttachmentInfo.format = colorFormat;
        renderPassAttachmentInfo.sampleCount = vk::SampleCountFlagBits::e1;
        renderPassAttachmentInfo.initialLayout = vk::ImageLayout::eUndefined;
        renderPassAttachmentInfo.finalLayout = isFinal ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eColorAttachmentOptimal;
        renderPassAttachmentInfo.loadOp = isFirst ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
        renderPassAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
        renderPassAttachmentInfo.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        renderPassAttachmentInfo.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

        return renderPassAttachmentInfo;
    }

    RenderPassAttachmentInfo createSimpleDepthAttachmentInfo(const vk::Format depthFormat, const bool isFirst, const uint32_t subpasses)
    {
        const bool hasStencil = detail::vulkanFormatHasStencil(depthFormat);
        RenderPassAttachmentInfo renderPassAttachmentInfo;
        renderPassAttachmentInfo.subpassInfos = std::vector<SubpassAttachmentInfo>(subpasses, {SubpassAttachmentInfo::AUTO_ATTACHMENT_ORDER, vk::ImageLayout::eDepthStencilAttachmentOptimal, SubPassAttachmentTypeFlagBits::eDepthStencil});
        renderPassAttachmentInfo.format = depthFormat;
        renderPassAttachmentInfo.sampleCount = vk::SampleCountFlagBits::e1;
        renderPassAttachmentInfo.initialLayout = vk::ImageLayout::eUndefined;
        renderPassAttachmentInfo.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        renderPassAttachmentInfo.loadOp = isFirst ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
        renderPassAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
        renderPassAttachmentInfo.stencilLoadOp = hasStencil ? (isFirst ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad) : vk::AttachmentLoadOp::eDontCare;
        renderPassAttachmentInfo.stencilStoreOp = hasStencil ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare;

        return renderPassAttachmentInfo;
    }

    RenderPassAttachmentInfo createSimpleResolveAttachmentInfo(const vk::Format colorFormat, bool isFinal, const uint32_t subpasses)
    {
        RenderPassAttachmentInfo renderPassAttachmentInfo;
        renderPassAttachmentInfo.subpassInfos = std::vector<SubpassAttachmentInfo>(subpasses, {SubpassAttachmentInfo::AUTO_ATTACHMENT_ORDER, vk::ImageLayout::eColorAttachmentOptimal, {}});
        renderPassAttachmentInfo.format = colorFormat;
        renderPassAttachmentInfo.sampleCount = vk::SampleCountFlagBits::e1;
        renderPassAttachmentInfo.initialLayout = vk::ImageLayout::eUndefined;
        renderPassAttachmentInfo.finalLayout = isFinal ? vk::ImageLayout::ePresentSrcKHR : vk::ImageLayout::eColorAttachmentOptimal;
        renderPassAttachmentInfo.loadOp = vk::AttachmentLoadOp::eDontCare;
        renderPassAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
        renderPassAttachmentInfo.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        renderPassAttachmentInfo.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

        return renderPassAttachmentInfo;
    }
}
