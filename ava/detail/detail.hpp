#ifndef AVA_DETAIL_DETAIL_HPP
#define AVA_DETAIL_DETAIL_HPP

#include <stdexcept>
#include <string>

#define AVA_CHECK(condition, errorMessage) \
{ \
if (!(condition)) \
{ \
throw std::runtime_error(errorMessage); \
} \
}

namespace ava::detail
{
}

#endif
