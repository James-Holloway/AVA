#include "memoryLocation.hpp"

namespace ava
{
    vma::MemoryUsage getMemoryUsageFromBufferLocation(const MemoryLocation bufferLocation)
    {
        switch (bufferLocation)
        {
        default:
        case MemoryLocation::eGpuOnly:
            return vma::MemoryUsage::eGpuOnly;
        case MemoryLocation::eCpuToGpu:
            return vma::MemoryUsage::eCpuToGpu;
        case MemoryLocation::eLazyGpu:
            return vma::MemoryUsage::eGpuLazilyAllocated;
        }
    }
}
