#ifndef AVA_DETAIL_SAMPLER_HPP
#define AVA_DETAIL_SAMPLER_HPP

#include "./vulkan.hpp"

namespace ava::detail
{
    struct Sampler
    {
        vk::Sampler sampler;
        vk::SamplerCreateInfo createInfo;
    };
}

#endif
