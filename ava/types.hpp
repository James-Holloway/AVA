#ifndef AVA_FORWARD_HPP
#define AVA_FORWARD_HPP

#include "detail/vulkan.hpp"
#include <memory>

namespace ava
{
    namespace detail
    {
        struct CommandBuffer;
        struct RenderPass;
        struct Framebuffer;
        struct Buffer;
        struct Image;
        struct ImageView;
        struct Sampler;
        struct DescriptorPool;
        struct DescriptorSet;
        struct Shader;
        struct GraphicsPipeline;
        struct ComputePipeline;
        struct VAO;
        struct VBO;
        struct IBO;
        struct VIBO;
    }

    using CommandBuffer = std::shared_ptr<detail::CommandBuffer>;
    using RenderPass = detail::RenderPass*;
    using Framebuffer = detail::Framebuffer*;
    using Buffer = detail::Buffer*;
    using Image = detail::Image*;
    using ImageView = detail::ImageView*;
    using Sampler = detail::Sampler*;
    using DescriptorPool = detail::DescriptorPool*;
    using DescriptorSet = std::weak_ptr<detail::DescriptorSet>;
    using Shader = detail::Shader*;
    using GraphicsPipeline = detail::GraphicsPipeline*;
    using ComputePipeline = detail::ComputePipeline*;
    using VAO = detail::VAO*;
    using VBO = detail::VBO*;
    using IBO = detail::IBO*;
    using VIBO = detail::VIBO*;
}

#endif
