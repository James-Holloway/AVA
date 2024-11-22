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
        if (shader != nullptr)
        {
            ava::destroyShader(shader);
        }
    }

    Shader::Shader(Shader&& other) noexcept
    {
        shader = other.shader;
        other.shader = nullptr;
    }

    Shader& Shader::operator=(Shader&& other) noexcept
    {
        if (this != &other)
        {
            shader = other.shader;
            other.shader = nullptr;
        }
        return *this;
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
