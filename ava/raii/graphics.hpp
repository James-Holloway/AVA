#ifndef AVA_RAII_GRAPHICS_HPP
#define AVA_RAII_GRAPHICS_HPP

#include "types.hpp"
#include "ava/graphics.hpp"

namespace ava::raii
{
    class GraphicsPipeline
    {
    public:
        explicit GraphicsPipeline(const ava::GraphicsPipeline& existingPipeline);
        ~GraphicsPipeline();

        ava::GraphicsPipeline pipeline;

        GraphicsPipeline(const GraphicsPipeline& other) = delete;
        GraphicsPipeline& operator=(GraphicsPipeline& other) = delete;
        GraphicsPipeline(GraphicsPipeline&& other) noexcept;
        GraphicsPipeline& operator=(GraphicsPipeline&& other) noexcept;

        void bind(const Pointer<CommandBuffer>& commandBuffer) const;

        static Pointer<GraphicsPipeline> create(const GraphicsPipelineCreationInfo& creationInfo);
    };

    // Graphics pipeline population
    void populateGraphicsPipelineCreationInfo(GraphicsPipelineCreationInfo& pipelineCreationInfo, const std::vector<Pointer<Shader>>& shaders, const Pointer<RenderPass>& renderPass, uint32_t subpass, const Pointer<VAO>& vao, bool depthTest, bool depthWrite);
}

#endif
