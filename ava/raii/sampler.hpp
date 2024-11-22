#ifndef AVA_RAII_SAMPLER_HPP
#define AVA_RAII_SAMPLER_HPP

#include "types.hpp"

namespace ava::raii
{
    class Sampler
    {
    public:
        explicit Sampler(const ava::Sampler& existingSampler);
        ~Sampler();

        ava::Sampler sampler;

        static Pointer<Sampler> create(vk::Filter minMagFilter = vk::Filter::eLinear, vk::SamplerMipmapMode mipFilter = vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode repeat = vk::SamplerAddressMode::eRepeat, float maxAnisotropy = 8.0f, const std::optional<vk::CompareOp>& compareOp = {});
    };
}

#endif
