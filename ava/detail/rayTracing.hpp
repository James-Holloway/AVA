#ifndef AVA_DETAIL_RAYTRACING_HPP
#define AVA_DETAIL_RAYTRACING_HPP

#include "./vulkan.hpp"
#include "../types.hpp"

namespace ava::detail
{
    struct AccelerationStructure
    {
        vk::AccelerationStructureKHR accelerationStructure;
        vk::AccelerationStructureTypeKHR accelerationStructureType;
        vk::DeviceAddress accelerationStructureAddress;
        ava::Buffer buffer; // Backing buffer for the acceleration structure
    };

    enum class BLASType
    {
        Triangles,
    };

    struct BLAS
    {
        BLASType type;
        AccelerationStructure* accelerationStructure;
        ava::Buffer meshBuffer; // Copied from intake mesh buffer

        vk::DeviceAddress vertexDeviceAddress; // Device address of the meshBuffer's vertices
        vk::DeviceAddress indexDeviceAddress; // Device address of the meshBuffer's indices

        uint32_t maxVertex; // vertexCount - 1
        uint32_t vertexStride;
        vk::IndexType indexType;
        uint32_t indexCount;
        uint32_t instanceCount; // indexCount / 3
        bool built;
    };

    struct BLASInstance
    {
        BLAS* blas;
        vk::TransformMatrixKHR transformMatrix;
        vk::GeometryInstanceFlagsKHR geometryInstanceFlags;
        int32_t instanceCustomIndex; // If negative, uses location in passed blasInstances to rebuildTLAS
        uint8_t mask; // Visibility mask
        uint32_t instanceShaderBindingTableRecordOffset; // Offset in calculating the hit shader binding table index
    };

    struct TLAS
    {
        AccelerationStructure* accelerationStructure;
        bool built;
        vk::AccelerationStructureBuildSizesInfoKHR lastBuildSizesInfo;
        uint64_t lastBuildHash;
        vk::BuildAccelerationStructureFlagsKHR lastBuildFlags;
    };

    AccelerationStructure* createAccelerationStructure(vk::AccelerationStructureTypeKHR type, const vk::AccelerationStructureBuildSizesInfoKHR& buildSizeInfo);
    void destroyAccelerationStructure(AccelerationStructure*& accelerationStructure);

    // Has to assume position is first element and a float3/vec3 (R32G32B32Sfloat)
    BLAS* createBottomLevelAccelerationStructure(ava::VIBO meshBuffer);
    BLAS* createBottomLevelAccelerationStructure(ava::VBO vbo, std::optional<ava::IBO> ibo);
    void destroyBottomLevelAccelerationStructure(BLAS*& blas);

    vk::AccelerationStructureGeometryKHR getBLASGeometry(const BLAS* blas, vk::GeometryFlagsKHR geometryFlags = vk::GeometryFlagBitsKHR::eOpaque);

    // Builds one acceleration structure with a single time command and a scratch buffer
    void rebuildBLAS(BLAS* blas, vk::BuildAccelerationStructureFlagsKHR buildFlags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace, vk::GeometryFlagsKHR geometryFlags = vk::GeometryFlagBitsKHR::eOpaque);

    BLASInstance* createBLASInstance(BLAS* blas);
    void destroyBLASInstance(BLASInstance*& blasInstance);

    TLAS* createTopLevelAccelerationStructure();
    void destroyTopLevelAccelerationStructure(TLAS*& tlas);

    // Builds one acceleration structure with a single time command and a scratch buffer
    void rebuildTLAS(TLAS* tlas, const std::vector<BLASInstance*>& blasInstances, vk::BuildAccelerationStructureFlagsKHR buildFlags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate, vk::GeometryFlagsKHR geometryFlags = {});
    // Returns false if function could not update the TLAS
    [[nodiscard]] bool updateTLAS(TLAS* tlas, const std::vector<BLASInstance*>& blasInstances, vk::BuildAccelerationStructureFlagsKHR buildFlags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate, vk::GeometryFlagsKHR geometryFlags = {});
}

#endif
