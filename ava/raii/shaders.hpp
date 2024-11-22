#ifndef AVA_RAII_SHADERS_HPP
#define AVA_RAII_SHADERS_HPP

#include "types.hpp"

namespace ava::raii
{
    class Shader
    {
    public:
        explicit Shader(const ava::Shader& existingShader);
        ~Shader();

        ava::Shader shader;

        // ReSharper disable once CppNonExplicitConversionOperator
        operator ava::Shader() const;

        static Pointer<Shader> create(const std::string& shaderPath, vk::ShaderStageFlagBits stage, const std::string& entry = "main");
        static Pointer<Shader> create(const std::vector<char>& shaderSpirv, vk::ShaderStageFlagBits stage, const std::string& entry = "main");
    };
}
#endif
