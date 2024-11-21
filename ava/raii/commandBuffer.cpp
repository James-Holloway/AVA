#include "commandBuffer.hpp"
#include "ava/commandBuffer.hpp"
#include "ava/detail/commandBuffer.hpp"

#include "ava/detail/detail.hpp"

namespace ava::raii
{
    CommandBuffer::CommandBuffer(const ava::CommandBuffer& existingCommandBuffer)
    {
        AVA_CHECK(existingCommandBuffer != nullptr && existingCommandBuffer->commandBuffer, "Cannot create a RAII command buffer from an invalid command buffer");
        commandBuffer = existingCommandBuffer;
    }

    CommandBuffer::~CommandBuffer()
    {
        commandBuffer.reset();
    }

    void CommandBuffer::start() const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot end an invalid command buffer");
        AVA_CHECK(!commandBuffer->started, "Cannot end a command buffer which has already been started");

        startCommandBuffer(commandBuffer);
    }

    void CommandBuffer::end() const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot end an invalid command buffer");
        AVA_CHECK(commandBuffer->started, "Cannot end a command buffer which has not started");

        endCommandBuffer(commandBuffer);
    }

    void CommandBuffer::endSingleTime() const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot end single time command buffer when command buffer is invalid");

        endSingleTimeCommands(commandBuffer);
    }

    void CommandBuffer::trackObject(const std::shared_ptr<void>& object) const
    {
        ava::trackObject(commandBuffer, object);
    }

    void CommandBuffer::untrackAllObjects() const
    {
        ava::untrackAllObjects(commandBuffer);
    }

    void CommandBuffer::beginRenderPass(const RenderPass& renderPass, const Framebuffer& framebuffer, const std::vector<vk::ClearValue>& clearValues) const
    {
        ava::beginRenderPass(commandBuffer, renderPass, framebuffer, clearValues);
    }

    void CommandBuffer::endRenderPass() const
    {
        ava::endRenderPass(commandBuffer);
    }

    void CommandBuffer::nextSubpass() const
    {
        ava::nextSubpass(commandBuffer);
    }

    void CommandBuffer::draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) const
    {
        ava::draw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void CommandBuffer::drawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance) const
    {
        ava::drawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    Pointer<CommandBuffer> CommandBuffer::beginSingleTime(const vk::QueueFlagBits queueType)
    {
        return std::make_shared<CommandBuffer>(ava::beginSingleTimeCommands(queueType));
    }
}
