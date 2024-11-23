#ifndef AVA_BUFFERLOCATION_HPP
#define AVA_BUFFERLOCATION_HPP

#include "types.hpp"

namespace ava
{
    enum class MemoryLocation
    {
        eGpuOnly,
        eCpuToGpu,
        eLazyGpu, // Lazily allocated (useful for multi-sample attachments)
    };

    vma::MemoryUsage getMemoryUsageFromBufferLocation(MemoryLocation bufferLocation);
}

#endif
