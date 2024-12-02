#ifndef AVA_DETAIL_UTILITY_HPP
#define AVA_DETAIL_UTILITY_HPP

#include <string>
#include <vector>
#include <cassert>

namespace ava::detail
{
    std::vector<char> readFile(const std::string& fileName);

    template <typename T>
    bool isPowerOf2(T x)
    {
        return (x & (x - 1)) == 0;
    }

    template <typename T>
    T alignUp(T value, T alignment)
    {
        assert(isPowerOf2(alignment));
        return (value + alignment - 1) & ~(alignment - 1);
    }

    template <typename T>
    T alignDown(T value, T alignment)
    {
        assert(isPowerOf2(alignment));
        return value & ~(alignment - 1);
    }
}


#endif
