#include "frame.hpp"

#include "creation.hpp"
#include "detail/commandBuffer.hpp"
#include "detail/detail.hpp"
#include "detail/state.hpp"

namespace ava
{
    using namespace detail;

    std::optional<ava::CommandBuffer> startFrame(uint32_t* currentFrame, uint32_t* imageIndex)
    {
        if (currentFrame != nullptr)
        {
            *currentFrame = ~0u;
        }
        if (imageIndex != nullptr)
        {
            *imageIndex = ~0u;
        }

        AVA_CHECK(!State.frameStarted, "Frame already started, present the previous one before starting a new frame");
        AVA_CHECK(State.device, "Device not initialized");
        AVA_CHECK(State.swapchain, "Swapchain not initialized");
        AVA_CHECK(State.inFlightGraphicsFences.size() >= State.currentFrame, "In flight graphics fence was not properly initialized");
        AVA_CHECK(State.imageAvailableSemaphores.size() >= State.currentFrame, "Image available semaphores was not properly initialized");
        AVA_CHECK(State.renderFinishedSemaphores.size() >= State.currentFrame, "Render finished semaphores was not properly initialized");
        vk::detail::resultCheck(State.device.waitForFences(State.inFlightGraphicsFences[State.currentFrame], true, std::numeric_limits<uint64_t>::max()), "Failed while waiting for previous frame fence");

        if (currentFrame != nullptr)
        {
            *currentFrame = State.currentFrame;
        }
        if (imageIndex != nullptr)
        {
            *imageIndex = State.imageIndex;
        }

        auto nextImageResult = State.device.acquireNextImageKHR(State.swapchain, std::numeric_limits<uint64_t>::max(), State.imageAvailableSemaphores[State.currentFrame], nullptr, &State.imageIndex);
        if (nextImageResult == vk::Result::eErrorOutOfDateKHR)
        {
            State.resizeNeeded = true;
            State.frameStarted = false;
            return {};
        }
        if (nextImageResult != vk::Result::eSuccess && nextImageResult != vk::Result::eSuboptimalKHR)
        {
            throw std::runtime_error("Failed to acquire swapchain image");
        }

        // Reset frame fence
        State.device.resetFences(State.inFlightGraphicsFences[State.currentFrame]);

        // Record command buffers
        auto commandBuffer = State.frameGraphicsCommandBuffers[State.currentFrame];
        // Clear any previously tracked objects
        commandBuffer->trackedObjects.clear();

        State.frameStarted = true;

        return {commandBuffer};
    }

    void presentFrame()
    {
        AVA_CHECK(State.frameStarted, "Frame not started, start one before presenting the frame");
        AVA_CHECK(State.swapchain, "Swapchain not initialized");
        AVA_CHECK(State.device, "Device not initialized");
        AVA_CHECK(State.graphicsQueue, "Graphics queue not initialized");

        State.frameStarted = false;

        // Submit command buffers
        auto waitSemaphores = {State.imageAvailableSemaphores[State.currentFrame]};
        auto signalSemaphores = {State.renderFinishedSemaphores[State.currentFrame]};
        auto commandBuffer = State.frameGraphicsCommandBuffers[State.currentFrame];
        vk::SubmitInfo submitInfo;
        constexpr vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.setWaitSemaphores(waitSemaphores);
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.setSignalSemaphores(signalSemaphores);
        submitInfo.setCommandBuffers(commandBuffer->commandBuffer);

        // Submit to the graphics queue
        State.graphicsQueue.submit(submitInfo, State.inFlightGraphicsFences[State.currentFrame]);

        // Present
        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphores(signalSemaphores);
        presentInfo.setSwapchains(State.swapchain);
        presentInfo.setImageIndices(State.imageIndex);

        const vk::Result presentResult = State.presentQueue.presentKHR(&presentInfo);
        if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
        {
            State.resizeNeeded = true;
        }
        else if (presentResult != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to present frame!");
        }

        // Change current frame
        State.currentFrame = (State.currentFrame + 1) % State.framesInFlight;
    }

    bool resizeNeeded()
    {
        return State.resizeNeeded;
    }
}
