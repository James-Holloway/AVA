#include "rayTracing.hpp"

#include "ava/rayTracing.hpp"
#include "ava/detail/rayTracing.hpp"

#include "ibo.hpp"
#include "vbo.hpp"
#include "vibo.hpp"
#include "ava/detail/detail.hpp"

namespace ava::raii
{
    BLAS::BLAS(const Pointer<VIBO>& meshBuffer)
    {
        AVA_CHECK(meshBuffer != nullptr && meshBuffer->vibo != nullptr, "Cannot create a BLAS from an invalid meshBuffer");
        blas = ava::createBottomLevelAccelerationStructure(meshBuffer->vibo);
    }

    BLAS::BLAS(const Pointer<VBO>& vbo, const Pointer<IBO>& ibo)
    {
        AVA_CHECK(vbo != nullptr && vbo->vbo != nullptr, "Cannot create a BLAS from an invalid VBO");
        if (ibo != nullptr)
        {
            AVA_CHECK(ibo->ibo, "Cannot create a BLAS from an invalid IBO");
            blas = ava::createBottomLevelAccelerationStructure(vbo->vbo, ibo->ibo);
            return;
        }

        blas = ava::createBottomLevelAccelerationStructure(vbo->vbo, {});
    }

    BLAS::~BLAS()
    {
        if (blas != nullptr)
        {
            ava::destroyBottomLevelAccelerationStructure(blas);
        }
    }

    BLAS::BLAS(BLAS&& other) noexcept
    {
        blas = other.blas;
        other.blas = nullptr;
    }

    BLAS& BLAS::operator=(BLAS&& other) noexcept
    {
        if (this != &other)
        {
            blas = other.blas;
            other.blas = nullptr;
        }
        return *this;
    }

    void BLAS::rebuild(const vk::BuildAccelerationStructureFlagsKHR buildFlags, const vk::GeometryFlagsKHR geometryFlags) const
    {
        ava::rebuildBLAS(blas, buildFlags, geometryFlags);
    }

    Pointer<BLASInstance> BLAS::createInstance(const int32_t instanceCustomIndex, const uint8_t mask) const
    {
        return std::make_shared<BLASInstance>(ava::createBLASInstance(blas, instanceCustomIndex, mask));
    }

    vk::DeviceAddress BLAS::getVertexBufferAddress() const
    {
        return ava::getVertexBufferAddress(blas);
    }

    vk::DeviceAddress BLAS::getIndexBufferAddress() const
    {
        return ava::getIndexBufferAddress(blas);
    }

    vk::IndexType BLAS::getIndexBufferType() const
    {
        return ava::getIndexBufferType(blas);
    }

    Pointer<BLAS> BLAS::create(const Pointer<VIBO>& meshBuffer)
    {
        return std::make_shared<BLAS>(meshBuffer);
    }

    Pointer<BLAS> BLAS::create(const Pointer<VBO>& vbo, const Pointer<IBO>& ibo)
    {
        return std::make_shared<BLAS>(vbo, ibo);
    }

    BLASInstance::BLASInstance(const Pointer<BLAS>& blas, const int32_t instanceCustomIndex, const uint8_t mask)
    {
        AVA_CHECK(blas != nullptr, "Cannot create a RAII BLAS instance from an invalid BLAS");
        blasInstance = ava::createBLASInstance(blas->blas, instanceCustomIndex, mask);
    }

    BLASInstance::BLASInstance(const ava::BLASInstance existingBLASInstance)
    {
        AVA_CHECK(existingBLASInstance != nullptr && existingBLASInstance->blas != nullptr, "Cannot create a RAII BLAS instance from an invalid BLAS instance");
        blasInstance = existingBLASInstance;
    }

    BLASInstance::~BLASInstance()
    {
        if (blasInstance != nullptr)
        {
            ava::destroyBLASInstance(blasInstance);
        }
    }

    BLASInstance::BLASInstance(BLASInstance&& other) noexcept
    {
        blasInstance = other.blasInstance;
        other.blasInstance = nullptr;
    }

    BLASInstance& BLASInstance::operator=(BLASInstance&& other) noexcept
    {
        if (this != &other)
        {
            blasInstance = other.blasInstance;
            other.blasInstance = nullptr;
        }
        return *this;
    }

    void BLASInstance::updateTransformMatrix(const vk::TransformMatrixKHR& transformMatrix) const
    {
        ava::updateTransformMatrix(blasInstance, transformMatrix);
    }

    Pointer<BLASInstance> BLASInstance::create(const Pointer<BLAS>& blas, const int32_t instanceCustomIndex, const uint8_t mask)
    {
        return std::make_shared<BLASInstance>(blas, instanceCustomIndex, mask);
    }

    TLAS::TLAS()
    {
        tlas = ava::createTopLevelAccelerationStructure();
    }

    TLAS::~TLAS()
    {
        if (tlas != nullptr)
        {
            ava::destroyTopLevelAccelerationStructure(tlas);
        }
    }

    TLAS::TLAS(TLAS&& other) noexcept
    {
        tlas = other.tlas;
        other.tlas = nullptr;
    }

    TLAS& TLAS::operator=(TLAS&& other) noexcept
    {
        if (this != &other)
        {
            tlas = other.tlas;
            other.tlas = nullptr;
        }
        return *this;
    }

    void TLAS::rebuild(const std::vector<Pointer<BLASInstance>>& blasInstances, vk::BuildAccelerationStructureFlagsKHR buildFlags, vk::GeometryFlagsKHR geometryFlags) const
    {
        auto avaBlasInstances = std::vector<ava::BLASInstance>();
        avaBlasInstances.reserve(blasInstances.size());
        for (const auto& blasInstance : blasInstances)
        {
            if (blasInstance != nullptr && blasInstance->blasInstance != nullptr)
            {
                avaBlasInstances.push_back(blasInstance->blasInstance);
            }
        }

        ava::rebuildTLAS(tlas, avaBlasInstances, buildFlags, geometryFlags);
    }

    bool TLAS::update(const std::vector<Pointer<BLASInstance>>& blasInstances, vk::BuildAccelerationStructureFlagsKHR buildFlags, vk::GeometryFlagsKHR geometryFlags) const
    {
        auto avaBlasInstances = std::vector<ava::BLASInstance>();
        avaBlasInstances.reserve(blasInstances.size());
        for (const auto& blasInstance : blasInstances)
        {
            if (blasInstance != nullptr && blasInstance->blasInstance != nullptr)
            {
                avaBlasInstances.push_back(blasInstance->blasInstance);
            }
        }

        return ava::updateTLAS(tlas, avaBlasInstances, buildFlags, geometryFlags);
    }

    Pointer<TLAS> TLAS::create()
    {
        return std::make_shared<TLAS>();
    }
}
