#include "rayTracing.hpp"
#include "ava/rayTracing.hpp"

#include "buffer.hpp"
#include "commandBuffer.hpp"
#include "detail.hpp"
#include "state.hpp"
#include "vbo.hpp"
#include "ibo.hpp"
#include "vibo.hpp"
#include "ava/buffer.hpp"
#include "ava/commandBuffer.hpp"

namespace ava::detail
{
    AccelerationStructure* createAccelerationStructure(const vk::AccelerationStructureTypeKHR type, const vk::AccelerationStructureBuildSizesInfoKHR& buildSizeInfo)
    {
        AVA_CHECK(buildSizeInfo.accelerationStructureSize != 0, "Cannot create an acceleration structure when buildSizeInfo.accelerationStructureSize is 0");
        AVA_CHECK(State.rayTracingEnabled, "Cannot create an acceleration structure when ray tracing is not enabled");

        // eShaderDeviceAddress flag bits are automatically added when ray tracing or shader device address feature is enabled
        const auto buffer = ava::createBuffer(buildSizeInfo.accelerationStructureSize, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR, MemoryLocation::eGpuOnly, 0);
        vk::AccelerationStructureCreateInfoKHR createInfo{};
        createInfo.buffer = buffer->buffer;
        createInfo.size = buildSizeInfo.accelerationStructureSize;
        createInfo.type = type;
        createInfo.offset = 0;

        const auto accelerationStructure = State.device.createAccelerationStructureKHR(createInfo, nullptr, State.dispatchLoader);

        vk::AccelerationStructureDeviceAddressInfoKHR addressInfo{};
        addressInfo.accelerationStructure = accelerationStructure;
        const auto asAddress = State.device.getAccelerationStructureAddressKHR(addressInfo, State.dispatchLoader);

        const auto outAccelerationStructure = new AccelerationStructure();
        outAccelerationStructure->buffer = buffer;
        outAccelerationStructure->accelerationStructure = accelerationStructure;
        outAccelerationStructure->accelerationStructureAddress = asAddress;
        outAccelerationStructure->accelerationStructureType = type;
        return outAccelerationStructure;
    }

    void destroyAccelerationStructure(AccelerationStructure*& accelerationStructure)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(accelerationStructure != nullptr, "Cannot destroy an invalid acceleration structure");

        if (accelerationStructure->accelerationStructure)
        {
            State.device.destroyAccelerationStructureKHR(accelerationStructure->accelerationStructure, nullptr, State.dispatchLoader);
        }
        destroyBuffer(accelerationStructure->buffer);

        delete accelerationStructure;
        accelerationStructure = nullptr;
    }

    constexpr auto blasMeshBufferUsage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;

    BLAS* createBottomLevelAccelerationStructure(const ava::VIBO meshBuffer)
    {
        AVA_CHECK(meshBuffer != nullptr && meshBuffer->buffer != nullptr && meshBuffer->buffer->buffer, "Cannot create a BLAS from an invalid meshBuffer");
        AVA_CHECK((meshBuffer->buffer->bufferUsage & vk::BufferUsageFlagBits::eTransferSrc) != vk::BufferUsageFlags{}, "Cannot create a BLAS from a meshBuffer without transfer src buffer usage");
        AVA_CHECK(meshBuffer->topology == vk::PrimitiveTopology::eTriangleList, "Cannot create a BLAS from a meshBuffer which is not a triangle list");
        AVA_CHECK(State.rayTracingEnabled, "Cannot create a BLAS when ray tracing is not enabled");

        const auto newMeshBuffer = ava::createBuffer(meshBuffer->buffer->size, blasMeshBufferUsage, MemoryLocation::eGpuOnly, 0);

        vk::BufferCopy copyRegion{};
        copyRegion.size = meshBuffer->buffer->size;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;

        const auto commandBuffer = beginSingleTimeCommands(vk::QueueFlagBits::eTransfer);
        commandBuffer->commandBuffer.copyBuffer(meshBuffer->buffer->buffer, newMeshBuffer->buffer, copyRegion);
        endSingleTimeCommands(commandBuffer);

        const auto baseMeshBufferDeviceAddress = getBufferDeviceAddress(newMeshBuffer);
        const auto vertexMeshBufferDeviceAddress = baseMeshBufferDeviceAddress + meshBuffer->vertexOffset;
        const auto indexMeshBufferDeviceAddress = baseMeshBufferDeviceAddress + meshBuffer->indexOffset;

        const auto outBLAS = new BLAS();
        outBLAS->type = BLASType::Triangles;
        outBLAS->accelerationStructure = nullptr;
        outBLAS->meshBuffer = newMeshBuffer;
        outBLAS->maxVertex = meshBuffer->vertexCount - 1;
        outBLAS->vertexStride = meshBuffer->stride;
        outBLAS->indexType = meshBuffer->indexType;
        outBLAS->indexCount = meshBuffer->indexCount;
        outBLAS->vertexDeviceAddress = vertexMeshBufferDeviceAddress;
        outBLAS->indexDeviceAddress = indexMeshBufferDeviceAddress;
        outBLAS->instanceCount = outBLAS->indexCount / 3;
        outBLAS->built = false;
        return outBLAS;
    }

    BLAS* createBottomLevelAccelerationStructure(const ava::VBO vbo, const std::optional<ava::IBO> ibo)
    {
        AVA_CHECK(vbo != nullptr && vbo->buffer != nullptr && vbo->buffer->buffer, "Cannot create a BLAS from an invalid VBO");
        AVA_CHECK((vbo->buffer->bufferUsage & vk::BufferUsageFlagBits::eTransferSrc) != vk::BufferUsageFlags{}, "Cannot create a BLAS from a vbo without transfer src buffer usage");
        AVA_CHECK(vbo->topology == vk::PrimitiveTopology::eTriangleList, "Cannot create a BLAS from a vbo which is not a triangle list");
        AVA_CHECK(State.rayTracingEnabled, "Cannot create a BLAS when ray tracing is not enabled");

        ava::Buffer iboBuffer = nullptr;
        if (ibo.has_value() && ibo.value() != nullptr)
        {
            AVA_CHECK(ibo.value()->buffer != nullptr && ibo.value()->buffer->buffer, "Cannot create a BLAS from an invalid IBO");
            AVA_CHECK((ibo.value()->buffer->bufferUsage & vk::BufferUsageFlagBits::eTransferSrc) != vk::BufferUsageFlags{}, "Cannot create a BLAS from an ibo without transfer src buffer usage");
            iboBuffer = ibo.value()->buffer;
        }
        else // Create an IBO from the VBO's vertex count
        {
            std::vector<uint32_t> indices(vbo->vertexCount);
            std::ranges::generate(indices, [i = 0u]() mutable -> unsigned { return i++; });

            iboBuffer = ava::createBuffer(indices.size() * sizeof(uint32_t), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc, MemoryLocation::eCpuToGpu, 0);
            ava::updateBuffer(iboBuffer, indices, 0);
        }

        const auto vboSize = vbo->buffer->size;
        const auto iboSize = iboBuffer->size;
        const auto newSize = vboSize + iboSize;
        const auto newMeshBuffer = ava::createBuffer(newSize, blasMeshBufferUsage, MemoryLocation::eGpuOnly, 0);

        vk::BufferCopy vboCopyRegion{};
        vboCopyRegion.size = vboSize;
        vboCopyRegion.srcOffset = 0;
        vboCopyRegion.dstOffset = 0;

        vk::BufferCopy iboCopyRegion{};
        iboCopyRegion.size = iboSize;
        iboCopyRegion.srcOffset = 0;
        iboCopyRegion.dstOffset = vboSize;

        const auto commandBuffer = beginSingleTimeCommands(vk::QueueFlagBits::eTransfer);
        commandBuffer->commandBuffer.copyBuffer(vbo->buffer->buffer, newMeshBuffer->buffer, vboCopyRegion);
        commandBuffer->commandBuffer.copyBuffer(iboBuffer->buffer, newMeshBuffer->buffer, iboCopyRegion);
        endSingleTimeCommands(commandBuffer);

        // When the created ibo buffer has served its purpose, destroy it
        if (!(ibo.has_value() && ibo.value() != nullptr))
        {
            destroyBuffer(iboBuffer);
        }

        const auto baseMeshBufferDeviceAddress = getBufferDeviceAddress(newMeshBuffer);
        const auto vertexMeshBufferDeviceAddress = baseMeshBufferDeviceAddress + 0ul;
        const auto indexMeshBufferDeviceAddress = baseMeshBufferDeviceAddress + vboSize;

        const auto outBLAS = new BLAS();
        outBLAS->type = BLASType::Triangles;
        outBLAS->accelerationStructure = nullptr;
        outBLAS->meshBuffer = newMeshBuffer;
        outBLAS->maxVertex = vbo->vertexCount - 1;
        outBLAS->vertexStride = vbo->stride;
        outBLAS->indexType = vk::IndexType::eUint32;
        outBLAS->indexCount = vbo->vertexCount;
        outBLAS->vertexDeviceAddress = vertexMeshBufferDeviceAddress;
        outBLAS->indexDeviceAddress = indexMeshBufferDeviceAddress;
        outBLAS->instanceCount = outBLAS->indexCount / 3;
        outBLAS->built = false;
        return outBLAS;
    }

    void destroyBottomLevelAccelerationStructure(BLAS*& blas)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(blas != nullptr, "Cannot destroy an invalid BLAS");

        if (blas->accelerationStructure != nullptr)
        {
            destroyAccelerationStructure(blas->accelerationStructure);
        }

        if (blas->meshBuffer != nullptr)
        {
            destroyBuffer(blas->meshBuffer);
        }

        delete blas;
        blas = nullptr;
    }

    vk::AccelerationStructureGeometryKHR getBLASGeometry(const BLAS* blas, vk::GeometryFlagsKHR geometryFlags)
    {
        AVA_CHECK(blas != nullptr && blas->meshBuffer, "Cannot get BLAS geometry from an invalid BLAS");

        vk::AccelerationStructureGeometryKHR geometry{};
        geometry.flags = geometryFlags;
        switch (blas->type)
        {
        case BLASType::Triangles:
            {
                geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
                geometry.geometry.triangles.sType = vk::StructureType::eAccelerationStructureGeometryTrianglesDataKHR;
                geometry.geometry.triangles.vertexFormat = vk::Format::eR32G32B32Sfloat;
                geometry.geometry.triangles.vertexData = blas->vertexDeviceAddress;
                geometry.geometry.triangles.vertexStride = blas->vertexStride;
                geometry.geometry.triangles.maxVertex = blas->maxVertex;
                geometry.geometry.triangles.indexType = blas->indexType;
                geometry.geometry.triangles.indexData = blas->indexDeviceAddress;
                geometry.geometry.triangles.transformData.deviceAddress = 0;
                break;
            }
        default:
            throw std::runtime_error("Unhandled BLAS Type");
        }
        return geometry;
    }

    void rebuildBLAS(BLAS* blas, const vk::BuildAccelerationStructureFlagsKHR buildFlags, const vk::GeometryFlagsKHR geometryFlags)
    {
        AVA_CHECK(blas != nullptr && blas->meshBuffer, "Cannot rebuild BLAS when acceleration structure is invalid");
        AVA_CHECK(blas->instanceCount > 0, "Cannot rebuild BLAS when BLAS's instance count is 0");

        vk::AccelerationStructureGeometryKHR geometry = getBLASGeometry(blas, geometryFlags);

        if (blas->accelerationStructure != nullptr)
        {
            destroyAccelerationStructure(blas->accelerationStructure);
        }
        blas->built = false;

        vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
        buildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
        buildGeometryInfo.flags = buildFlags;
        buildGeometryInfo.setGeometries(geometry);

        const auto buildSizesInfo = State.device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eHost, buildGeometryInfo, blas->instanceCount, State.dispatchLoader);

        blas->accelerationStructure = createAccelerationStructure(vk::AccelerationStructureTypeKHR::eBottomLevel, buildSizesInfo);

        auto scratchBuffer = ava::createBuffer(buildSizesInfo.buildScratchSize, vk::BufferUsageFlagBits::eStorageBuffer, MemoryLocation::eGpuOnly, State.accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment);

        buildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
        buildGeometryInfo.flags = buildFlags;
        buildGeometryInfo.setGeometries(geometry);
        buildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
        buildGeometryInfo.dstAccelerationStructure = blas->accelerationStructure->accelerationStructure;
        buildGeometryInfo.scratchData.deviceAddress = ava::detail::getBufferDeviceAddress(scratchBuffer);

        vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
        buildRangeInfo.primitiveCount = blas->instanceCount;
        buildRangeInfo.primitiveOffset = 0;
        buildRangeInfo.firstVertex = 0;
        buildRangeInfo.transformOffset = 0;

        const std::vector buildRangeInfos{&buildRangeInfo};

        const auto commandBuffer = beginSingleTimeCommands(vk::QueueFlagBits::eGraphics);
        commandBuffer->commandBuffer.buildAccelerationStructuresKHR(buildGeometryInfo, buildRangeInfos, State.dispatchLoader);
        endSingleTimeCommands(commandBuffer);

        destroyBuffer(scratchBuffer);

        blas->built = true;
    }

    BLASInstance* createBLASInstance(BLAS* blas)
    {
        AVA_CHECK(blas != nullptr, "Cannot create BLAS instance from an invalid BLAS");

        constexpr float defaultTransformMatrix[12] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
        };

        const auto outBLASInstance = new BLASInstance();
        outBLASInstance->blas = blas;
        outBLASInstance->transformMatrix = getTransformMatrix34(defaultTransformMatrix);
        outBLASInstance->geometryInstanceFlags = {};
        outBLASInstance->instanceCustomIndex = -1;
        outBLASInstance->mask = 0xFF;
        outBLASInstance->instanceShaderBindingTableRecordOffset = 0;
        return outBLASInstance;
    }

    void destroyBLASInstance(BLASInstance*& blasInstance)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(blasInstance != nullptr, "Cannot destroy an invalid BLAS instance");

        delete blasInstance;
        blasInstance = nullptr;
    }

    TLAS* createTopLevelAccelerationStructure()
    {
        AVA_CHECK(State.rayTracingEnabled, "Cannot create a TLAS when ray tracing is not enabled");

        const auto outTLAS = new TLAS();
        outTLAS->accelerationStructure = nullptr;
        outTLAS->built = false;
        return outTLAS;
    }

    void destroyTopLevelAccelerationStructure(TLAS*& tlas)
    {
        AVA_CHECK_NO_EXCEPT_RETURN(tlas != nullptr, "Cannot destroy an invalid TLAS");

        if (tlas->accelerationStructure != nullptr)
        {
            destroyAccelerationStructure(tlas->accelerationStructure);
        }

        delete tlas;
        tlas = nullptr;
    }

    void rebuildTLAS(TLAS* tlas, const std::vector<BLASInstance*>& blasInstances, vk::BuildAccelerationStructureFlagsKHR buildFlags, vk::GeometryFlagsKHR geometryFlags)
    {
        AVA_CHECK(tlas != nullptr, "Cannot rebuild TLAS when TLAS is invalid");

        std::vector<VkAccelerationStructureInstanceKHR> instances;
        instances.reserve(blasInstances.size());
        for (size_t i = 0; i < blasInstances.size(); i++)
        {
            const auto blasInstance = blasInstances[i];
            if (blasInstance == nullptr || blasInstance->blas == nullptr) continue;
            if (!blasInstance->blas->built)
            {
                AVA_WARN("Attempted to build TLAS when BLAS instance's BLAS is not built");
                continue;
            }

            VkAccelerationStructureInstanceKHR instance{};
            instance.accelerationStructureReference = blasInstance->blas->accelerationStructure->accelerationStructureAddress;
            instance.flags = static_cast<VkGeometryInstanceFlagsKHR>(blasInstance->geometryInstanceFlags);
            instance.instanceCustomIndex = (blasInstance->instanceCustomIndex < 0) ? i : blasInstance->instanceCustomIndex; // if negative use i rather than explicit value
            instance.mask = blasInstance->mask;
            instance.transform = blasInstance->transformMatrix;
            instance.instanceShaderBindingTableRecordOffset = blasInstance->instanceShaderBindingTableRecordOffset;

            instances.emplace_back(instance);
        }

        const auto instancesBufferSize = sizeof(VkAccelerationStructureInstanceKHR) * instances.size();
        auto instancesBuffer = ava::createBuffer(instancesBufferSize, vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR, MemoryLocation::eCpuToGpu, 0);
        updateBuffer(instancesBuffer, instances);

        vk::DeviceOrHostAddressConstKHR instanceDataDeviceAddress;
        instanceDataDeviceAddress.deviceAddress = getBufferDeviceAddress(instancesBuffer);

        vk::AccelerationStructureGeometryKHR geometry{};
        geometry.flags = geometryFlags;
        geometry.geometryType = vk::GeometryTypeKHR::eInstances;
        geometry.geometry.instances.sType = vk::StructureType::eAccelerationStructureGeometryInstancesDataKHR;
        geometry.geometry.instances.arrayOfPointers = false;
        geometry.geometry.instances.data = instanceDataDeviceAddress;

        vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
        buildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
        buildGeometryInfo.flags = buildFlags;
        buildGeometryInfo.setGeometries(geometry);

        const auto buildSizesInfo = State.device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, buildGeometryInfo, instances.size(), State.dispatchLoader);

        tlas->built = false;
        if (tlas->accelerationStructure != nullptr)
        {
            destroyAccelerationStructure(tlas->accelerationStructure);
        }

        tlas->accelerationStructure = createAccelerationStructure(vk::AccelerationStructureTypeKHR::eTopLevel, buildSizesInfo);

        auto scratchBuffer = ava::createBuffer(buildSizesInfo.buildScratchSize, vk::BufferUsageFlagBits::eStorageBuffer, MemoryLocation::eGpuOnly, State.accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment);

        buildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
        buildGeometryInfo.flags = buildFlags;
        buildGeometryInfo.setGeometries(geometry);
        buildGeometryInfo.dstAccelerationStructure = tlas->accelerationStructure->accelerationStructure;
        buildGeometryInfo.scratchData.deviceAddress = getBufferDeviceAddress(scratchBuffer);

        vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
        buildRangeInfo.primitiveCount = instances.size();
        buildRangeInfo.primitiveOffset = 0;
        buildRangeInfo.firstVertex = 0;
        buildRangeInfo.transformOffset = 0;

        const std::vector buildRangeInfos = {&buildRangeInfo};

        auto commandBuffer = beginSingleTimeCommands(vk::QueueFlagBits::eGraphics);
        commandBuffer->commandBuffer.buildAccelerationStructuresKHR(buildGeometryInfo, buildRangeInfos, State.dispatchLoader);
        endSingleTimeCommands(commandBuffer);

        ava::destroyBuffer(scratchBuffer);
        ava::destroyBuffer(instancesBuffer);

        tlas->built = true;
    }
}
