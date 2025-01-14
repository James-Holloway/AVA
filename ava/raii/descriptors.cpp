#include "descriptors.hpp"
#include "ava/descriptors.hpp"
#include "ava/detail/descriptors.hpp"

#include "buffer.hpp"
#include "commandBuffer.hpp"
#include "compute.hpp"
#include "graphics.hpp"
#include "image.hpp"
#include "sampler.hpp"
#include "rayTracing.hpp"
#include "rayTracingPipeline.hpp"
#include "ava/detail/detail.hpp"

namespace ava::raii
{
    DescriptorPool::DescriptorPool(const ava::DescriptorPool& existingPool)
    {
        AVA_CHECK(existingPool != nullptr, "Cannot create a RAII descriptor pool from an invalid descriptor pool");

        descriptorPool = existingPool;
    }

    DescriptorPool::~DescriptorPool()
    {
        if (descriptorPool != nullptr)
        {
            ava::destroyDescriptorPool(descriptorPool);
        }
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
    {
        descriptorPool = other.descriptorPool;
        other.descriptorPool = nullptr;
    }

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept
    {
        if (this != &other)
        {
            descriptorPool = other.descriptorPool;
            other.descriptorPool = nullptr;
        }
        return *this;
    }

    Pointer<DescriptorSet> DescriptorPool::allocateDescriptorSet(const uint32_t set)
    {
        return std::make_shared<DescriptorSet>(shared_from_this(), ava::allocateDescriptorSet(descriptorPool, set));
    }

    std::vector<Pointer<DescriptorSet>> DescriptorPool::allocateDescriptorSets(const uint32_t set, const uint32_t count)
    {
        std::vector<Pointer<DescriptorSet>> descriptorSets;
        descriptorSets.reserve(count);
        for (uint32_t i = 0; i < count; i++)
        {
            descriptorSets.push_back(allocateDescriptorSet(set));
        }
        return descriptorSets;
    }

    Pointer<DescriptorPool> DescriptorPool::create(const Pointer<GraphicsPipeline>& graphicsPipeline, const uint32_t maxSetsMultiplier)
    {
        AVA_CHECK(graphicsPipeline != nullptr && graphicsPipeline->pipeline, "Cannot create a descriptor pool from an invalid graphics pipeline");
        return std::make_shared<DescriptorPool>(ava::createDescriptorPool(graphicsPipeline->pipeline, maxSetsMultiplier));
    }

    Pointer<DescriptorPool> DescriptorPool::create(const Pointer<ComputePipeline>& computePipeline, const uint32_t maxSetsMultiplier)
    {
        AVA_CHECK(computePipeline != nullptr && computePipeline->pipeline, "Cannot create a descriptor pool from an invalid compute pipeline");
        return std::make_shared<DescriptorPool>(ava::createDescriptorPool(computePipeline->pipeline, maxSetsMultiplier));
    }

    Pointer<DescriptorPool> DescriptorPool::create(const Pointer<RayTracingPipeline>& rayTracingPipeline, const uint32_t maxSetsMultiplier)
    {
        AVA_CHECK(rayTracingPipeline != nullptr && rayTracingPipeline->pipeline, "Cannot create a descriptor pool from an invalid ray tracing pipeline");
        return std::make_shared<DescriptorPool>(ava::createDescriptorPool(rayTracingPipeline->pipeline, maxSetsMultiplier));
    }

    DescriptorSet::DescriptorSet(const Pointer<DescriptorPool>& poolAllocatedFrom, const ava::DescriptorSet& existingSet)
    {
        AVA_CHECK(!existingSet.expired(), "Cannot create a RAII descriptor set from an invalid descriptor set");
        const auto ds = existingSet.lock();
        AVA_CHECK(ds != nullptr && ds->descriptorSet, "Cannot create a RAII descriptor set from an invalid descriptor set")
        AVA_CHECK(poolAllocatedFrom != nullptr && poolAllocatedFrom->descriptorPool != nullptr, "Cannot create a RAII descriptor set when the pool allocated from is invalid");

        allocatedFromPool = poolAllocatedFrom;
        descriptorSet = ds;
    }

    DescriptorSet::~DescriptorSet()
    {
        if (!descriptorSet.expired())
        {
            const auto pool = allocatedFromPool.lock();
            AVA_CHECK_NO_EXCEPT_RETURN(pool != nullptr, "Cannot free descriptor set when allocated from pool has expired (gone out of scope before this set)");
            ava::freeDescriptorSet(pool->descriptorPool, descriptorSet);
        }
    }

    DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept
    {
        descriptorSet = other.descriptorSet;
        allocatedFromPool = other.allocatedFromPool;
        other.descriptorSet = {};
        other.allocatedFromPool = {};
    }

    DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept
    {
        if (this != &other)
        {
            descriptorSet = other.descriptorSet;
            allocatedFromPool = other.allocatedFromPool;
            other.descriptorSet = {};
            other.allocatedFromPool = {};
        }
        return *this;
    }

    void DescriptorSet::bindDescriptorSet(const Pointer<CommandBuffer>& commandBuffer) const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer != nullptr, "Cannot bind descriptor set with an invalid command buffer")
        ava::bindDescriptorSet(commandBuffer->commandBuffer, descriptorSet);
    }

    void DescriptorSet::bindBuffer(const uint32_t binding, const Pointer<Buffer>& buffer, const vk::DeviceSize bufferSize, const vk::DeviceSize bufferOffset, const uint32_t dstArrayElement) const
    {
        AVA_CHECK(buffer != nullptr && buffer->buffer, "Cannot bind buffer to a descriptor set when buffer is invalid");
        ava::bindBuffer(descriptorSet, binding, buffer->buffer, bufferSize, bufferOffset, dstArrayElement);
    }

    void DescriptorSet::bindNullBuffer(const uint32_t binding, const uint32_t dstArrayElement) const
    {
        ava::bindNullBuffer(descriptorSet, binding, dstArrayElement);
    }

    void DescriptorSet::bindImage(const uint32_t binding, const Pointer<Image>& image, const Pointer<ImageView>& imageView, const Pointer<Sampler>& sampler, const std::optional<vk::ImageLayout> imageLayout, const uint32_t dstArrayElement) const
    {
        AVA_CHECK(image != nullptr && image->image, "Cannot bind image to a descriptor set when image is invalid");
        AVA_CHECK(imageView != nullptr && imageView->imageView != nullptr, "Cannot bind image to a descriptor set when image view is invalid");
        if (sampler != nullptr)
        {
            AVA_CHECK(sampler->sampler != nullptr, "Cannot bind image to a descriptor set when provided sampler is invalid");
        }
        ava::bindImage(descriptorSet, binding, image->image, imageView->imageView, sampler != nullptr ? sampler->sampler : nullptr, imageLayout, dstArrayElement);
    }

    void DescriptorSet::bindNullImage(const uint32_t binding, const uint32_t dstArrayElement) const
    {
        ava::bindNullImage(descriptorSet, binding, dstArrayElement);
    }

    void DescriptorSet::bindTLAS(const uint32_t binding, const Pointer<TLAS>& tlas, const uint32_t dstArrayElement) const
    {
        AVA_CHECK(tlas != nullptr && tlas->tlas, "Cannot bind TLAS to a descriptor set when TLAS is invalid");
        ava::bindTLAS(descriptorSet, binding, tlas->tlas, dstArrayElement);
    }

    void DescriptorSet::bindNullTLAS(const uint32_t binding, const uint32_t dstArrayElement) const
    {
        ava::bindNullTLAS(descriptorSet, binding, dstArrayElement);
    }
}
