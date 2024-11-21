#ifndef AVA_DESCRIPTORS_HPP
#define AVA_DESCRIPTORS_HPP

#include "shaders.hpp"
#include "buffer.hpp"
#include "image.hpp"
#include "sampler.hpp"
#include "types.hpp"

namespace ava
{
    // Descriptor pools
    [[nodiscard]] DescriptorPool createDescriptorPool(const GraphicsPipeline& graphicsPipeline, uint32_t maxSetsMultiplier = 32);
    void resetDescriptorPool(const DescriptorPool& descriptorPool);
    void destroyDescriptorPool(DescriptorPool& descriptorPool);

    // Descriptor sets
    [[nodiscard]] DescriptorSet allocateDescriptorSet(const DescriptorPool& descriptorPool, uint32_t set);
    [[nodiscard]] std::vector<DescriptorSet> allocateDescriptorSets(const DescriptorPool& descriptorPool, uint32_t set, uint32_t count);
    void freeDescriptorSet(const DescriptorPool& descriptorPool, DescriptorSet& descriptorSet);
    void freeDescriptorSets(const DescriptorPool& descriptorPool, const std::vector<DescriptorSet>& descriptorSet);

    void bindDescriptorSet(const CommandBuffer& commandBuffer, const DescriptorSet& set);

    void bindBuffer(const DescriptorSet& descriptorSet, uint32_t binding, const Buffer& buffer, vk::DeviceSize bufferSize = vk::WholeSize, vk::DeviceSize bufferOffset = 0, uint32_t dstArrayElement = 0);
    void bindImage(const DescriptorSet& descriptorSet, uint32_t binding, const Image& image, const ImageView& imageView, const Sampler& sampler = nullptr, uint32_t dstArrayElement = 0);
}

#endif
