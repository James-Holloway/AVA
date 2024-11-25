#ifndef AVA_DETAIL_DETAIL_HPP
#define AVA_DETAIL_DETAIL_HPP

#include <stdexcept>
#include <string>
#include <iostream>

#define AVA_CHECK(condition, errorMessage) \
{ \
if (!(condition)) \
{ \
throw std::runtime_error(errorMessage); \
} \
}

#define AVA_CHECK_NO_EXCEPT_RETURN(condition, errorMessage) \
{ \
if (!(condition))\
{ \
std::cerr << errorMessage << '\n'; \
return; \
} \
}

#ifndef AVA_WARN
#define AVA_WARN(message) \
    std::cout << "[AVA WARNING] "<< message << '\n';
#endif

namespace ava::detail
{
}

#endif
