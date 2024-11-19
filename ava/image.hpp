#ifndef AVA_IMAGE_HPP
#define AVA_IMAGE_HPP

#include "detail/vulkan.hpp"

namespace ava
{
    namespace detail
    {
        struct Image;
    }

    using Image = detail::Image*;

    // Generic and more advanced image creation
    Image createImage(vk::Extent3D extent, vk::Format format, vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::ImageType imageType = vk::ImageType::e2D, vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
                      uint32_t mipLevels = 1, uint32_t arrayLayers = 1, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1, vma::MemoryUsage memoryUsage = vma::MemoryUsage::eGpuOnly);

    // Simpler image creation
    Image createImage2D(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, uint32_t mipLevels = 1);

    // Image view creation
    vk::ImageView createImageView(const Image& image, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor, vk::ImageViewType imageViewType = vk::ImageViewType::e2D, std::optional<vk::ImageSubresourceRange> subresourceRange = {});
}


#endif
