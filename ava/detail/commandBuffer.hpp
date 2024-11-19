#ifndef AVA_DETAIL_COMMANDBUFFER_HPP
#define AVA_DETAIL_COMMANDBUFFER_HPP

#include "./vulkan.hpp"

namespace ava::detail
{
    std::vector<vk::CommandBuffer> createGraphicsCommandBuffers(uint32_t count, bool secondary = false);
    std::vector<vk::CommandBuffer> createComputeCommandBuffers(uint32_t count, bool secondary = false);
}

#endif
