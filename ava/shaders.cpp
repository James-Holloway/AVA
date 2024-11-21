#include "shaders.hpp"

#include "detail/shaders.hpp"
#include "detail/detail.hpp"
#include "detail/state.hpp"

namespace ava
{
    Shader createShader(const std::string& shaderPath, const vk::ShaderStageFlagBits stage, const std::string& entry)
    {
        std::vector<char> spirv;
        const auto module = detail::loadShaderModule(shaderPath, spirv);
        const auto outShader = new detail::Shader();
        outShader->module = module;
        outShader->stage = stage;
        outShader->entry = entry;
        outShader->spriv = spirv;
        return outShader;
    }

    Shader createShader(const std::vector<char>& shaderSpirv, const vk::ShaderStageFlagBits stage, const std::string& entry)
    {
        AVA_CHECK(!shaderSpirv.empty(), "Cannot create shader from empty shader SPIR-V vector");

        const auto module = detail::createShaderModule(shaderSpirv);
        const auto outShader = new detail::Shader();
        outShader->module = module;
        outShader->stage = stage;
        outShader->entry = entry;
        outShader->spriv = shaderSpirv;
        return outShader;
    }

    void destroyShader(Shader& shader)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(shader != nullptr, "Cannot destroy an invalid shader");
        if (shader->module)
        {
            AVA_CHECK_NO_EXCEPT_RETURN(detail::State.device, "Cannot destroy a shader when State's device is invalid");
            detail::State.device.destroyShaderModule(shader->module);
        }

        delete shader;
        shader = nullptr;
    }
}
