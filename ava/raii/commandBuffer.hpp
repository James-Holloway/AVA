#ifndef AVA_RAII_COMMANDBUFFER_HPP
#define AVA_RAII_COMMANDBUFFER_HPP

#include "types.hpp"
#include "../types.hpp"

namespace ava::raii
{
    class CommandBuffer
    {
    public:
        using Ptr = Pointer<CommandBuffer>;

        explicit CommandBuffer(const ava::CommandBuffer& existingCommandBuffer);
        virtual ~CommandBuffer();

        ava::CommandBuffer commandBuffer;

        CommandBuffer(const CommandBuffer& other) = delete;
        CommandBuffer& operator=(CommandBuffer& other) = delete;
        CommandBuffer(CommandBuffer&& other) noexcept;
        CommandBuffer& operator=(CommandBuffer&& other) noexcept;

        [[nodiscard]] vk::CommandBuffer getCommandBuffer() const;

        void start(vk::CommandBufferUsageFlags usageFlags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit) const;
        void end() const;
        void endSingleTime() const;

        void trackObject(const std::shared_ptr<void>& object) const;
        void untrackAllObjects() const;

        void beginRenderPass(const Pointer<RenderPass>& renderPass, const Pointer<Framebuffer>& framebuffer, const std::vector<vk::ClearValue>& clearValues) const;
        void endRenderPass() const;
        void nextSubpass() const;

        void bindVBO(const Pointer<VBO>& vbo) const;
        void bindIBO(const Pointer<IBO>& ibo) const;
        void bindVIBO(const Pointer<VIBO>& vibo) const;

        void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) const;
        // indexCount of 0 means it will draw the number of indices in the currently bound IBO
        void drawIndexed(uint32_t indexCount = 0, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) const;
        void dispatch(uint32_t groupCountX, uint32_t groupCountY = 1, uint32_t groupCountZ = 1) const;

        void bindComputePipeline(const Pointer<ComputePipeline>& pipeline) const;
        void bindGraphicsPipeline(const Pointer<GraphicsPipeline>& pipeline) const;

        void bindDescriptorSet(const Pointer<DescriptorSet>& set) const;

        void insertImageMemoryBarrier(const Pointer<Image>& image, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eAllCommands,
                                      vk::AccessFlags srcAccessMask = {}, vk::AccessFlags dstAccessMask = {}, const std::optional<vk::ImageSubresourceRange>& subresourceRange = {}) const;
        void transitionImageLayout(const Pointer<Image>& image, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eAllCommands,
                                   const std::optional<vk::ImageSubresourceRange>& subresourceRange = {}) const;

        void insertBufferMemoryBarrier(const Pointer<Buffer>& buffer, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0) const;

        void pushConstants(vk::ShaderStageFlags shaderStages, const void* data, uint32_t size = 0, uint32_t offset = 0) const;

        template <typename T>
        void pushConstants(vk::ShaderStageFlags shaderStages, const T& data, const uint32_t offset = 0) const
        {
            pushConstants(shaderStages, &data, sizeof(T), offset);
        }

        void bindRayTracingPipeline(const Pointer<RayTracingPipeline>& rayTracingPipeline) const;
        void traceRays(uint32_t width, uint32_t height, uint32_t depth = 1) const;

        static Pointer<CommandBuffer> beginSingleTime(vk::QueueFlagBits queueType);
    };
}

#endif
