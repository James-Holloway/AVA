#include "renderPass.hpp"
#include "ava/renderPass.hpp"
#include "ava/detail/renderPass.hpp"

#include "ava/detail/detail.hpp"

namespace ava::raii
{
    RenderPass::RenderPass(const ava::RenderPass& existingRenderPass)
    {
        AVA_CHECK(existingRenderPass != nullptr && existingRenderPass->renderPass, "Cannot create RAII render pass from an invalid render pass");
        renderPass = existingRenderPass;
    }

    RenderPass::~RenderPass()
    {
        if (renderPass != nullptr)
        {
            ava::destroyRenderPass(renderPass);
        }
    }

    RenderPass::RenderPass(RenderPass&& other) noexcept
    {
        renderPass = other.renderPass;
        other.renderPass = nullptr;
    }

    RenderPass& RenderPass::operator=(RenderPass&& other) noexcept
    {
        if (this != &other)
        {
            renderPass = other.renderPass;
            other.renderPass = nullptr;
        }
        return *this;
    }

    RenderPass::operator ava::RenderPass() const
    {
        return renderPass;
    }

    Pointer<RenderPass> RenderPass::create(const ava::RenderPassCreateInfo& renderPassCreateInfo)
    {
        return std::make_shared<RenderPass>(ava::createRenderPass(renderPassCreateInfo));
    }

    Pointer<RenderPass> RenderPass::create(const vk::RenderPassCreateInfo& renderPassCreateInfo)
    {
        return std::make_shared<RenderPass>(ava::createRenderPass(renderPassCreateInfo));
    }
}
