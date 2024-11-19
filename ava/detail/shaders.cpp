#include "shaders.hpp"

#include <filesystem>

#include "detail.hpp"
#include "state.hpp"
#include "utility.hpp"

namespace ava::detail
{
    vk::ShaderModule createShaderModule(const std::vector<char>& spirv)
    {
        AVA_CHECK(State.device, "Cannot create shader module when State's device is invalid")

        vk::ShaderModuleCreateInfo createInfo{};
        createInfo.codeSize = static_cast<uint32_t>(spirv.size());
        createInfo.pCode = reinterpret_cast<const uint32_t*>(spirv.data());
        createInfo.flags = {};

        return State.device.createShaderModule(createInfo);
    }

    vk::ShaderModule loadShaderModule(const std::string& filePath, std::vector<char>& spirv)
    {
        AVA_CHECK(!filePath.empty(), "Shader Module file path cannot be empty");
        AVA_CHECK(std::filesystem::exists(filePath) && !std::filesystem::is_directory(filePath), "Shader Module file does not exist");

        spirv = detail::readFile(filePath);

        return createShaderModule(spirv);
    }
}
