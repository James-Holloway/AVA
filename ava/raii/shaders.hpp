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

        Shader(const Shader& other) = delete;
        Shader& operator=(Shader& other) = delete;
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        // ReSharper disable once CppNonExplicitConversionOperator
        operator ava::Shader() const;

        static Pointer<Shader> create(const std::string& shaderPath, vk::ShaderStageFlagBits stage, const std::string& entry = "main");
        static Pointer<Shader> create(const std::vector<char>& shaderSpirv, vk::ShaderStageFlagBits stage, const std::string& entry = "main");
    };
}
#endif
