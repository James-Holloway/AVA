#ifndef AVA_RAII_IBO_HPP
#define AVA_RAII_IBO_HPP

#include "types.hpp"

namespace ava::raii
{
    class IBO
    {
    public:
        explicit IBO(const ava::IBO& existingIBO);
        ~IBO();

        ava::IBO ibo;

        void bind(const Pointer<CommandBuffer>& commandBuffer) const;

        static Pointer<IBO> create(const uint32_t* indices, uint32_t indexCount);
        static Pointer<IBO> create(const uint16_t* indices, uint32_t indexCount);
        static Pointer<IBO> create(std::span<const uint32_t> indices);
        static Pointer<IBO> create(std::span<const uint16_t> indices);
        static Pointer<IBO> create(const std::vector<uint32_t>& indices);
        static Pointer<IBO> create(const std::vector<uint16_t>& indices);
    };
} // ava

#endif
