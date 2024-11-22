#ifndef AVA_RAII_RENDERPASS_HPP
#define AVA_RAII_RENDERPASS_HPP

#include "types.hpp"
#include "ava/renderPass.hpp"

namespace ava::raii
{
    class RenderPass
    {
    public:
        explicit RenderPass(const ava::RenderPass& existingRenderPass);
        ~RenderPass();

        ava::RenderPass renderPass;

        RenderPass(const RenderPass& other) = delete;
        RenderPass& operator=(RenderPass& other) = delete;

        // ReSharper disable once CppNonExplicitConversionOperator
        operator ava::RenderPass() const;

        static Pointer<RenderPass> create(const ava::RenderPassCreateInfo& renderPassCreateInfo);
        static Pointer<RenderPass> create(const vk::RenderPassCreateInfo& renderPassCreateInfo);
    };
}

#endif