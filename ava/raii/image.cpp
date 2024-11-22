#include "image.hpp"
#include "ava/image.hpp"
#include "ava/detail/image.hpp"

#include "buffer.hpp"
#include "commandBuffer.hpp"
#include "ava/detail/detail.hpp"

namespace ava::raii
{
    Image::Image(const ava::Image& existingImage)
    {
        AVA_CHECK(existingImage != nullptr && existingImage->image, "Cannot create a RAII image from an invalid image");

        image = existingImage;
    }

    Image::~Image()
    {
        destroyImage(image);
    }

    void Image::insertImageMemoryBarrier(const Pointer<CommandBuffer>& commandBuffer, const vk::ImageLayout newLayout, const vk::AccessFlags srcAccessMask, const vk::AccessFlags dstAccessMask, const vk::ImageAspectFlags aspectFlags, const vk::PipelineStageFlags srcStage, const vk::PipelineStageFlags dstStage,
                                         const std::optional<vk::ImageSubresourceRange>& subresourceRange) const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer != nullptr, "Cannot insert an image memory barrier when command buffer is invalid");
        ava::insertImageMemoryBarrier(commandBuffer->commandBuffer, image, newLayout, srcAccessMask, dstAccessMask, aspectFlags, srcStage, dstStage, subresourceRange);
    }

    void Image::transitionImageLayout(const Pointer<CommandBuffer>& commandBuffer, const vk::ImageLayout newLayout, const vk::ImageAspectFlags aspectFlags, const vk::PipelineStageFlags srcStage, const vk::PipelineStageFlags dstStage, const std::optional<vk::ImageSubresourceRange>& subresourceRange) const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer != nullptr, "Cannot transition image layout when command buffer is invalid");
        ava::transitionImageLayout(commandBuffer->commandBuffer, image, newLayout, aspectFlags, srcStage, dstStage, subresourceRange);
    }

    void Image::update(const Pointer<CommandBuffer>& commandBuffer, const Pointer<Buffer>& stagingBuffer, const vk::BufferImageCopy& bufferImageCopy, const std::optional<vk::ImageSubresourceRange>& subresourceRange) const
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot update image with an invalid command buffer");
        updateImage(commandBuffer->commandBuffer, image, stagingBuffer->buffer, bufferImageCopy, subresourceRange);
    }

    void Image::update(const void* data, const vk::DeviceSize dataSize, const vk::ImageAspectFlags aspectFlags, const std::optional<vk::ImageSubresourceLayers>& subresourceLayers, const std::optional<vk::ImageSubresourceRange>& subresourceRange) const
    {
        updateImage(image, data, dataSize, aspectFlags, subresourceLayers, subresourceRange);
    }

    Pointer<ImageView> Image::createImageView(const vk::ImageAspectFlags aspectFlags, const vk::ImageViewType imageViewType, const std::optional<vk::Format> format, const std::optional<vk::ImageSubresourceRange>& subresourceRange) const
    {
        return std::make_shared<ImageView>(ava::createImageView(image, aspectFlags, imageViewType, format, subresourceRange));
    }

    Pointer<Image> Image::create(const vk::Extent3D extent, const vk::Format format, const vk::ImageUsageFlags usageFlags, const vk::ImageType imageType, const vk::ImageTiling tiling, const uint32_t mipLevels, const uint32_t arrayLayers, const vk::SampleCountFlagBits samples, const vma::MemoryUsage memoryUsage)
    {
        return std::make_shared<Image>(ava::createImage(extent, format, usageFlags, imageType, tiling, mipLevels, arrayLayers, samples, memoryUsage));
    }

    Pointer<Image> Image::create2D(const vk::Extent2D extent, const vk::Format format, const vk::ImageUsageFlags usageFlags, const uint32_t mipLevels)
    {
        return std::make_shared<Image>(ava::createImage2D(extent, format, usageFlags, mipLevels));
    }

    ImageView::ImageView(const ava::ImageView& existingImageView)
    {
        AVA_CHECK(existingImageView != nullptr && existingImageView->imageView != nullptr, "Cannot create a RAII image view from an invalid iamge view")
        imageView = existingImageView;
    }

    ImageView::~ImageView()
    {
        ava::destroyImageView(imageView);
    }
}
