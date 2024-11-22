#include "descriptors.hpp"
#include "ava/descriptors.hpp"
#include "ava/detail/descriptors.hpp"

#include "buffer.hpp"
#include "commandBuffer.hpp"
#include "compute.hpp"
#include "graphics.hpp"
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
        ava::destroyDescriptorPool(descriptorPool);
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
        const auto pool = allocatedFromPool.lock();
        AVA_CHECK_NO_EXCEPT_RETURN(pool != nullptr, "Cannot free descriptor set when allocated from pool has expired (gone out of scope before this set)");
        ava::freeDescriptorSet(pool->descriptorPool, descriptorSet);
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
}
