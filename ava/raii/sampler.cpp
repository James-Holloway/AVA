#include "sampler.hpp"
#include "ava/sampler.hpp"
#include "ava/detail/sampler.hpp"

#include "ava/detail/detail.hpp"

namespace ava::raii
{
    Sampler::Sampler(const ava::Sampler& existingSampler)
    {
        AVA_CHECK(existingSampler != nullptr && existingSampler->sampler, "Cannot create a RAII sampler from an invalid sampler");

        sampler = existingSampler;
    }

    Sampler::~Sampler()
    {
        if (sampler != nullptr)
        {
            ava::destroySampler(sampler);
        }
    }

    Sampler::Sampler(Sampler&& other) noexcept
    {
        sampler = other.sampler;
        other.sampler = nullptr;
    }

    Sampler& Sampler::operator=(Sampler&& other) noexcept
    {
        if (this != &other)
        {
            sampler = other.sampler;
            other.sampler = nullptr;
        }
        return *this;
    }

    Pointer<Sampler> Sampler::create(const vk::Filter minMagFilter, const vk::SamplerMipmapMode mipFilter, const vk::SamplerAddressMode repeat, const float maxAnisotropy, const std::optional<vk::CompareOp>& compareOp)
    {
        return std::make_shared<Sampler>(ava::createSampler(minMagFilter, mipFilter, repeat, maxAnisotropy, compareOp));
    }
}
