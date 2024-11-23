#ifndef AVA_RAII_SAMPLER_HPP
#define AVA_RAII_SAMPLER_HPP

#include "types.hpp"

namespace ava::raii
{
    class Sampler
    {
    public:
        using Ptr = Pointer<Sampler>;

        explicit Sampler(const ava::Sampler& existingSampler);
        ~Sampler();

        ava::Sampler sampler;

        Sampler(const Sampler& other) = delete;
        Sampler& operator=(Sampler& other) = delete;
        Sampler(Sampler&& other) noexcept;
        Sampler& operator=(Sampler&& other) noexcept;

        static Pointer<Sampler> create(vk::Filter minMagFilter = vk::Filter::eLinear, vk::SamplerMipmapMode mipFilter = vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode repeat = vk::SamplerAddressMode::eRepeat, float maxAnisotropy = 8.0f, const std::optional<vk::CompareOp>& compareOp = {});
    };
}

#endif
