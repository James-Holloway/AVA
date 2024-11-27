#include "rayTracing.hpp"
#include "detail/rayTracing.hpp"

#include "ava.hpp"
#include "detail/detail.hpp"
#include "detail/state.hpp"

namespace ava
{
    std::vector<const char*> rayTracingDeviceExtensions{
        vk::KHRRayTracingPipelineExtensionName,
        vk::KHRAccelerationStructureExtensionName,
        vk::KHRRayQueryExtensionName,
        vk::KHRDeferredHostOperationsExtensionName,
    };

    std::vector<const char*> rayTracingPre13DeviceExtensions{
        vk::KHRShaderNonSemanticInfoExtensionName,
    };

    std::vector<const char*> rayTracingPre12DeviceExtensions{
        vk::KHRBufferDeviceAddressExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRShaderFloatControlsExtensionName,
        vk::EXTDescriptorIndexingExtensionName,
    };

    bool queryRayTracingSupport(Version apiVersion)
    {
        if (apiVersion.major == 1 && apiVersion.minor < 1)
        {
            return false; // Requires Vulkan 1.1 at a minimum
        }

        vkb::Instance instance = detail::State.vkbInstance;

        // If instance does not exist then create one for this query
        if (!detail::State.stateConfigured)
        {
            vkb::InstanceBuilder instanceBuilder;
            instanceBuilder
                .set_engine_name("ava")
                .set_engine_version(AVAVersion.major, AVAVersion.minor, AVAVersion.patch)
                .require_api_version(apiVersion.major, apiVersion.minor, apiVersion.patch);

            auto ibRet = instanceBuilder.build();
            if (!ibRet.has_value())
            {
                return false;
            }
            instance = ibRet.value();
        }
        if (!instance)
        {
            return false;
        }

        vkb::PhysicalDeviceSelector physicalDeviceSelector{instance};
        physicalDeviceSelector.defer_surface_initialization();

        physicalDeviceSelector.add_required_extensions(rayTracingDeviceExtensions);

        if (apiVersion.major == 1 && apiVersion.minor < 2) // if version is less than Vulkan 1.2
        {
            physicalDeviceSelector.add_required_extensions(rayTracingPre12DeviceExtensions);
        }
        if (apiVersion.major == 1 && apiVersion.minor < 3) // if version is less than Vulkan 1.3
        {
            physicalDeviceSelector.add_required_extensions(rayTracingPre13DeviceExtensions);
        }

        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR pipelineFeatures{};
        pipelineFeatures.rayTracingPipeline = true;

        vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
        accelerationStructureFeatures.accelerationStructure = true;

        vk::PhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
        rayQueryFeatures.rayQuery = true;

        vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressFeatures{};
        bufferDeviceAddressFeatures.bufferDeviceAddress = true;

        physicalDeviceSelector.add_required_extension_features(pipelineFeatures);
        physicalDeviceSelector.add_required_extension_features(accelerationStructureFeatures);
        physicalDeviceSelector.add_required_extension_features(rayQueryFeatures);
        physicalDeviceSelector.add_required_extension_features(bufferDeviceAddressFeatures);

        auto pdsRet = physicalDeviceSelector.select(vkb::DeviceSelectionMode::only_fully_suitable);
        detail::State.rayTracingQueried = true;

        bool rayTracingSupported = pdsRet.has_value();

        // Destroy instance if it was created for this query
        if (!detail::State.stateConfigured)
        {
            vkb::destroy_instance(instance);
        }

        return rayTracingSupported;
    }

    vk::TransformMatrixKHR getTransformMatrix44(const float matrix[4][4])
    {
        return vk::TransformMatrixKHR{
            std::array<std::array<float, 4>, 3>{
                matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
                matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
                matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
            }
        };
    }

    vk::TransformMatrixKHR getTransformMatrix44(const float matrix[16])
    {
        return vk::TransformMatrixKHR{
            std::array<std::array<float, 4>, 3>{
                matrix[0], matrix[1], matrix[2], matrix[3],
                matrix[4], matrix[5], matrix[6], matrix[7],
                matrix[8], matrix[9], matrix[10], matrix[11],
            }
        };
    }

    vk::TransformMatrixKHR getTransformMatrix34(const float matrix[3][4])
    {
        return vk::TransformMatrixKHR{
            std::array<std::array<float, 4>, 3>{
                matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
                matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
                matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
            }
        };
    }

    vk::TransformMatrixKHR getTransformMatrix34(const float matrix[12])
    {
        return vk::TransformMatrixKHR{
            std::array<std::array<float, 4>, 3>{
                matrix[0], matrix[1], matrix[2], matrix[3],
                matrix[4], matrix[5], matrix[6], matrix[7],
                matrix[8], matrix[9], matrix[10], matrix[11],
            }
        };
    }

    BLAS createBottomLevelAccelerationStructure(const ava::VIBO meshBuffer)
    {
        return detail::createBottomLevelAccelerationStructure(meshBuffer);
    }

    BLAS createBottomLevelAccelerationStructure(const ava::VBO vbo, const std::optional<ava::IBO> ibo)
    {
        return detail::createBottomLevelAccelerationStructure(vbo, ibo);
    }

    void destroyBottomLevelAccelerationStructure(BLAS& blas)
    {
        detail::destroyBottomLevelAccelerationStructure(blas);
    }

    void rebuildBLAS(const BLAS blas, const vk::BuildAccelerationStructureFlagsKHR buildFlags, const vk::GeometryFlagsKHR geometryFlags)
    {
        return detail::rebuildBLAS(blas, buildFlags, geometryFlags);
    }

    BLASInstance createBLASInstance(const BLAS blas, const int32_t instanceCustomIndex, const uint8_t mask)
    {
        auto outBlasInstance = detail::createBLASInstance(blas);
        outBlasInstance->instanceCustomIndex = instanceCustomIndex;
        outBlasInstance->mask = mask;
        return outBlasInstance;
    }

    void destroyBLASInstance(BLASInstance& blasInstance)
    {
        detail::destroyBLASInstance(blasInstance);
    }

    void updateTransformMatrix(const BLASInstance& blasInstance, const vk::TransformMatrixKHR& transformMatrix)
    {
        AVA_CHECK(blasInstance != nullptr, "Cannot update transform matrix of an invalid BLAS instance");
        blasInstance->transformMatrix = transformMatrix;
    }

    TLAS createTopLevelAccelerationStructure()
    {
        return detail::createTopLevelAccelerationStructure();
    }

    void destroyTopLevelAccelerationStructure(TLAS& tlas)
    {
        detail::destroyTopLevelAccelerationStructure(tlas);
    }

    void rebuildTLAS(const TLAS tlas, const std::vector<BLASInstance>& blasInstances, vk::BuildAccelerationStructureFlagsKHR buildFlags, vk::GeometryFlagsKHR geometryFlags)
    {
        detail::rebuildTLAS(tlas, blasInstances, buildFlags, geometryFlags);
    }
}
