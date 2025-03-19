#ifndef AVA_IMAGE_HPP
#define AVA_IMAGE_HPP

#include "detail/vulkan.hpp"
#include "types.hpp"
#include "memoryLocation.hpp"

namespace ava
{
    constexpr vk::ImageUsageFlags DEFAULT_IMAGE_TRANSFER_USAGE_FLAGS = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
    constexpr vk::ImageUsageFlags DEFAULT_IMAGE_SAMPLED_USAGE_FLAGS = vk::ImageUsageFlagBits::eSampled | DEFAULT_IMAGE_TRANSFER_USAGE_FLAGS;
    constexpr vk::ImageUsageFlags DEFAULT_IMAGE_STORAGE_USAGE_FLAGS = vk::ImageUsageFlagBits::eStorage | DEFAULT_IMAGE_TRANSFER_USAGE_FLAGS;
    constexpr vk::ImageUsageFlags DEFAULT_IMAGE_DEPTH_ATTACHMENT_USAGE_FLAGS = vk::ImageUsageFlagBits::eDepthStencilAttachment;
    constexpr vk::ImageUsageFlags DEFAULT_IMAGE_COLOR_ATTACHMENT_USAGE_FLAGS = vk::ImageUsageFlagBits::eColorAttachment;

    // Generic and more advanced image creation
    [[nodiscard]] Image createImage(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usageFlags = DEFAULT_IMAGE_SAMPLED_USAGE_FLAGS, vk::ImageType imageType = vk::ImageType::e2D, vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
                                    uint32_t mipLevels = 1, uint32_t arrayLayers = 1, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1, MemoryLocation memoryLocation = MemoryLocation::eGpuOnly);
    // Simpler image creation
    [[nodiscard]] Image createImage2D(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usageFlags = DEFAULT_IMAGE_SAMPLED_USAGE_FLAGS, uint32_t mipLevels = 1);

    void destroyImage(Image& image);
    vk::Image getImage(const Image& image);

    // Image view creation
    [[nodiscard]] ImageView createImageView(const Image& image, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, vk::ImageViewType imageViewType = vk::ImageViewType::e2D, std::optional<vk::Format> format = {}, std::optional<vk::ImageSubresourceRange> subresourceRange = {});
    void destroyImageView(ImageView& imageView);

    vk::ImageView getImageView(const ImageView& imageView);

    void insertImageMemoryBarrier(const CommandBuffer& commandBuffer, const Image& image, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eAllCommands,
                                  vk::AccessFlags srcAccessMask = {}, vk::AccessFlags dstAccessMask = {}, std::optional<vk::ImageSubresourceRange> subresourceRange = {});
    void transitionImageLayout(const CommandBuffer& commandBuffer, const Image& image, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eAllCommands,
                               std::optional<vk::ImageSubresourceRange> subresourceRange = {});
    void overrideOldImageLayout(const Image& image, vk::ImageLayout imageLayout);

    // Update whole image using a buffer image copy
    void updateImage(const CommandBuffer& commandBuffer, const Image& image, const Buffer& stagingBuffer, const vk::BufferImageCopy& bufferImageCopy, std::optional<vk::ImageSubresourceRange> subresourceRange = {});
    void updateImage(const Image& image, const void* data, vk::DeviceSize dataSize, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, std::optional<vk::ImageSubresourceLayers> subresourceLayers = {}, std::optional<vk::ImageSubresourceRange> subresourceRange = {});

    template <typename T>
    void updateImage(const Image& image, const std::span<T> data, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, std::optional<vk::ImageSubresourceLayers> subresourceLayers = {}, std::optional<vk::ImageSubresourceRange> subresourceRange = {})
    {
        updateImage(image, data.data(), data.size() * sizeof(T), aspectFlags, subresourceLayers, subresourceRange);
    }

    template <typename T>
    void updateImage(const Image& image, const std::vector<T>& data, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, std::optional<vk::ImageSubresourceLayers> subresourceLayers = {}, std::optional<vk::ImageSubresourceRange> subresourceRange = {})
    {
        updateImage(image, data.data(), data.size() * sizeof(T), aspectFlags, subresourceLayers, subresourceRange);
    }

    ava::Image getSwapchainImage(uint32_t imageIndex);
    ava::ImageView getSwapchainImageView(uint32_t imageIndex);
    std::vector<ava::Image> getSwapchainImages();
    std::vector<ava::ImageView> getSwapchainImageViews();

    vk::ImageAspectFlags getImageAspectFlagsForFormat(vk::Format format);
}


#endif
