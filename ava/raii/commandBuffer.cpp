#include "commandBuffer.hpp"
#include "ava/commandBuffer.hpp"
#include "ava/detail/commandBuffer.hpp"

#include "buffer.hpp"
#include "compute.hpp"
#include "descriptors.hpp"
#include "framebuffer.hpp"
#include "graphics.hpp"
#include "image.hpp"
#include "vbo.hpp"
#include "ibo.hpp"
#include "rayTracingPipeline.hpp"
#include "renderPass.hpp"
#include "vibo.hpp"
#include "ava/buffer.hpp"
#include "ava/compute.hpp"
#include "ava/descriptors.hpp"
#include "ava/vbo.hpp"
#include "ava/ibo.hpp"
#include "ava/vibo.hpp"
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

    CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
    {
        commandBuffer = std::move(other.commandBuffer);
    }

    CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
    {
        commandBuffer = std::move(other.commandBuffer);
        return *this;
    }

    vk::CommandBuffer CommandBuffer::getCommandBuffer() const
    {
        if (commandBuffer == nullptr)
            return nullptr;
        return commandBuffer->commandBuffer;
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

    void CommandBuffer::beginRenderPass(const Pointer<RenderPass>& renderPass, const Pointer<Framebuffer>& framebuffer, const std::vector<vk::ClearValue>& clearValues) const
    {
        ava::beginRenderPass(commandBuffer, renderPass->renderPass, framebuffer->framebuffer, clearValues);
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
        AVA_CHECK(ibo != nullptr && ibo->ibo != nullptr, "Cannot bind an invalid IBO to command buffer");
        ava::bindIBO(commandBuffer, ibo->ibo);
    }

    void CommandBuffer::bindVIBO(const Pointer<VIBO>& vibo) const
    {
        AVA_CHECK(vibo != nullptr && vibo->vibo != nullptr, "Cannot bind an invalid VIBO to command buffer");
        ava::bindVIBO(commandBuffer, vibo->vibo);
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

    void CommandBuffer::insertBufferMemoryBarrier(const Pointer<Buffer>& buffer, vk::PipelineStageFlags srcStage, vk::PipelineStageFlagBits dstStage, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::DeviceSize size, vk::DeviceSize offset) const
    {
        AVA_CHECK(buffer != nullptr && buffer->buffer, "Cannot insert memory barrier when buffer is invalid");
        ava::insertBufferMemoryBarrier(commandBuffer, buffer->buffer, srcStage, dstStage, srcAccessMask, dstAccessMask, size, offset);
    }

    void CommandBuffer::pushConstants(const vk::ShaderStageFlags shaderStages, const void* data, const uint32_t size, const uint32_t offset) const
    {
        ava::pushConstants(commandBuffer, shaderStages, data, size, offset);
    }

    void CommandBuffer::bindRayTracingPipeline(const Pointer<RayTracingPipeline>& rayTracingPipeline) const
    {
        AVA_CHECK(rayTracingPipeline != nullptr && rayTracingPipeline->pipeline != nullptr, "Cannot bind invalid ray tracing pipeline");

        ava::bindRayTracingPipeline(commandBuffer, rayTracingPipeline->pipeline);
    }

    void CommandBuffer::traceRays(const uint32_t width, const uint32_t height, const uint32_t depth) const
    {
        ava::traceRays(commandBuffer, width, height, depth);
    }

    Pointer<CommandBuffer> CommandBuffer::beginSingleTime(const vk::QueueFlagBits queueType)
    {
        return std::make_shared<CommandBuffer>(ava::beginSingleTimeCommands(queueType));
    }
}
