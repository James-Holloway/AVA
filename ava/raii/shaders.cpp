#include "shaders.hpp"
#include "ava/shaders.hpp"
#include "ava/detail/shaders.hpp"

#include "ava/detail/detail.hpp"

namespace ava::raii
{
    Shader::Shader(const ava::Shader& existingShader)
    {
        AVA_CHECK(existingShader != nullptr && existingShader->module, "Cannot create RAII shader from invalid shader");
        shader = existingShader;
    }

    Shader::~Shader()
    {
        ava::destroyShader(shader);
    }

    Shader::operator ava::Shader() const
    {
        return shader;
    }

    Pointer<Shader> Shader::create(const std::string& shaderPath, const vk::ShaderStageFlagBits stage, const std::string& entry)
    {
        return std::make_shared<Shader>(ava::createShader(shaderPath, stage, entry));
    }

    Pointer<Shader> Shader::create(const std::vector<char>& shaderSpirv, const vk::ShaderStageFlagBits stage, const std::string& entry)
    {
        return std::make_shared<Shader>(ava::createShader(shaderSpirv, stage, entry));
    }
}
