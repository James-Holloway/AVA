#ifndef AVA_SHADERS_HPP
#define AVA_SHADERS_HPP

#include "types.hpp"

namespace ava
{
    [[nodiscard]] Shader createShader(const std::string& shaderPath, vk::ShaderStageFlagBits stage, const std::string& entry = "main");
    [[nodiscard]] Shader createShader(const std::vector<char>& shaderSpirv, vk::ShaderStageFlagBits stage, const std::string& entry = "main");
    [[nodiscard]] Shader createShader(const std::vector<uint8_t>& shaderSpirv, vk::ShaderStageFlagBits stage, const std::string& entry = "main");
    void destroyShader(Shader& shader);
}

#endif
