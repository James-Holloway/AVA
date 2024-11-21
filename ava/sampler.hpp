#ifndef AVA_SAMPLER_HPP
#define AVA_SAMPLER_HPP

#include "detail/vulkan.hpp"
#include "types.hpp"

namespace ava
{
    [[nodiscard]] Sampler createSampler(vk::Filter minMagFilter = vk::Filter::eLinear, vk::SamplerMipmapMode mipFilter = vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode repeat = vk::SamplerAddressMode::eRepeat, float maxAnisotropy = 8.0f, std::optional<vk::CompareOp> compareOp = {});
    void destroySampler(Sampler& sampler);
    vk::Sampler getSampler(const Sampler& sampler);
}

#endif
