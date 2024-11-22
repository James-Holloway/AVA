#include "commandBuffer.hpp"
#include "ava/commandBuffer.hpp"
#include "ava/detail/commandBuffer.hpp"

#include "compute.hpp"
#include "descriptors.hpp"
#include "graphics.hpp"
#include "image.hpp"
#include "vbo.hpp"
#include "ibo.hpp"
#include "ava/compute.hpp"
#include "ava/descriptors.hpp"
#include "ava/vbo.hpp"
#include "ava/ibo.hpp"
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

    void CommandBuffer::start(const vk::CommandBufferUsageFlags usageFlags) const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot end an invalid command buffer");

        startCommandBuffer(commandBuffer, usageFlags);
    }

    void CommandBuffer::end() const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot end an invalid command buffer");

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

    void CommandBuffer::bindVBO(const Pointer<VBO>& vbo) const
    {
        AVA_CHECK(vbo != nullptr && vbo->vbo !=nullptr, "Cannot bind an invalid VBO to command buffer");
        ava::bindVBO(commandBuffer, vbo->vbo);
    }

    void CommandBuffer::bindIBO(const Pointer<IBO>& ibo) const
    {
        AVA_CHECK(ibo != nullptr && ibo->ibo !=nullptr, "Cannot bind an invalid IBO to command buffer");
        ava::bindIBO(commandBuffer, ibo->ibo);
    }

    void CommandBuffer::draw(const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance) const
    {
        ava::draw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void CommandBuffer::drawIndexed(const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance) const
    {
        ava::drawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void CommandBuffer::dispatch(const uint32_t groupCountX, const uint32_t groupCountY, const uint32_t groupCountZ) const
    {
        ava::dispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
    }

    void CommandBuffer::bindComputePipeline(const Pointer<ComputePipeline>& pipeline) const
    {
        AVA_CHECK(pipeline != nullptr && pipeline->pipeline, "Cannot bind an invalid compute pipeline");
        ava::bindComputePipeline(commandBuffer, pipeline->pipeline);
    }

    void CommandBuffer::bindGraphicsPipeline(const Pointer<GraphicsPipeline>& pipeline) const
    {
        AVA_CHECK(pipeline != nullptr && pipeline->pipeline, "Cannot bind an invalid graphics pipeline");
        ava::bindGraphicsPipeline(commandBuffer, pipeline->pipeline);
    }

    void CommandBuffer::bindDescriptorSet(const Pointer<DescriptorSet>& set) const
    {
        AVA_CHECK(set != nullptr, "Cannot bind an invalid descriptor set")
        ava::bindDescriptorSet(commandBuffer, set->descriptorSet);
    }

    void CommandBuffer::insertImageMemoryBarrier(const Pointer<Image>& image, const vk::ImageLayout newLayout, const vk::AccessFlags srcAccessMask, const vk::AccessFlags dstAccessMask, const vk::ImageAspectFlags aspectFlags, const vk::PipelineStageFlags srcStage, const vk::PipelineStageFlags dstStage,
                                                 const std::optional<vk::ImageSubresourceRange>& subresourceRange) const
    {
        AVA_CHECK(image != nullptr && image->image, "Cannot insert memory barrier when image is invalid");
        ava::insertImageMemoryBarrier(commandBuffer, image->image, newLayout, srcAccessMask, dstAccessMask, aspectFlags, srcStage, dstStage, subresourceRange);
    }

    void CommandBuffer::transitionImageLayout(const Pointer<Image>& image, const vk::ImageLayout newLayout, const vk::ImageAspectFlags aspectFlags, const vk::PipelineStageFlags srcStage, const vk::PipelineStageFlags dstStage, const std::optional<vk::ImageSubresourceRange>& subresourceRange) const
    {
        AVA_CHECK(image != nullptr && image->image, "Cannot transition image layout when image is invalid");
        ava::transitionImageLayout(commandBuffer, image->image, newLayout, aspectFlags, srcStage, dstStage, subresourceRange);
    }

    Pointer<CommandBuffer> CommandBuffer::beginSingleTime(const vk::QueueFlagBits queueType)
    {
        return std::make_shared<CommandBuffer>(ava::beginSingleTimeCommands(queueType));
    }
}
