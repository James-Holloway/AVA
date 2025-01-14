#include "sampler.hpp"
#include "detail/sampler.hpp"

#include "detail/detail.hpp"
#include "detail/state.hpp"

ava::Sampler ava::createSampler(vk::Filter minMagFilter, vk::SamplerMipmapMode mipFilter, vk::SamplerAddressMode repeat, float maxAnisotropy, std::optional<vk::CompareOp> compareOp)
{
    AVA_CHECK(detail::State.device, "Cannot create sampler when State's device is invalid");

    float maxHardwareAnisotropy = detail::State.vkbPhysicalDevice.properties.limits.maxSamplerAnisotropy;
    if (maxHardwareAnisotropy < maxAnisotropy)
    {
        maxAnisotropy = maxHardwareAnisotropy;
    }
    if (!detail::State.vkbPhysicalDevice.features.samplerAnisotropy)
    {
        maxAnisotropy = 0;
    }

    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.minFilter = samplerInfo.magFilter = minMagFilter;
    samplerInfo.mipmapMode = mipFilter;
    samplerInfo.addressModeU = samplerInfo.addressModeV = samplerInfo.addressModeW = repeat;
    samplerInfo.maxAnisotropy = maxAnisotropy;
    samplerInfo.anisotropyEnable = maxAnisotropy > 0.0f;
    samplerInfo.compareEnable = compareOp.has_value();
    samplerInfo.compareOp = compareOp.value_or(vk::CompareOp::eAlways);
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = vk::LodClampNone;
    samplerInfo.unnormalizedCoordinates = false;

    const auto sampler = detail::State.device.createSampler(samplerInfo);

    const auto outSampler = new detail::Sampler();
    outSampler->sampler = sampler;
    outSampler->createInfo = samplerInfo;
    return outSampler;
}

void ava::destroySampler(Sampler& sampler)
{
    AVA_CHECK_NO_EXCEPT_RETURN(sampler != nullptr, "Cannot destroy invalid sampler");
    AVA_CHECK_NO_EXCEPT_RETURN(detail::State.device, "Cannot destroy sampler when State's device is invalid");

    if (sampler->sampler != nullptr)
    {
        detail::State.device.destroySampler(sampler->sampler);
    }

    delete sampler;
    sampler = nullptr;
}
