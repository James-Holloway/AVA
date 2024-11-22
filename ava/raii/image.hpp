#ifndef AVA_RAII_IMAGE_HPP
#define AVA_RAII_IMAGE_HPP

#include "types.hpp"
#include "ava/image.hpp"

namespace ava::raii
{
    class Image
    {
    public:
        explicit Image(const ava::Image& existingImage);
        ~Image();

        ava::Image image;

        Image(const Image& other) = delete;
        Image& operator=(Image& other) = delete;
        Image(Image&& other) noexcept;
        Image& operator=(Image&& other) noexcept;

        void insertImageMemoryBarrier(const Pointer<CommandBuffer>& commandBuffer, vk::ImageLayout newLayout, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor,
                                      vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eAllCommands, const std::optional<vk::ImageSubresourceRange>& subresourceRange = {}) const;
        void transitionImageLayout(const Pointer<CommandBuffer>& commandBuffer, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eAllCommands,
                                   const std::optional<vk::ImageSubresourceRange>& subresourceRange = {}) const;

        // Update whole image using a buffer image copy
        void update(const Pointer<CommandBuffer>& commandBuffer, const Pointer<Buffer>& stagingBuffer, const vk::BufferImageCopy& bufferImageCopy, const std::optional<vk::ImageSubresourceRange>& subresourceRange = {}) const;
        void update(const void* data, vk::DeviceSize dataSize, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, const std::optional<vk::ImageSubresourceLayers>& subresourceLayers = {}, const std::optional<vk::ImageSubresourceRange>& subresourceRange = {}) const;

        template <typename T>
        void update(const std::span<T> data, const vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, const std::optional<vk::ImageSubresourceLayers> subresourceLayers = {}, const std::optional<vk::ImageSubresourceRange> subresourceRange = {})
        {
            update(data.data(), data.size() * sizeof(T), aspectFlags, subresourceLayers, subresourceRange);
        }

        template <typename T>
        void update(const std::vector<T>& data, const vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, const std::optional<vk::ImageSubresourceLayers> subresourceLayers = {}, const std::optional<vk::ImageSubresourceRange> subresourceRange = {})
        {
            update(data.data(), data.size() * sizeof(T), aspectFlags, subresourceLayers, subresourceRange);
        }

        [[nodiscard]] Pointer<ImageView> createImageView(vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, vk::ImageViewType imageViewType = vk::ImageViewType::e2D, std::optional<vk::Format> format = {}, const std::optional<vk::ImageSubresourceRange>& subresourceRange = {}) const;

        // Generic and more advanced image creation
        [[nodiscard]] static Pointer<Image> create(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usageFlags = DEFAULT_IMAGE_SAMPLED_USAGE_FLAGS, vk::ImageType imageType = vk::ImageType::e2D, vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
                                                   uint32_t mipLevels = 1, uint32_t arrayLayers = 1, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1, vma::MemoryUsage memoryUsage = vma::MemoryUsage::eGpuOnly);
        // Simpler image creation
        [[nodiscard]] static Pointer<Image> create2D(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usageFlags = DEFAULT_IMAGE_SAMPLED_USAGE_FLAGS, uint32_t mipLevels = 1);
    };

    class ImageView
    {
    public:
        explicit ImageView(const ava::ImageView& existingImageView);
        ~ImageView();

        ava::ImageView imageView;

        ImageView(const ImageView& other) = delete;
        ImageView& operator=(ImageView& other) = delete;
    };
}

#endif
