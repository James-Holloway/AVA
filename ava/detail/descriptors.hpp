#ifndef AVA_DETAIL_DESCRIPTORS_HPP
#define AVA_DETAIL_DESCRIPTORS_HPP

#include <vector>
#include "./vulkan.hpp"
#include <memory>
#include <map>

namespace ava::detail
{
    struct IndividualDescriptorPool
    {
        vk::DescriptorPool descriptorPool;
        uint32_t maxSets;
        std::map<vk::DescriptorType, uint32_t> poolSizes;
        std::map<vk::DescriptorType, uint32_t> poolSizesRemaining;
        uint32_t poolIndex = 0; // 0 is never assigned to
        vk::DescriptorPoolCreateFlags creationFlags = {};
        bool outOfRotation = false;
    };

    struct DescriptorSet;

    struct DescriptorPool
    {
        std::vector<IndividualDescriptorPool> pools;
        vk::PipelineLayout pipelineLayout;
        std::vector<std::vector<vk::DescriptorSetLayoutBinding>> layoutBindings; // The pipeline's layout bindings
        std::vector<std::map<vk::DescriptorType, uint32_t>> setRequiredDescriptors; // The required amount of each descriptor type for each set
        std::vector<vk::PushConstantRange> pushConstants;
        std::map<vk::DescriptorType, uint32_t> defaultPoolSizes;
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
        std::vector<std::shared_ptr<DescriptorSet>> sets;
        uint32_t defaultMaxSets = 0;
        uint32_t pipelineSets;
        float poolSizeMultiplier = 1.0f;
    };

    struct DescriptorSet
    {
        vk::DescriptorSet descriptorSet;
        std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings;
        uint32_t poolIndex;
        uint32_t setIndex;
        bool freeable;
    };
}

#endif
