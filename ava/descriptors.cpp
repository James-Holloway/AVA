#include "descriptors.hpp"

#include "detail/descriptors.hpp"
#include "detail/detail.hpp"
#include "detail/shaders.hpp"
#include "detail/state.hpp"
#include <cmath>

#include "detail/buffer.hpp"
#include "detail/commandBuffer.hpp"
#include "detail/image.hpp"
#include "detail/rayTracing.hpp"
#include "detail/rayTracingPipeline.hpp"
#include "detail/sampler.hpp"

namespace ava
{
    // Use a template because every pipeline is going to have the common factors:
    // layout, layoutBindings, pushConstants and descriptorSetLayouts
    template <typename T>
    static DescriptorPool createDescriptorPoolMain(const T& pipeline, const uint32_t maxSetsMultiplier)
    {
        std::vector<vk::DescriptorSetLayoutCreateInfo> descriptorSetLayoutCreateInfos{};
        descriptorSetLayoutCreateInfos.reserve(pipeline->layoutBindings.size());
        std::vector<std::map<vk::DescriptorType, uint32_t>> setRequiredDescriptors;
        std::map<vk::DescriptorType, uint32_t> defaultPoolSizes;
        for (auto& layoutSet : pipeline->layoutBindings)
        {
            setRequiredDescriptors.push_back({});
            auto& currentSetRequiredDescriptors = setRequiredDescriptors.back();

            for (auto& layoutBinding : layoutSet)
            {
                defaultPoolSizes[layoutBinding.descriptorType] += layoutBinding.descriptorCount;
                currentSetRequiredDescriptors[layoutBinding.descriptorType] += layoutBinding.descriptorCount;
            }

            descriptorSetLayoutCreateInfos.push_back(vk::DescriptorSetLayoutCreateInfo{{}, layoutSet});
        }

        const uint32_t pipelineSets = pipeline->layoutBindings.size();

        const auto outDescriptorPool = new detail::DescriptorPool();
        outDescriptorPool->pipelineLayout = pipeline->layout;
        outDescriptorPool->layoutBindings = pipeline->layoutBindings;
        outDescriptorPool->pushConstants = pipeline->pushConstants;
        outDescriptorPool->defaultPoolSizes = defaultPoolSizes;
        outDescriptorPool->setRequiredDescriptors = setRequiredDescriptors;
        outDescriptorPool->descriptorSetLayouts = pipeline->descriptorSetLayouts;
        outDescriptorPool->defaultMaxSets = pipelineSets * maxSetsMultiplier;
        outDescriptorPool->pipelineSets = pipelineSets;
        return outDescriptorPool;
    }

    DescriptorPool createDescriptorPool(const GraphicsPipeline& graphicsPipeline, const uint32_t maxSetsMultiplier)
    {
        AVA_CHECK(graphicsPipeline != nullptr && graphicsPipeline->layout, "Cannot create descriptor pool from an invalid graphics pipeline");
        AVA_CHECK(maxSetsMultiplier > 0, "Cannot create a descriptor pool with a max sets multiplier of 0");

        return createDescriptorPoolMain(graphicsPipeline, maxSetsMultiplier);
    }

    DescriptorPool createDescriptorPool(const ComputePipeline& computePipeline, const uint32_t maxSetsMultiplier)
    {
        AVA_CHECK(computePipeline != nullptr && computePipeline->layout, "Cannot create descriptor pool from an invalid compute pipeline");
        AVA_CHECK(maxSetsMultiplier > 0, "Cannot create a descriptor pool with a max sets multiplier of 0");

        return createDescriptorPoolMain(computePipeline, maxSetsMultiplier);
    }

    DescriptorPool createDescriptorPool(const RayTracingPipeline& rayTracingPipeline, uint32_t maxSetsMultiplier)
    {
        AVA_CHECK(rayTracingPipeline != nullptr && rayTracingPipeline->layout, "Cannot create descriptor pool from an invalid ray tracing pipeline");
        AVA_CHECK(maxSetsMultiplier > 0, "Cannot create a descriptor pool with a max sets multiplier of 0");

        return createDescriptorPoolMain(rayTracingPipeline, maxSetsMultiplier);
    }

    void resetDescriptorPool(const DescriptorPool& descriptorPool)
    {
        AVA_CHECK(descriptorPool != nullptr, "Cannot reset invalid descriptor pool");
        AVA_CHECK(detail::State.device, "Cannot reset descriptor pool when State's device is invalid");

        for (auto& pool : descriptorPool->pools)
        {
            detail::State.device.resetDescriptorPool(pool.descriptorPool); // Reset the actual pool
            pool.poolSizesRemaining = pool.poolSizes; // Reset pool sizes remaining
            pool.outOfRotation = false; // Re-add to rotation if previously not
        }

        descriptorPool->sets.clear();
    }

    void destroyDescriptorPool(DescriptorPool& descriptorPool)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(descriptorPool != nullptr, "Cannot destroy invalid descriptor pool");
        AVA_CHECK_NO_EXCEPT_RETURN(detail::State.device, "Cannot destroy descriptor pool when State's device is invalid");

        for (auto& pool : descriptorPool->pools)
        {
            detail::State.device.destroyDescriptorPool(pool.descriptorPool);
            pool.descriptorPool = nullptr;
        }
        descriptorPool->sets.clear();
        descriptorPool->pools.clear();

        delete descriptorPool;
        descriptorPool = nullptr;
    }

    static detail::IndividualDescriptorPool& createIndividualDescriptorPool(const DescriptorPool& descriptorPool)
    {
        // Get pool sizes, multiplying by descriptorPool's multiplier
        std::vector<vk::DescriptorPoolSize> poolSizes;
        std::map<vk::DescriptorType, uint32_t> poolSizesMap;
        for (auto [type, size] : descriptorPool->defaultPoolSizes)
        {
            // Multiply default pool size by pool size multiplier
            size = static_cast<uint32_t>(std::ceil(size * descriptorPool->poolSizeMultiplier) + 0.5f);
            size = std::max(size, 1u);
            poolSizes.emplace_back(type, size);
            poolSizesMap[type] = size;
        }

        // Pool create info
        vk::DescriptorPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.setPoolSizes(poolSizes);
        // Multiply default max sets by pool size multiplier
        poolCreateInfo.maxSets = std::max(static_cast<uint32_t>(std::ceil(descriptorPool->defaultMaxSets * descriptorPool->poolSizeMultiplier) + 0.5f), 1u);
        if (detail::State.apiVersion.major >= 1 && detail::State.apiVersion.minor >= 2) // Flags require Vulkan 1.2
        {
            poolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
        }

        // Create the Vulkan descriptor pool
        const auto vkDescriptorPool = detail::State.device.createDescriptorPool(poolCreateInfo);

        // Create individual pool
        descriptorPool->pools.push_back(detail::IndividualDescriptorPool{});
        auto& individualPool = descriptorPool->pools.back();

        individualPool.poolIndex = ++detail::State.descriptorPoolIndexCounter;
        individualPool.creationFlags = poolCreateInfo.flags;
        individualPool.poolSizes = poolSizesMap;
        individualPool.poolSizesRemaining = poolSizesMap;
        individualPool.descriptorPool = vkDescriptorPool;
        individualPool.maxSets = poolCreateInfo.maxSets;

        // Increase pool size multiplier
        descriptorPool->poolSizeMultiplier *= 1.5f;
        return individualPool;
    }

    // Returns whether the individual pool has enough descriptor sets to assign to
    static bool hasEnoughDescriptorSets(const detail::IndividualDescriptorPool& pool, const std::map<vk::DescriptorType, uint32_t>& required)
    {
        if (pool.outOfRotation)
        {
            return false;
        }

        for (const auto& [type, count] : required)
        {
            if (!pool.poolSizesRemaining.contains(type) || pool.poolSizesRemaining.at(type) < count)
            {
                return false;
            }
        }

        return true;
    }

    static detail::IndividualDescriptorPool& getIndividualDescriptorPool(const DescriptorPool& descriptorPool, const std::map<vk::DescriptorType, uint32_t>& required)
    {
        for (auto& pool : descriptorPool->pools)
        {
            if (hasEnoughDescriptorSets(pool, required))
            {
                return pool;
            }
        }

        return createIndividualDescriptorPool(descriptorPool);
    }

    static void reducePoolSizesRemaining(detail::IndividualDescriptorPool& pool, const std::map<vk::DescriptorType, uint32_t>& consumed)
    {
        for (auto& [type, count] : consumed)
        {
            [[unlikely]] if (!pool.poolSizesRemaining.contains(type)) continue;
            pool.poolSizesRemaining[type] -= count;
        }
    }

    static DescriptorSet allocateDescriptorSetMain(const DescriptorPool& descriptorPool, const uint32_t set, uint32_t attemptsRemaining)
    {
        if (attemptsRemaining <= 0)
        {
            return {}; // Return nullptr
        }

        const auto& requiredForSet = descriptorPool->setRequiredDescriptors.at(set);

        auto& pool = getIndividualDescriptorPool(descriptorPool, requiredForSet);

        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
        descriptorSetAllocateInfo.descriptorPool = pool.descriptorPool;
        descriptorSetAllocateInfo.setSetLayouts(descriptorPool->descriptorSetLayouts.at(set));

        vk::DescriptorSet descriptorSet;
        auto result = detail::State.device.allocateDescriptorSets(&descriptorSetAllocateInfo, &descriptorSet);

        reducePoolSizesRemaining(pool, requiredForSet);
        if (result == vk::Result::eErrorOutOfPoolMemory) // Hopefully unlikely
        {
            allocateDescriptorSetMain(descriptorPool, set, attemptsRemaining - 1);
        }
        else if (result == vk::Result::eErrorFragmentedPool)
        {
            pool.outOfRotation = true; // If pool is fragmented then remove pool from the rotation
            allocateDescriptorSetMain(descriptorPool, set, attemptsRemaining - 1);
        }

        auto outDescriptorSet = std::make_shared<detail::DescriptorSet>();
        outDescriptorSet->freeable = (pool.creationFlags & vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet) != vk::DescriptorPoolCreateFlags{};
        outDescriptorSet->descriptorSet = descriptorSet;
        outDescriptorSet->poolIndex = pool.poolIndex;
        outDescriptorSet->setIndex = set;
        outDescriptorSet->setLayoutBindings = descriptorPool->layoutBindings.at(set);

        // Save the set
        descriptorPool->sets.push_back(outDescriptorSet);
        return outDescriptorSet;
    }

    DescriptorSet allocateDescriptorSet(const DescriptorPool& descriptorPool, uint32_t set)
    {
        AVA_CHECK(descriptorPool != nullptr && descriptorPool->pipelineLayout, "Cannot allocate a descriptor set from an invalid descriptor pool");
        AVA_CHECK(set < descriptorPool->pipelineSets, "Cannot allocate a descriptor set from an out-of-range set " + std::to_string(set) + " out of " + std::to_string(descriptorPool->pipelineSets));

        auto outDescriptorSet = allocateDescriptorSetMain(descriptorPool, set, 3); // Attempt to allocate 3 times
        AVA_CHECK(!outDescriptorSet.expired(), "Failed to allocate descriptor set");
        return outDescriptorSet;
    }

    std::vector<DescriptorSet> allocateDescriptorSets(const DescriptorPool& descriptorPool, uint32_t set, uint32_t count)
    {
        AVA_CHECK(descriptorPool != nullptr, "Cannot allocate descriptor sets from an invalid descriptor pool");
        AVA_CHECK(set < descriptorPool->pipelineSets, "Cannot allocate descriptor sets for set " + std::to_string(set) + " as it is larger than the number of sets for this pool (" + std::to_string(descriptorPool->pipelineSets) + ")");

        std::vector<DescriptorSet> outDescriptorSets;
        outDescriptorSets.reserve(count);
        for (uint32_t i = 0; i < count; i++)
        {
            outDescriptorSets.emplace_back(allocateDescriptorSet(descriptorPool, set));
        }

        return outDescriptorSets;
    }

    void freeDescriptorSet(const DescriptorPool& descriptorPool, DescriptorSet& descriptorSetWeak)
    {
        auto descriptorSet = descriptorSetWeak.lock();
        AVA_CHECK(descriptorPool != nullptr, "Cannot free descriptor set from invalid descriptor pool");
        AVA_CHECK(descriptorSet != nullptr, "Cannot free invalid descriptor set");
        AVA_CHECK(detail::State.device, "Cannot free descriptor set when State's device is invalid");

        if (descriptorSet->descriptorSet && descriptorSet->freeable)
        {
            bool freed = false;
            for (const auto& pool : descriptorPool->pools)
            {
                if (descriptorSet->poolIndex == pool.poolIndex)
                {
                    detail::State.device.freeDescriptorSets(pool.descriptorPool, descriptorSet->descriptorSet);
                    freed = true;
                    break;
                }
            }
            AVA_CHECK(freed, "Unable to free descriptor set from a descriptor pool that did not allocate it");
        }

        // Free descriptor set
        const auto& [removedFirst, removedLast] = std::ranges::remove_if(descriptorPool->sets, [&descriptorSet](const std::shared_ptr<detail::DescriptorSet>& set) -> bool
        {
            return descriptorSet == set;
        });

        descriptorPool->sets.erase(removedFirst, removedLast);
        descriptorSetWeak = {}; // Expire descriptor set pointer
    }

    void freeDescriptorSets(const DescriptorPool& descriptorPool, const std::vector<DescriptorSet>& descriptorSets)
    {
        for (auto& descriptorSet : descriptorSets)
        {
            auto ds = descriptorSet;
            freeDescriptorSet(descriptorPool, ds);
        }
    }

    void bindDescriptorSet(const ava::CommandBuffer& commandBuffer, const DescriptorSet& set)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot bind a descriptor set to an invalid command buffer");
        AVA_CHECK(!set.expired(), "Cannot bind an invalid descriptor set");
        const auto ds = set.lock();
        AVA_CHECK(ds->descriptorSet, "Cannot bind an invalid descriptor set");
        AVA_CHECK(commandBuffer->pipelineCurrentlyBound, "Cannot bind a descriptor set when no pipeline is currently bound");

        commandBuffer->commandBuffer.bindDescriptorSets(commandBuffer->currentPipelineBindPoint, commandBuffer->currentPipelineLayout, ds->setIndex, ds->descriptorSet, nullptr);
    }

    static std::optional<vk::DescriptorType> getDescriptorType(const std::shared_ptr<detail::DescriptorSet>& descriptorSet, const uint32_t binding)
    {
        for (auto& layoutBinding : descriptorSet->setLayoutBindings)
        {
            if (layoutBinding.binding == binding)
            {
                return layoutBinding.descriptorType;
            }
        }
        return {};
    }

    void bindBuffer(const DescriptorSet& descriptorSet, const uint32_t binding, const Buffer& buffer, const vk::DeviceSize bufferSize, const vk::DeviceSize bufferOffset, const uint32_t dstArrayElement)
    {
        AVA_CHECK(detail::State.device, "Cannot bind a buffer to a descriptor set when State's device is invalid");
        AVA_CHECK(!descriptorSet.expired(), "Cannot bind a buffer to an invalid descriptor set");
        const auto ds = descriptorSet.lock();
        AVA_CHECK(ds->descriptorSet, "Cannot bind a buffer to an invalid descriptor set");
        AVA_CHECK(buffer != nullptr, "Cannot bind an invalid buffer to a descriptor set");
        AVA_CHECK(bufferSize > 0, "Cannot bind a buffer to a descriptor set with bufferSize of 0")
        if (bufferSize != vk::WholeSize)
        {
            AVA_CHECK((bufferSize + bufferOffset) <= buffer->size, "Cannot bind a buffer to a descriptor set when bufferSize + bufferOffset (" + std::to_string(bufferSize + bufferOffset) + ") is greater than buffer size (" + std::to_string(buffer->size) + ")");
        }
        else
        {
            AVA_CHECK(bufferOffset < buffer->size, "Cannot bind a buffer to a descriptor set when bufferOffset (" + std::to_string(bufferOffset) +") is not less than buffer's size (" + std::to_string(buffer->size) + ")");
        }

        const auto descriptorType = getDescriptorType(ds, binding);
        if (!descriptorType.has_value())
        {
            AVA_WARN("Could not bind a buffer to a descriptor set, binding " + std::to_string(binding) + " when the descriptor type could not be found from the layout bindings (does the binding exist in the shader?)");
            return; // If no binding type could be found then don't do any binding
        }

        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer->buffer;
        bufferInfo.offset = bufferOffset;
        bufferInfo.range = bufferSize;

        vk::WriteDescriptorSet wds{};
        wds.dstSet = ds->descriptorSet;
        wds.dstBinding = binding;
        wds.dstArrayElement = dstArrayElement;
        wds.descriptorCount = 1;
        wds.descriptorType = descriptorType.value();
        wds.setBufferInfo(bufferInfo);

        detail::State.device.updateDescriptorSets(wds, nullptr);
    }

    void bindImage(const DescriptorSet& descriptorSet, uint32_t binding, const Image& image, const ImageView& imageView, const Sampler& sampler, std::optional<vk::ImageLayout> imageLayout, const uint32_t dstArrayElement)
    {
        AVA_CHECK(detail::State.device, "Cannot bind an image to a descriptor set when State's device is invalid");
        AVA_CHECK(!descriptorSet.expired(), "Cannot bind an image to an invalid descriptor set");
        const auto ds = descriptorSet.lock();
        AVA_CHECK(ds->descriptorSet, "Cannot bind an image to an invalid descriptor set");
        AVA_CHECK(image != nullptr, "Cannot bind an invalid image to a descriptor set");
        AVA_CHECK(imageView != nullptr && imageView->imageView, "Cannot bind an image to a descriptor set when image view is invalid");
        if (sampler != nullptr)
        {
            AVA_CHECK(sampler->sampler, "Cannot bind an image to a descriptor set as the provided sampler was invalid");
        }

        const auto descriptorType = getDescriptorType(ds, binding);
        if (!descriptorType.has_value())
        {
            AVA_WARN("Could not bind an image to a descriptor set, binding " + std::to_string(binding) + " when the descriptor type could not be found from the layout bindings (does the binding exist in the shader?)");
            return; // If no binding type could be found then don't do any binding
        }

        vk::DescriptorImageInfo imageInfo{};
        imageInfo.sampler = sampler != nullptr ? sampler->sampler : nullptr;
        imageInfo.imageLayout = imageLayout.value_or(image->imageLayout);
        imageInfo.imageView = imageView->imageView;

        vk::WriteDescriptorSet wds{};
        wds.dstSet = ds->descriptorSet;
        wds.dstBinding = binding;
        wds.dstArrayElement = dstArrayElement;
        wds.descriptorCount = 1;
        wds.descriptorType = descriptorType.value();
        wds.setImageInfo(imageInfo);

        detail::State.device.updateDescriptorSets(wds, nullptr);
    }

    void bindTLAS(const DescriptorSet& descriptorSet, uint32_t binding, const TLAS& tlas, uint32_t dstArrayElement)
    {
        AVA_CHECK(detail::State.device, "Cannot bind an image to a descriptor set when State's device is invalid");
        AVA_CHECK(detail::State.rayTracingEnabled, "Cannot bind a TLAS to a descriptor set when ray tracing is not enabled");
        AVA_CHECK(!descriptorSet.expired(), "Cannot bind a TLAS to an invalid descriptor set");
        const auto ds = descriptorSet.lock();
        AVA_CHECK(ds->descriptorSet, "Cannot bind a TLAS to an invalid descriptor set");
        AVA_CHECK(tlas != nullptr, "Cannot bind an invalid TLAS to a descriptor set");
        AVA_CHECK(tlas->built, "Cannot bind an un-built TLAS to a descriptor set");
        AVA_CHECK(tlas->accelerationStructure, "Cannot bind a TLAS to a descriptor set when TLAS' acceleration structure is invalid");

        const auto descriptorType = getDescriptorType(ds, binding);
        if (!descriptorType.has_value())
        {
            AVA_WARN("Could not bind a TLAS to a descriptor set, binding " + std::to_string(binding) + " when the descriptor type could not be found from the layout bindings (does the binding exist in the shader?)");
            return; // If no binding type could be found then don't do any binding
        }

        vk::WriteDescriptorSetAccelerationStructureKHR accelerationStructureInfo{};
        accelerationStructureInfo.setAccelerationStructures(tlas->accelerationStructure->accelerationStructure);

        vk::WriteDescriptorSet wds;
        wds.dstSet = ds->descriptorSet;
        wds.dstBinding = binding;
        wds.dstArrayElement = dstArrayElement;
        wds.descriptorCount = 1;
        wds.descriptorType = descriptorType.value();
        wds.pNext = &accelerationStructureInfo;

        detail::State.device.updateDescriptorSets(wds, nullptr);
    }
}
