#ifndef AVA_RAII_COMMANDBUFFER_HPP
#define AVA_RAII_COMMANDBUFFER_HPP

#include "types.hpp"
#include "../types.hpp"

namespace ava::raii
{
    class CommandBuffer
    {
    public:
        explicit CommandBuffer(const ava::CommandBuffer& existingCommandBuffer);
        virtual ~CommandBuffer();

        ava::CommandBuffer commandBuffer;

        void start() const;
        void end() const;
        void endSingleTime() const;

        void trackObject(const std::shared_ptr<void>& object) const;
        void untrackAllObjects() const;

        void beginRenderPass(const RenderPass& renderPass, const Framebuffer& framebuffer, const std::vector<vk::ClearValue>& clearValues) const;
        void endRenderPass() const;
        void nextSubpass() const;

        void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
        // indexCount of 0 means it will draw the number of indices in the currently bound IBO
        void drawIndexed(uint32_t indexCount = 0, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) const;

        static Pointer<CommandBuffer> beginSingleTime(vk::QueueFlagBits queueType);
    };
}

#endif
