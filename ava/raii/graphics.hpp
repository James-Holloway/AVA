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

        void bind(const Pointer<CommandBuffer>& commandBuffer) const;

        static Pointer<GraphicsPipeline> create(const GraphicsPipelineCreationInfo& creationInfo);
    };
}

#endif
