#ifndef AVA_RAII_RENDERPASS_HPP
#define AVA_RAII_RENDERPASS_HPP

#include "types.hpp"
#include "ava/renderPass.hpp"

namespace ava::raii
{
    class RenderPass
    {
    public:
        using Ptr = Pointer<RenderPass>;

        explicit RenderPass(const ava::RenderPass& existingRenderPass);
        ~RenderPass();

        ava::RenderPass renderPass;

        RenderPass(const RenderPass& other) = delete;
        RenderPass& operator=(RenderPass& other) = delete;
        RenderPass(RenderPass&& other) noexcept;
        RenderPass& operator=(RenderPass&& other) noexcept;

        // ReSharper disable once CppNonExplicitConversionOperator
        operator ava::RenderPass() const;

        static Pointer<RenderPass> create(const ava::RenderPassCreationInfo& renderPassCreateInfo);
        static Pointer<RenderPass> create(const vk::RenderPassCreateInfo& renderPassCreateInfo);
    };
}

#endif
