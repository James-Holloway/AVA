#include "commandBuffer.hpp"

#include "detail/detail.hpp"
#include "detail/state.hpp"
#include "detail/commandBuffer.hpp"

#include "detail/renderPass.hpp"

namespace ava
{
    using namespace detail;

    void startCommandBuffer(const ava::CommandBuffer& commandBuffer, const vk::CommandBufferUsageFlags usageFlags)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Command buffer is invalid");
        commandBuffer->commandBuffer.reset();
        commandBuffer->commandBuffer.begin(vk::CommandBufferBeginInfo{usageFlags});
        commandBuffer->pipelineCurrentlyBound = false;
        commandBuffer->lastBoundIndexBufferIndexCount = 0;
    }

    void endCommandBuffer(const ava::CommandBuffer& commandBuffer)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Command buffer is invalid");
        commandBuffer->commandBuffer.end();
    }

    ava::CommandBuffer beginSingleTimeCommands(const vk::QueueFlagBits queueType)
    {
        AVA_CHECK(State.device != nullptr, "Cannot begin single time commands while State device is invalid");

        switch (queueType)
        {
        case vk::QueueFlagBits::eGraphics:
        case vk::QueueFlagBits::eTransfer:
            {
                auto commandBuffer = createGraphicsCommandBuffers(1, false).at(0);
                startCommandBuffer(commandBuffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
                return commandBuffer;
            }
        case vk::QueueFlagBits::eCompute:
            {
                auto commandBuffer = createComputeCommandBuffers(1, false).at(0);
                startCommandBuffer(commandBuffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
                return commandBuffer;
            }
        default:
            throw std::runtime_error("Unhandled queue type");
        }
    }

    void endSingleTimeCommands(const ava::CommandBuffer& commandBuffer)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Command buffer is invalid");
        AVA_CHECK(State.device != nullptr, "Cannot end single time commands while State device is invalid");

        switch (commandBuffer->queue)
        {
        case vk::QueueFlagBits::eGraphics:
        case vk::QueueFlagBits::eTransfer:
        case vk::QueueFlagBits::eCompute:
            break;
        default:
            throw std::runtime_error("Unhandled queue type");
        }

        endCommandBuffer(commandBuffer);
        vk::SubmitInfo submitInfo{};
        submitInfo.setCommandBuffers(commandBuffer->commandBuffer);

        constexpr vk::FenceCreateInfo fenceCreateInfo{vk::FenceCreateFlagBits::eSignaled};

        const auto fence = State.device.createFence(fenceCreateInfo);
        State.device.resetFences(fence);

        vk::Queue queue;
        vk::CommandPool pool;
        switch (commandBuffer->queue)
        {
        case vk::QueueFlagBits::eGraphics:
        case vk::QueueFlagBits::eTransfer:
            {
                queue = State.graphicsQueue;
                pool = State.graphicsCommandPool;
                break;
            }
        case vk::QueueFlagBits::eCompute:
            {
                queue = State.computeQueue;
                pool = State.computeCommandPool;
                break;
            }
        // ReSharper disable once CppDFAUnreachableCode
        default:
            break;
        }

        queue.submit(submitInfo, fence);

        const auto result = State.device.waitForFences(1, &fence, true, std::numeric_limits<uint64_t>::max());
        State.device.destroyFence(fence);
        State.device.freeCommandBuffers(pool, commandBuffer->commandBuffer);
        vk::detail::resultCheck(result, "endSingleTimeCommands failed waiting on submission fence");
    }

    void beginRenderPass(const ava::CommandBuffer& commandBuffer, const ava::RenderPass& renderPass, const ava::Framebuffer& framebuffer, const std::vector<vk::ClearValue>& clearValues)
    {
        AVA_CHECK(State.device != nullptr, "Cannot begin render pass while State device is invalid");
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot begin render pass while command buffer is invalid");
        AVA_CHECK(renderPass != nullptr && renderPass->renderPass, "Cannot begin render pass while RenderPass is invalid");
        AVA_CHECK(framebuffer != nullptr && framebuffer->framebuffers.size() >= State.swapchainImageCount, "Cannot begin render pass while Framebuffer is invalid");

        vk::RenderPassBeginInfo beginInfo{};
        // ReSharper disable once CppDFANullDereference
        beginInfo.framebuffer = framebuffer->framebuffers[State.imageIndex];
        // ReSharper disable once CppDFANullDereference
        beginInfo.renderPass = renderPass->renderPass;
        beginInfo.renderArea = vk::Rect2D{{0, 0}, framebuffer->extent};
        beginInfo.setClearValues(clearValues);

        commandBuffer->commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);

        commandBuffer->currentRenderPassExtent = framebuffer->extent;
    }

    void endRenderPass(const ava::CommandBuffer& commandBuffer)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot end render pass while CommandBuffer is invalid");

        commandBuffer->commandBuffer.endRenderPass();

        commandBuffer->currentRenderPassExtent = vk::Extent2D{0, 0};
        commandBuffer->pipelineCurrentlyBound = false;
        commandBuffer->lastBoundIndexBufferIndexCount = 0;
    }

    void nextSubpass(const ava::CommandBuffer& commandBuffer)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot go to the next subpass while CommandBuffer is invalid");

        commandBuffer->commandBuffer.nextSubpass(vk::SubpassContents::eInline);
    }

    vk::CommandBuffer getCommandBuffer(const ava::CommandBuffer& commandBuffer)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot get Vulkan command buffer from an invalid command buffer");
        return commandBuffer->commandBuffer;
    }

    void draw(const ava::CommandBuffer& commandBuffer, const uint32_t vertexCount, const uint32_t instanceCount, const uint32_t firstVertex, const uint32_t firstInstance)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot draw with an invalid command buffer");
        commandBuffer->commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void drawIndexed(const ava::CommandBuffer& commandBuffer, uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex, const int32_t vertexOffset, const uint32_t firstInstance)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot drawIndexed with an invalid command buffer");

        if (indexCount == 0)
        {
            indexCount = commandBuffer->lastBoundIndexBufferIndexCount;
        }

        commandBuffer->commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }
}
