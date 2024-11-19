#ifndef AVA_DETAIL_VULKAN_HPP
#define AVA_DETAIL_VULKAN_HPP

#include <vulkan/vulkan.hpp>
#include <VkBootstrap.h>
#include <vk_mem_alloc.hpp>

namespace ava::detail
{
    bool vulkanFormatHasStencil(vk::Format format);
    size_t vulkanFormatByteWidth(vk::Format format);
}

#endif
