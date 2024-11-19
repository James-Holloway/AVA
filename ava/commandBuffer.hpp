#ifndef AVA_COMMANDBUFFER_HPP
#define AVA_COMMANDBUFFER_HPP

#include "frameBuffer.hpp"
#include "renderPass.hpp"
#include "detail/vulkan.hpp"

namespace ava
{
    void startCommandBuffer(const vk::CommandBuffer& commandBuffer, vk::CommandBufferUsageFlags usageFlags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    void endCommandBuffer(const vk::CommandBuffer& commandBuffer);

    vk::CommandBuffer beginSingleTimeCommands(vk::QueueFlagBits queueType);
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer, vk::QueueFlagBits queueType);

    void beginRenderPass(const vk::CommandBuffer& commandBuffer, const RenderPass& renderPass, const Framebuffer& framebuffer, const std::vector<vk::ClearValue>& clearValues);
    void endRenderPass(const vk::CommandBuffer& commandBuffer);
    void nextSubpass(const vk::CommandBuffer& commandBuffer);
}

#endif
