#ifndef AVA_RAYTRACING_HPP
#define AVA_RAYTRACING_HPP

#include "types.hpp"
#include "version.hpp"

namespace ava
{
    // Query ray tracing support before setting enableRayTracing in CreateInfo
    bool queryRayTracingSupport(Version apiVersion);

    vk::TransformMatrixKHR getTransformMatrix44(const float matrix[4][4]);
    vk::TransformMatrixKHR getTransformMatrix44(const float matrix[16]);
    vk::TransformMatrixKHR getTransformMatrix34(const float matrix[3][4]);
    vk::TransformMatrixKHR getTransformMatrix34(const float matrix[12]);

    // Has to assume position is first element and a float3/vec3 (R32G32B32Sfloat)
    BLAS createBottomLevelAccelerationStructure(ava::VIBO meshBuffer);
    BLAS createBottomLevelAccelerationStructure(ava::VBO vbo, std::optional<ava::IBO> ibo);
    void destroyBottomLevelAccelerationStructure(BLAS& blas);

    // Builds one acceleration structure with a single time command and a scratch buffer
    void rebuildBLAS(BLAS blas, vk::BuildAccelerationStructureFlagsKHR buildFlags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace, vk::GeometryFlagsKHR geometryFlags = vk::GeometryFlagBitsKHR::eOpaque);

    vk::DeviceAddress getVertexBufferAddress(BLAS blas);
    vk::DeviceAddress getIndexBufferAddress(BLAS blas);
    vk::IndexType getIndexBufferType(BLAS blas);

    BLASInstance createBLASInstance(BLAS blas, int32_t instanceCustomIndex = -1, uint8_t mask = 0xFF);
    void destroyBLASInstance(BLASInstance& blasInstance);

    void updateTransformMatrix(const BLASInstance& blasInstance, const vk::TransformMatrixKHR& transformMatrix);

    TLAS createTopLevelAccelerationStructure();
    void destroyTopLevelAccelerationStructure(TLAS& tlas);

    // Builds one acceleration structure with a single time command and a scratch buffer
    void rebuildTLAS(TLAS tlas, const std::vector<BLASInstance>& blasInstances, vk::BuildAccelerationStructureFlagsKHR buildFlags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate, vk::GeometryFlagsKHR geometryFlags = {});
    [[nodiscard]] bool updateTLAS(TLAS tlas, const std::vector<BLASInstance>& blasInstances, vk::BuildAccelerationStructureFlagsKHR buildFlags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate, vk::GeometryFlagsKHR geometryFlags = {});
}

#endif
