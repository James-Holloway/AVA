#include "image.hpp"

#include "detail/image.hpp"
#include "detail/buffer.hpp"
#include "detail/commandBuffer.hpp"
#include "detail/detail.hpp"
#include "detail/state.hpp"
#include "buffer.hpp"
#include "commandBuffer.hpp"

namespace ava
{
    Image createImage(const vk::Extent3D extent, const vk::Format format, const vk::ImageUsageFlags usageFlags, const vk::ImageType imageType, const vk::ImageTiling tiling, const uint32_t mipLevels, const uint32_t arrayLayers, const vk::SampleCountFlagBits samples, const MemoryLocation memoryLocation)
    {
        AVA_CHECK(extent.width > 0 && extent.height > 0 && extent.depth > 0, "Invalid image extent when creating image");
        AVA_CHECK(detail::State.allocator, "Cannot create an image without a valid State allocator");

        vk::ImageCreateInfo createInfo;
        createInfo.imageType = imageType;
        createInfo.extent = extent;
        createInfo.tiling = tiling;
        createInfo.mipLevels = mipLevels;
        createInfo.arrayLayers = arrayLayers;
        createInfo.format = format;
        createInfo.initialLayout = vk::ImageLayout::eUndefined;
        createInfo.usage = usageFlags;
        createInfo.samples = samples;
        createInfo.sharingMode = vk::SharingMode::eExclusive;

        vma::AllocationCreateInfo allocInfo;
        allocInfo.usage = getMemoryUsageFromBufferLocation(memoryLocation);

        const auto pair = detail::State.allocator.createImage(createInfo, allocInfo);
        const auto allocationInfo = detail::State.allocator.getAllocationInfo(pair.second);

        auto newImage = new detail::Image();
        newImage->image = pair.first;
        newImage->allocation = pair.second;
        newImage->imageLayout = vk::ImageLayout::eUndefined;
        newImage->creationInfo = createInfo;
        newImage->allocationInfo = allocationInfo;

        auto commandBuffer = beginSingleTimeCommands(vk::QueueFlagBits::eTransfer);
        transitionImageLayout(commandBuffer, newImage, vk::ImageLayout::eGeneral, getImageAspectFlagsForFormat(format));
        endSingleTimeCommands(commandBuffer);
        return newImage;
    }

    Image createImage2D(const vk::Extent2D extent, const vk::Format format, const vk::ImageUsageFlags usageFlags, const uint32_t mipLevels)
    {
        return createImage(vk::Extent3D{extent, 1}, format, usageFlags, vk::ImageType::e2D, vk::ImageTiling::eOptimal, mipLevels, 1, vk::SampleCountFlagBits::e1, MemoryLocation::eGpuOnly);
    }

    void destroyImage(Image& image)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(image != nullptr, "Cannot destroy an invalid image view")
        AVA_CHECK_NO_EXCEPT_RETURN(detail::State.device, "Cannot destroy image when State's device is invalid");
        AVA_CHECK_NO_EXCEPT_RETURN(!image->isSwapchainImage, "Cannot destroy swapchain image");

        if (image->image)
        {
            if (image->allocation)
            {
                detail::State.allocator.destroyImage(image->image, image->allocation);
            }
            else
            {
                detail::State.device.destroyImage(image->image);
            }
        }

        delete image;
        image = nullptr;
    }

    vk::Image getImage(const Image& image)
    {
        AVA_CHECK(image != nullptr && image->image, "Cannot get an image from an invalid image");

        return image->image;
    }

    ImageView createImageView(const Image& image, const vk::ImageAspectFlags aspectFlags, const vk::ImageViewType imageViewType, const std::optional<vk::Format> format, std::optional<vk::ImageSubresourceRange> subresourceRange)
    {
        AVA_CHECK(image != nullptr && image->image, "Cannot create an image view from an invalid image");
        if (!subresourceRange.has_value())
        {
            subresourceRange = vk::ImageSubresourceRange{aspectFlags, 0, image->creationInfo.mipLevels, 0, image->creationInfo.arrayLayers};
        }

        const auto selectedFormat = format.value_or(image->creationInfo.format);

        vk::ImageViewCreateInfo createInfo;
        createInfo.image = image->image;
        createInfo.format = selectedFormat;
        createInfo.viewType = imageViewType;
        createInfo.subresourceRange = subresourceRange.value();
        createInfo.setComponents(vk::ComponentMapping{});

        const auto imageView = detail::State.device.createImageView(createInfo);

        const auto outImageView = new detail::ImageView();
        outImageView->imageView = imageView;
        outImageView->format = selectedFormat;

        return outImageView;
    }

    void destroyImageView(ImageView& imageView)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(imageView != nullptr, "Cannot destroy an invalid image view")
        AVA_CHECK_NO_EXCEPT_RETURN(detail::State.device, "Cannot destroy image view when State's device is invalid");
        AVA_CHECK_NO_EXCEPT_RETURN(!imageView->isSwapchainImageView, "Cannot destroy swapchain image view");

        if (imageView->imageView)
        {
            detail::State.device.destroyImageView(imageView->imageView);
        }

        delete imageView;
        imageView = nullptr;
    }

    vk::ImageView getImageView(const ImageView& imageView)
    {
        AVA_CHECK(imageView != nullptr && imageView->imageView, "Cannot get vulkan image view from an invalid image view");

        return imageView->imageView;
    }

    void insertImageMemoryBarrier(const CommandBuffer& commandBuffer, const Image& image, const vk::ImageLayout newLayout, const vk::AccessFlags srcAccessMask, const vk::AccessFlags dstAccessMask, const vk::ImageAspectFlags aspectFlags, const vk::PipelineStageFlags srcStage, const vk::PipelineStageFlags dstStage,
                                  std::optional<vk::ImageSubresourceRange> subresourceRange)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot insert image memory barrier when command buffer is invalid");
        AVA_CHECK(image != nullptr && image->image, "Cannot insert image memory barrier when image is invalid");

        const auto oldLayout = image->imageLayout;
        if (oldLayout == newLayout)
        {
            return;
        }

        if (!subresourceRange.has_value())
        {
            subresourceRange = vk::ImageSubresourceRange{aspectFlags, 0, image->creationInfo.mipLevels, 0, image->creationInfo.arrayLayers};
        }

        vk::ImageMemoryBarrier barrier;
        barrier.image = image->image;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcAccessMask = srcAccessMask;
        barrier.dstAccessMask = dstAccessMask;
        barrier.subresourceRange = subresourceRange.value();

        constexpr vk::DependencyFlags dependencyFlags{};

        commandBuffer->commandBuffer.pipelineBarrier(srcStage, dstStage, dependencyFlags, nullptr, nullptr, barrier);

        image->imageLayout = newLayout;
    }

    void transitionImageLayout(const CommandBuffer& commandBuffer, const Image& image, const vk::ImageLayout newLayout, const vk::ImageAspectFlags aspectFlags, const vk::PipelineStageFlags srcStage, const vk::PipelineStageFlags dstStage, std::optional<vk::ImageSubresourceRange> subresourceRange)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot insert image memory barrier when command buffer is invalid");
        AVA_CHECK(image != nullptr && image->image, "Cannot insert image memory barrier when image is invalid");

        const auto oldLayout = image->imageLayout;
        if (oldLayout == newLayout)
        {
            return;
        }

        if (!subresourceRange.has_value())
        {
            subresourceRange = vk::ImageSubresourceRange{aspectFlags, 0, image->creationInfo.mipLevels, 0, image->creationInfo.arrayLayers};
        }

        vk::ImageMemoryBarrier barrier{};
        barrier.image = image->image;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = commandBuffer->familyQueueIndex;
        barrier.dstQueueFamilyIndex = commandBuffer->familyQueueIndex;
        barrier.subresourceRange = subresourceRange.value();
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eNone;

        switch (oldLayout)
        {
        case vk::ImageLayout::eUndefined:
            // Image layout is undefined (or does not matter)
            // Only valid as initial layout
            // No flags required, listed only for completeness
            barrier.srcAccessMask = vk::AccessFlagBits::eNone;
            break;

        case vk::ImageLayout::ePreinitialized:
            // Image is pre initialized
            // Only valid as initial layout for linear images, preserves memory contents
            // Make sure host writes have been finished
            barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
            break;

        case vk::ImageLayout::eColorAttachmentOptimal:
            // Image is a color attachment
            // Make sure any writes to the color buffer have been finished
            barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
            break;

        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        case vk::ImageLayout::eDepthAttachmentOptimal:
            // Image is a depth/stencil attachment
            // Make sure any writes to the depth/stencil buffer have been finished
            barrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            break;

        case vk::ImageLayout::eTransferSrcOptimal:
            // Image is a transfer source
            // Make sure any reads from the image have been finished
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            break;

        case vk::ImageLayout::eTransferDstOptimal:
            // Image is a transfer destination
            // Make sure any writes to the image have been finished
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            break;

        case vk::ImageLayout::eReadOnlyOptimal:
            // Image is read by a shader
            // Make sure any shader reads from the image have been finished
            barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
        switch (newLayout)
        {
        case vk::ImageLayout::eTransferDstOptimal:
            // Image will be used as a transfer destination
            // Make sure any writes to the image have been finished
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            break;

        case vk::ImageLayout::eTransferSrcOptimal:
            // Image will be used as a transfer source
            // Make sure any reads from the image have been finished
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
            break;

        case vk::ImageLayout::eColorAttachmentOptimal:
            // Image will be used as a color attachment
            // Make sure any writes to the color buffer have been finished
            barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
            break;

        case vk::ImageLayout::eDepthAttachmentOptimal:
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            // Image layout will be used as a depth/stencil attachment
            // Make sure any writes to depth/stencil buffer have been finished
            barrier.dstAccessMask = barrier.dstAccessMask | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            break;

        case vk::ImageLayout::eReadOnlyOptimal:
            // Image will be read in a shader (sampler, input attachment)
            // Make sure any writes to the image have been finished
            if (barrier.srcAccessMask == vk::AccessFlagBits::eNone)
            {
                barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
            }
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            break;
        default:
            // Other source layouts aren't handled (yet)
            break;
        }

        constexpr vk::DependencyFlags dependencyFlags{};

        commandBuffer->commandBuffer.pipelineBarrier(srcStage, dstStage, dependencyFlags, nullptr, nullptr, barrier);

        image->imageLayout = newLayout;
    }

    void updateImage(const CommandBuffer& commandBuffer, const Image& image, const Buffer& stagingBuffer, const vk::BufferImageCopy& bufferImageCopy, std::optional<vk::ImageSubresourceRange> subresourceRange)
    {
        AVA_CHECK(commandBuffer != nullptr && commandBuffer->commandBuffer, "Cannot update image from staging buffer with an invalid command buffer");
        AVA_CHECK(image != nullptr && image->image, "Cannot update image from staging buffer when image is invalid");
        AVA_CHECK(stagingBuffer != nullptr && stagingBuffer->buffer, "Cannot update image from staging buffer when staging buffer is invalid");
        AVA_CHECK((image->creationInfo.usage & vk::ImageUsageFlagBits::eTransferDst) != vk::ImageUsageFlags{}, "Cannot update image as it was not created with TransferDst image usage flags");
        AVA_CHECK((stagingBuffer->bufferUsage & vk::BufferUsageFlagBits::eTransferSrc) != vk::BufferUsageFlags{}, "Cannot update image as staging buffer was not created with TransferSrc buffer usage flags");

        if (!subresourceRange.has_value())
        {
            subresourceRange = vk::ImageSubresourceRange{getImageAspectFlagsForFormat(image->creationInfo.format), 0, image->creationInfo.mipLevels, 0, image->creationInfo.arrayLayers};
        }

        const auto oldLayout = image->imageLayout;

        transitionImageLayout(commandBuffer, image, vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eNone, vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eTransfer, subresourceRange);
        commandBuffer->commandBuffer.copyBufferToImage(stagingBuffer->buffer, image->image, image->imageLayout, bufferImageCopy);
        transitionImageLayout(commandBuffer, image, oldLayout, vk::ImageAspectFlagBits::eNone, vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eAllCommands, subresourceRange);
    }

    void updateImage(const Image& image, const void* data, const vk::DeviceSize dataSize, const vk::ImageAspectFlags aspectFlags, std::optional<vk::ImageSubresourceLayers> subresourceLayers, std::optional<vk::ImageSubresourceRange> subresourceRange)
    {
        AVA_CHECK(image != nullptr && image->image, "Cannot update image when image is invalid");
        AVA_CHECK(dataSize <= image->allocationInfo.size, "Cannot update image when data size is larger than image's size (" + std::to_string(image->allocationInfo.size) + ")");
        AVA_CHECK(dataSize > 0, "Cannot update image when data size is 0");
        AVA_CHECK(data != nullptr, "Cannot update image when data is nullptr");

        if (!subresourceLayers.has_value())
        {
            subresourceLayers = vk::ImageSubresourceLayers{aspectFlags, 0, 0, image->creationInfo.arrayLayers};
        }

        vk::BufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageExtent = image->creationInfo.extent;
        region.imageOffset = vk::Offset3D{0, 0, 0};
        region.imageSubresource = subresourceLayers.value();

        auto stagingBuffer = createBuffer(dataSize, vk::BufferUsageFlagBits::eTransferSrc, MemoryLocation::eCpuToGpu, 0);
        updateBuffer(stagingBuffer, data, dataSize);

        const auto commandBuffer = beginSingleTimeCommands(vk::QueueFlagBits::eTransfer);
        updateImage(commandBuffer, image, stagingBuffer, region, subresourceRange);

        endSingleTimeCommands(commandBuffer);
        destroyBuffer(stagingBuffer);
    }

    ava::Image getSwapchainImage(const uint32_t index)
    {
        AVA_CHECK(index < detail::State.swapchainImageCount, "Cannot get swapchain image when index is out of range of image count");
        return detail::State.swapchainAvaImages.at(index);
    }

    ava::ImageView getSwapchainImageView(const uint32_t index)
    {
        AVA_CHECK(index < detail::State.swapchainImageCount, "Cannot get swapchain image when index is out of range of image count");
        return detail::State.swapchainAvaImageViews.at(index);
    }

    std::vector<ava::Image> getSwapchainImages()
    {
        return detail::State.swapchainAvaImages;
    }

    std::vector<ava::ImageView> getSwapchainImageViews()
    {
        return detail::State.swapchainAvaImageViews;
    }

    vk::ImageAspectFlags getImageAspectFlagsForFormat(const vk::Format format)
    {
        vk::ImageAspectFlags aspectFlags{};

        if (detail::vulkanFormatHasDepth(format))
        {
            aspectFlags |= vk::ImageAspectFlagBits::eDepth;
        }
        else
        {
            aspectFlags |= vk::ImageAspectFlagBits::eColor;
        }

        if (detail::vulkanFormatHasStencil(format))
        {
            aspectFlags |= vk::ImageAspectFlagBits::eStencil;
        }
        return aspectFlags;
    }
}
