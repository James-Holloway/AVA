#include "image.hpp"

#include "detail/image.hpp"
#include "detail/detail.hpp"
#include "detail/state.hpp"

namespace ava
{
    Image createImage(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usageFlags, vk::ImageType imageType, vk::ImageTiling tiling, uint32_t mipLevels, uint32_t arrayLayers, vk::SampleCountFlagBits samples, vma::MemoryUsage memoryUsage)
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
        allocInfo.usage = memoryUsage;

        const auto pair = detail::State.allocator.createImage(createInfo, allocInfo);

        auto newImage = new detail::Image();
        newImage->image = pair.first;
        newImage->allocation = pair.second;
        newImage->imageLayout = vk::ImageLayout::eUndefined;
        newImage->creationInfo = createInfo;

        return newImage;
    }

    Image createImage2D(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usageFlags, uint32_t mipLevels)
    {
        return createImage(vk::Extent3D{extent, 1}, format, usageFlags, vk::ImageType::e2D, vk::ImageTiling::eOptimal, mipLevels, 1, vk::SampleCountFlagBits::e1, vma::MemoryUsage::eGpuOnly);
    }

    vk::ImageView createImageView(const Image& image, vk::ImageAspectFlags aspectFlags, vk::ImageViewType imageViewType, std::optional<vk::ImageSubresourceRange> subresourceRange)
    {
        AVA_CHECK(image != nullptr, "Image cannot be null");
        if (!subresourceRange.has_value())
        {
            subresourceRange = vk::ImageSubresourceRange(aspectFlags, 0, image->creationInfo.mipLevels, 0, image->creationInfo.arrayLayers);
        }

        vk::ImageViewCreateInfo createInfo;
        createInfo.image = image->image;
        createInfo.viewType = imageViewType;
        createInfo.subresourceRange = subresourceRange.value();
        createInfo.setComponents(vk::ComponentMapping{});

        const auto imageView = detail::State.device.createImageView(createInfo);
        return imageView;
    }
}
