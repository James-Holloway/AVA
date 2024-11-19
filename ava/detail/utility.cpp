#include "utility.hpp"

#include <stdexcept>
#include <fstream>

namespace ava::detail
{
    std::vector<char> readFile(const std::string& fileName)
    {
        std::ifstream file(fileName, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + fileName);
        }

        const auto fileSize = file.tellg();
        std::vector<char> buffer(static_cast<size_t>(fileSize));

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
}
