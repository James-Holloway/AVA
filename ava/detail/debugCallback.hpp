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

        if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            std::cout << "[" << severity << ": " << type << "] " << pCallbackData->pMessage << '\n';
            (void)breakpoint;
        }
        else
        {
            std::cerr << "[" << severity << ": " << type << "] " << pCallbackData->pMessage << '\n';
            (void)breakpoint;
        }

        return VK_FALSE;
    }
}

#endif
