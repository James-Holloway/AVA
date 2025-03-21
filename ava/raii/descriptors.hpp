#ifndef AVA_RAII_DESCRIPTORS_HPP
#define AVA_RAII_DESCRIPTORS_HPP

#include "types.hpp"

namespace ava::raii
{
    class DescriptorPool : public std::enable_shared_from_this<DescriptorPool>
    {
    public:
        using Ptr = Pointer<DescriptorPool>;

        explicit DescriptorPool(const ava::DescriptorPool& existingPool);
        ~DescriptorPool();

        ava::DescriptorPool descriptorPool;

        DescriptorPool(const DescriptorPool& other) = delete;
        DescriptorPool& operator=(DescriptorPool& other) = delete;
        DescriptorPool(DescriptorPool&& other) noexcept;
        DescriptorPool& operator=(DescriptorPool&& other) noexcept;

        [[nodiscard]] Pointer<DescriptorSet> allocateDescriptorSet(uint32_t set);
        [[nodiscard]] std::vector<Pointer<DescriptorSet>> allocateDescriptorSets(uint32_t set, uint32_t count);

        static Pointer<DescriptorPool> create(const Pointer<GraphicsPipeline>& graphicsPipeline, uint32_t maxSetsMultiplier = 32);
        static Pointer<DescriptorPool> create(const Pointer<ComputePipeline>& computePipeline, uint32_t maxSetsMultiplier = 32);
        static Pointer<DescriptorPool> create(const Pointer<RayTracingPipeline>& rayTracingPipeline, uint32_t maxSetsMultiplier = 32);
    };

    class DescriptorSet
    {
    public:
        using Ptr = Pointer<DescriptorSet>;

        explicit DescriptorSet(const Pointer<DescriptorPool>& poolAllocatedFrom, const ava::DescriptorSet& existingSet);
        ~DescriptorSet();

        WeakPointer<DescriptorPool> allocatedFromPool;
        ava::DescriptorSet descriptorSet;

        DescriptorSet(const DescriptorSet& other) = delete;
        DescriptorSet& operator=(DescriptorSet& other) = delete;
        DescriptorSet(DescriptorSet&& other) noexcept;
        DescriptorSet& operator=(DescriptorSet&& other) noexcept;

        void bindDescriptorSet(const Pointer<CommandBuffer>& commandBuffer) const;

        void bindBuffer(uint32_t binding, const Pointer<Buffer>& buffer, vk::DeviceSize bufferSize = vk::WholeSize, vk::DeviceSize bufferOffset = 0, uint32_t dstArrayElement = 0) const;
        void bindNullBuffer(uint32_t binding, uint32_t dstArrayElement = 0) const;
        void bindImage(uint32_t binding, const Pointer<Image>& image, const Pointer<ImageView>& imageView, const Pointer<Sampler>& sampler = nullptr, std::optional<vk::ImageLayout> imageLayout = {}, uint32_t dstArrayElement = 0) const;
        void bindNullImage(uint32_t binding, uint32_t dstArrayElement = 0) const;
        void bindTLAS(uint32_t binding, const Pointer<TLAS>& tlas, uint32_t dstArrayElement = 0) const;
        void bindNullTLAS(uint32_t binding, uint32_t dstArrayElement = 0) const;
    };
}

#endif
