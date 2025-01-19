#ifndef AVA_DETAIL_DEBUGCALLBACK_HPP
#define AVA_DETAIL_DEBUGCALLBACK_HPP

#include "./vulkan.hpp"
#include <iostream>

namespace ava::detail
{
    inline VkBool32 debugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, const VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        const auto severity = vkb::to_string_message_severity(messageSeverity);
        const auto type = vkb::to_string_message_type(messageType);
        static volatile char breakpoint;
        std::string message = std::format("[{}: {}] {}", severity, type, pCallbackData->pMessage);

        if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            std::cout << message << std::endl;
            (void)breakpoint;
        }
        else if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) //NOLINTNEXTLINE(*-branch-clone)
        {
            std::cerr << message << std::endl;
            (void)breakpoint;
        }
        else
        {
            std::cerr << message << std::endl;
            (void)breakpoint;
        }

        return VK_FALSE;
    }
}

#endif
