# AVA - Another Vulkan Abstraction - Style Guide

## File Names

* Sources: `lowerCamelCase.cpp`
* Headers: `lowerCamelCase.hpp`

## Style Guide

| Type                        | Style          |
|-----------------------------|----------------| 
| Classes                     | UpperCamelCase |
| Structs                     | UpperCamelCase |
| Concepts                    | UpperCamelCase |
| Enums                       | UpperCamelCase |
| Named Unions                | UpperCamelCase |
| Template Parameters         | UpperCamelCase |
| Parameters                  | lowerCamelCase |
| Local Variables             | lowerCamelCase |
| Global Variables            | UpperCamelCase |
| Global Functions            | lowerCamelCase |
| Class/Struct Methods        | lowerCamelCase |
| Class/Struct Private Fields | lowerCamelCase |
| Class/Struct Public Fields  | UpperCamelCase |
| Namespaces                  | lowerCamelCase |
| Typedefs/Usings             | UpperCamelCase |
| Constexpr consts            | ALL_UPPER      | 
| Macros                      | ALL_UPPER      |

## Header Guards

Main directory:

```c++
#ifndef AVA_[NAME]_HPP
#define AVA_[NAME]_HPP

#endif
```

Detail directory:

```c++
#ifndef AVA_DETAIL_[NAME]_HPP
#define AVA_DETAIL_[NAME]_HPP

#endif
```
