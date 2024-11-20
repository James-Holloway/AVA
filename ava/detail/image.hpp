#ifndef AVA_DETAIL_IMAGE_HPP
#define AVA_DETAIL_IMAGE_HPP

#include "./vulkan.hpp"

namespace ava::detail
{
    struct Image
    {
        vk::Image image;
        vk::ImageLayout imageLayout;
        vma::Allocation allocation;
        vma::AllocationInfo allocationInfo;
        vk::ImageCreateInfo creationInfo;

        bool isSwapchainImage = false;
    };

    struct ImageView
    {
        vk::ImageView imageView;
        vk::Format format;
    };
}

#endif
