#ifndef AVA_DETAIL_VULKAN_HPP
#define AVA_DETAIL_VULKAN_HPP

#include <vulkan/vulkan.hpp>
#include <VkBootstrap.h>
#include <vma/vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>
#include <optional>

namespace ava::detail
{
    bool vulkanFormatHasDepth(vk::Format format);
    bool vulkanFormatHasStencil(vk::Format format);
    size_t vulkanFormatByteWidth(vk::Format format);
}

#endif
