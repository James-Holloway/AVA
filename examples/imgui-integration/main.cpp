#include <framework.hpp>
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include "ava/detail/commandBuffer.hpp"
#include "ava/detail/renderPass.hpp"

class ImGuiIntegration : public AvaFramework
{
public:
    ImGuiIntegration() : AvaFramework("AVA ImGui Integration")
    {
    }

    ~ImGuiIntegration() override = default;

    vk::DescriptorPool descriptorPool;
    ava::raii::RenderPass::Ptr renderPass;
    std::vector<ava::raii::Framebuffer::Ptr> framebuffers;

    void init() override
    {
        AvaFramework::resize();

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window, true);

        const ava::RenderPassCreationInfo renderPassInfo{
            {
                ava::createSimpleColorAttachmentInfo(surfaceFormat, true, true, 1)
            },
            1
        };
        renderPass = ava::raii::RenderPass::create(renderPassInfo);

        const auto vkDevice = ava::getVulkanDevice();

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.setMaxSets(512);
        std::vector poolSizes = {
            vk::DescriptorPoolSize{vk::DescriptorType::eSampler, 512},
            vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 512},
            vk::DescriptorPoolSize{vk::DescriptorType::eInputAttachment, 512},
            vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, 512},
            vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 512},
            vk::DescriptorPoolSize{vk::DescriptorType::eStorageImage, 512},
            vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 512},
            vk::DescriptorPoolSize{vk::DescriptorType::eUniformBufferDynamic, 512},
            vk::DescriptorPoolSize{vk::DescriptorType::eUniformTexelBuffer, 512},
            vk::DescriptorPoolSize{vk::DescriptorType::eStorageTexelBuffer, 512},
            vk::DescriptorPoolSize{vk::DescriptorType::eStorageBufferDynamic, 512},
        };
        descriptorPoolCreateInfo.setPoolSizes(poolSizes);
        descriptorPoolCreateInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

        descriptorPool = vkDevice.createDescriptorPool(descriptorPoolCreateInfo);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Device = vkDevice;
        initInfo.Instance = ava::getVulkanInstance();
        initInfo.Queue = ava::getVulkanQueue(vk::QueueFlagBits::eGraphics);
        initInfo.DescriptorPool = descriptorPool;
        initInfo.RenderPass = renderPass->renderPass->renderPass;
        initInfo.Subpass = 0;
        initInfo.ImageCount = ava::getSwapchainImageCount();
        initInfo.MinImageCount = ava::getSwapchainImageCount();
        initInfo.PhysicalDevice = ava::getVulkanPhysicalDevice();
        ImGui_ImplVulkan_Init(&initInfo);
    }

    void cleanup() override
    {
        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext();

        framebuffers.clear();
        renderPass.reset();
        ava::getVulkanDevice().destroyDescriptorPool(descriptorPool);
    }

    void update() override
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("Info"))
        {
            auto deviceProperties = ava::getVulkanPhysicalDevice().getProperties();
            ImGui::Text("AVA ImGui Integration");
            ImGui::Text("%s", deviceProperties.deviceName.data());
            ImGui::Text("Vulkan API %d.%d.%d", VK_API_VERSION_MAJOR(deviceProperties.apiVersion), VK_API_VERSION_MINOR(deviceProperties.apiVersion), VK_API_VERSION_PATCH(deviceProperties.apiVersion));
        }

        ImGui::End();

        ImGui::ShowDemoWindow();
    }

    void draw(const ava::raii::CommandBuffer::Ptr& commandBuffer, uint32_t currentFrame, uint32_t imageIndex) override
    {
        vk::ClearValue colorClearValue{{0.0f, 0.0f, 0.0f, 1.0f}};
        commandBuffer->beginRenderPass(renderPass, framebuffers.at(imageIndex), {colorClearValue});

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer->commandBuffer->commandBuffer, nullptr);

        commandBuffer->endRenderPass();
    }

    void resize() override
    {
        AvaFramework::resize();

        framebuffers = ava::raii::Framebuffer::createSwapchain(renderPass);
    }
};

int main()
{
    ImGuiIntegration imguiIntegration;
    imguiIntegration.run();
    return 0;
}
