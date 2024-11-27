#include "buffer.hpp"

#include "detail.hpp"
#include "state.hpp"

namespace ava::detail
{
    vk::DeviceAddress getBufferDeviceAddress(const Buffer* buffer)
    {
        AVA_CHECK(State.shaderDeviceAddressEnabled, "Cannot get buffer device address when feature has not been enabled on creation");
        AVA_CHECK(buffer != nullptr && buffer->buffer, "Cannot get buffer device address from an invalid buffer");

        vk::BufferDeviceAddressInfo info{};
        info.buffer = buffer->buffer;
        return State.device.getBufferAddress(info, State.dispatchLoader);
    }
}
