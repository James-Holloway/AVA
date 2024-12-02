#ifndef AVA_RAII_RAYTRACING_HPP
#define AVA_RAII_RAYTRACING_HPP

#include "types.hpp"
#include "../types.hpp"

namespace ava::raii
{
    class BLAS
    {
    public:
        using Ptr = Pointer<BLAS>;

        explicit BLAS(const Pointer<VIBO>& meshBuffer);
        explicit BLAS(const Pointer<VBO>& vbo, const Pointer<IBO>& ibo = nullptr);
        ~BLAS();

        ava::BLAS blas;

        BLAS(const BLAS& other) = delete;
        BLAS& operator=(BLAS& other) = delete;
        BLAS(BLAS&& other) noexcept;
        BLAS& operator=(BLAS&& other) noexcept;

        void rebuild(vk::BuildAccelerationStructureFlagsKHR buildFlags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace, vk::GeometryFlagsKHR geometryFlags = vk::GeometryFlagBitsKHR::eOpaque) const;
        Pointer<BLASInstance> createInstance(int32_t instanceCustomIndex = -1, uint8_t mask = 0xFF) const;

        vk::DeviceAddress getVertexBufferAddress() const;
        vk::DeviceAddress getIndexBufferAddress() const;
        vk::IndexType getIndexBufferType() const;

        static Pointer<BLAS> create(const Pointer<VIBO>& meshBuffer);
        static Pointer<BLAS> create(const Pointer<VBO>& vbo, const Pointer<IBO>& ibo = nullptr);
    };

    class BLASInstance
    {
    public:
        using Ptr = Pointer<BLASInstance>;
        explicit BLASInstance(const Pointer<BLAS>& blas, int32_t instanceCustomIndex = -1, uint8_t mask = 0xFF);
        explicit BLASInstance(ava::BLASInstance existingBLASInstance);
        ~BLASInstance();

        ava::BLASInstance blasInstance;

        BLASInstance(const BLASInstance& other) = delete;
        BLASInstance& operator=(BLASInstance& other) = delete;
        BLASInstance(BLASInstance&& other) noexcept;
        BLASInstance& operator=(BLASInstance&& other) noexcept;

        void updateTransformMatrix(const vk::TransformMatrixKHR& transformMatrix) const;

        static Pointer<BLASInstance> create(const Pointer<BLAS>& blas, int32_t instanceCustomIndex = -1, uint8_t mask = 0xFF);
    };

    class TLAS
    {
    public:
        using Ptr = Pointer<TLAS>;
        TLAS();
        ~TLAS();

        ava::TLAS tlas;

        TLAS(const TLAS& other) = delete;
        TLAS& operator=(TLAS& other) = delete;
        TLAS(TLAS&& other) noexcept;
        TLAS& operator=(TLAS&& other) noexcept;

        void rebuild(const std::vector<Pointer<BLASInstance>>& blasInstances, vk::BuildAccelerationStructureFlagsKHR buildFlags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate, vk::GeometryFlagsKHR geometryFlags = {}) const;
        [[nodiscard]] bool update(const std::vector<Pointer<BLASInstance>>& blasInstances, vk::BuildAccelerationStructureFlagsKHR buildFlags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate, vk::GeometryFlagsKHR geometryFlags = {}) const;

        static Pointer<TLAS> create();
    };
}

#endif
