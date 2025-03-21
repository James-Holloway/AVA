#ifndef AVA_COMMANDBUFFER_HPP
#define AVA_COMMANDBUFFER_HPP

#include "types.hpp"
#include "detail/vulkan.hpp"
#include <memory>

namespace ava
{
    void startCommandBuffer(const CommandBuffer& commandBuffer, vk::CommandBufferUsageFlags usageFlags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    void endCommandBuffer(const CommandBuffer& commandBuffer);

    [[nodiscard]] CommandBuffer beginSingleTimeCommands(vk::QueueFlagBits queueType);
    void endSingleTimeCommands(const CommandBuffer& commandBuffer);

    void beginRenderPass(const CommandBuffer& commandBuffer, const RenderPass& renderPass, const Framebuffer& framebuffer, const std::vector<vk::ClearValue>& clearValues);
    void endRenderPass(const CommandBuffer& commandBuffer);
    void nextSubpass(const CommandBuffer& commandBuffer);

    vk::CommandBuffer getCommandBuffer(const CommandBuffer& commandBuffer);
    void trackObject(const CommandBuffer& commandBuffer, const std::shared_ptr<void>& object);
    void untrackAllObjects(const CommandBuffer& commandBuffer);

    void draw(const CommandBuffer& commandBuffer, uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    // indexCount of 0 means it will draw the number of indices in the currently bound IBO
    void drawIndexed(const CommandBuffer& commandBuffer, uint32_t indexCount = 0, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);

    void pushConstants(const CommandBuffer& commandBuffer, vk::ShaderStageFlags shaderStages, const void* data, uint32_t size = 0, uint32_t offset = 0);

    template <typename T>
    void pushConstants(const CommandBuffer& commandBuffer, const vk::ShaderStageFlags shaderStages, const T& data, const uint32_t offset = 0)
    {
        pushConstants(commandBuffer, shaderStages, &data, sizeof(T), offset);
    }
}

#endif
