#ifndef AVA_RAII_TYPES_HPP
#define AVA_RAII_TYPES_HPP

#include "../types.hpp"

namespace ava::raii
{
    class Buffer;
    class Image;
    class ImageView;
    class Sampler;
    class CommandBuffer;
    class ComputePipeline;
    class GraphicsPipeline;
    class DescriptorPool;
    class DescriptorSet;
    class VAO;
    class VBO;
    class IBO;
    class RenderPass;
    class Framebuffer;

    template <typename T>
    using Pointer = std::shared_ptr<T>;

    template <typename T>
    using WeakPointer = std::weak_ptr<T>;
}

#endif
