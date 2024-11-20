#ifndef AVA_COMMANDBUFFER_HPP
#define AVA_COMMANDBUFFER_HPP

#include "frameBuffer.hpp"
#include "renderPass.hpp"
#include "detail/vulkan.hpp"
#include <memory>

namespace ava
{
    namespace detail
    {
        struct CommandBuffer;
    }

    using CommandBuffer = std::shared_ptr<detail::CommandBuffer>;

    void startCommandBuffer(const CommandBuffer& commandBuffer, vk::CommandBufferUsageFlags usageFlags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    void endCommandBuffer(const CommandBuffer& commandBuffer);

    CommandBuffer beginSingleTimeCommands(vk::QueueFlagBits queueType);
    void endSingleTimeCommands(const CommandBuffer& commandBuffer);

    void beginRenderPass(const CommandBuffer& commandBuffer, const RenderPass& renderPass, const Framebuffer& framebuffer, const std::vector<vk::ClearValue>& clearValues);
    void endRenderPass(const CommandBuffer& commandBuffer);
    void nextSubpass(const CommandBuffer& commandBuffer);

    vk::CommandBuffer getCommandBuffer(const CommandBuffer& commandBuffer);
    void draw(const CommandBuffer& commandBuffer, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
}

#endif
