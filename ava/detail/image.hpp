#ifndef AVA_DETAIL_IMAGE_HPP
#define AVA_DETAIL_IMAGE_HPP

#include "./vulkan.hpp"

namespace ava::detail
{
    struct Image
    {
        vk::Image image;
        vk::ImageLayout imageLayout;
        std::vector<vk::ImageView> imageViews;
        vma::Allocation allocation;
        vk::ImageCreateInfo creationInfo;

        bool isSwapchainImage = false;
    };

    struct ImageView
    {
        vk::ImageView imageView;
    };
}

#endif
