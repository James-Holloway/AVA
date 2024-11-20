#ifndef AVA_DESCRIPTORS_HPP
#define AVA_DESCRIPTORS_HPP

#include "shaders.hpp"
#include "buffer.hpp"
#include <memory>

namespace ava
{
    namespace detail
    {
        struct DescriptorPool;
        struct DescriptorSet;
    }

    using DescriptorPool = detail::DescriptorPool*;
    using DescriptorSet = std::weak_ptr<detail::DescriptorSet>;

    // Descriptor pools
    DescriptorPool createDescriptorPool(const GraphicsPipeline& graphicsPipeline, uint32_t maxSetsMultiplier = 32);
    void resetDescriptorPool(const DescriptorPool& descriptorPool);
    void destroyDescriptorPool(DescriptorPool& descriptorPool);

    // Descriptor sets
    DescriptorSet allocateDescriptorSet(const DescriptorPool& descriptorPool, uint32_t set);
    std::vector<DescriptorSet> allocateDescriptorSets(const DescriptorPool& descriptorPool, uint32_t set, uint32_t count);
    void freeDescriptorSet(const DescriptorPool& descriptorPool, DescriptorSet& descriptorSet);
    void freeDescriptorSets(const DescriptorPool& descriptorPool, const std::vector<DescriptorSet>& descriptorSet);

    void bindDescriptorSet(const vk::CommandBuffer& commandBuffer, const DescriptorSet& set);

    void bindBuffer(const DescriptorSet& descriptorSet, uint32_t binding, const Buffer& buffer, vk::DeviceSize bufferSize = vk::WholeSize, vk::DeviceSize bufferOffset = 0, uint32_t dstArrayElement = 0);
}

#endif
