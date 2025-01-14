# AVA - Another Vulkan Abstraction

Aim: To be an abstraction of Vulkan that is simpler than dealing with `vulkan.h`/`vulkan.hpp` normally, somewhere between OpenGL and Vulkan

You will still need to provide a windowing system, input and main game loop (unless using the example framework)

## Features

* Abstraction over Vulkan features without sacrificing control
* CreateInfo configuration to simplify creation of the initial Vulkan state
* [Slang](https://shader-slang.com/) shader compilation support. All examples are written in Slang, though anything that compiles to SPIR-V can be used
* Supporting latest Vulkan features such as Acceleration Structures and Ray Tracing
* Installation using CMake so AVA can be used in other projects system-wide
* Both unmanaged and RAII objects
* Textures and images
* Samplers
* Vertex buffer objects, index buffer objects and combined VIBO
* Vertex attribute objects to bridge knowledge of OpenGL to Vulkan
* Framebuffers
* Render passes
* Geometry and tesselation shaders
* Compute shaders
* Shader reflection using [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) to get layout bindings and push constants at runtime

## Examples

| Example              | Screenshot                                                                                                                             |
|----------------------|----------------------------------------------------------------------------------------------------------------------------------------|
| Tessellation         | ![Tesselation screenshot](screenshots/tessellation.png) <br/> Showing support for tessellation and geometry shaders using PN triangles |
| Cube                 | ![Cube Screenshot](screenshots/cube.png) <br/> Simple rotating cube                                                                    |
| Multisampled Monkeys | ![Multisampled Monkeys](screenshots/multisampled-monkeys.png) <br/> Multisampling render targets with rotating Suzanne monkeys         |
| Deferred             | ![Deferred Screenshot](screenshots/deferred.png) <br/> Drawing to multiple render targets and compositing them in another shader       |
| ImGui Integration    | ![ImGui Integration Screenshot](screenshots/imgui-integration.png) <br/> ImGui integration into AVA                                    |
| Push Constants       | ![Push Constants Screenshot](screenshots/pushconstants.png) Push constants changing the color and MVP of each cube                     |
| Ray Query            | ![Ray Query Screenshot](screenshots/rayquery.png) <br/> Ray queries using Vulkan's acceleration structure extension                    |
| Ray Tracing          | ![Ray Tracing Screenshot](screenshots/raytracing.png) <br/> Sponza scene, rendered using Vulkan's ray tracing extensions               | 

## Requirements

AVA requirements:

* [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) (tested with 1.4.304.0). AVA uses the following included in the SDK:
  * SPIRV-Cross
  * VulkanMemoryAllocator

AVA Example requirements:

* [glm](https://github.com/g-truc/glm)
* [glfw3](https://www.glfw.org/)

Libraries fetched with CMake:

* [VulkanMemoryAllocator-Hpp](https://github.com/YaaZ/VulkanMemoryAllocator-Hpp)
* [vk-bootstrap](https://github.com/charles-lunarg/vk-bootstrap)

Other libraries used by AVA examples:

* [stb_image.h](https://github.com/nothings/stb)
* [Dear ImGui](https://github.com/ocornut/imgui)

## Building

First clone the repository:

```shell
git clone https://github.com/James-Holloway/AVA
```

Build with CMake, or your IDE of choice (that supports CMake):

```shell
cmake -B build
cmake --build build
```

## Installation and Usage

To install into system-wide directories:

```shell
cmake --build build --target install
```

Note that this doesn't compile examples or the example framework header

To use with your CMake:

```cmake
find_package(ava REQUIRED)
[...]
target_link_libraries(MyProgram PUBLIC ava::ava)
```

Otherwise, link with `libava.a` (on Linux) and include the header directory (`/usr/local/include/ava/` by default on Linux)

## Headers

* `ava/ava.hpp`
    * Main header to be used when using AVA and unmanaged AVA objects
    * Includes all of `ava/*.hpp`
* `ava/raii.hpp`
    * Additional main header to be used for RAII-style objects
    * Includes all of `ava/raii/*.hpp`
* `ava/detail/*.hpp`
    * Implementation details of objects, include the relevant header to get fields of the unmanaged objects
* `examples/framework/framework.hpp`
    * The simple GLFW/GLM framework used by all examples in this repo. This may be a good starting point for your own AVA-based application

## How To Use AVA

See the examples for a more code-based example. `tessellation` is one of the most complete examples, featuring depth attachments, .OBJ loading, UBOs, descriptor pools & sets, and all shaders of the classic rasterization pipeline (vert/hull/domain/geometry/fragment).

### Main Loop

* Populate the `ava::CreateInfo createInfo;` struct with Vulkan API version, app name, debug state, device features and any extensions you wish to use.
* Configure the AVA State `ava::configureState(createInfo);`. This configures the state and creates the Vulkan instance.
* Create your window and window's Vulkan surface.
* Pass your surface to `ava::createState(surface);` to create the AVA State, which includes
* Create a swapchain with `ava::createSwapchain`
* Initialize any resources
* While your application/window is running:
    * Check for a resize event every frame with `ava::resizeNeeded()`
    * Get a new frame command buffer with `ava::startFrame` or `ava::raii::startFrame`
    * Start the command buffer
    * Render using the command buffer
    * End the command buffer
    * Present the frame using `ava::presentFrame();`
* Once the application has finished, clean up any resources. If using RAII, you can just `.reset()` on the shared pointers
* Destroy the ava state with `ava::destroyState();`

### Unmanaged Objects

* Create an object with the ava::createX function
    * E.g. `ava::createBuffer(size, vkbufferUsage, bufferLocation, alignment);`
* Destroy the object with the ava::destroyX function:
    * E.g. `destroyBuffer(buffer);`
    * This will deallocate the memory used and set the buffer to `nullptr`
* Use provided functions or `detail` headers to access the underlying Vulkan objects
* Use the provided functions to act on the relevant objects
    * E.g. `bindVIBO(commandBuffer, vibo);`

### RAII Objects

* RAII objects have the `Ptr` typedef member which refers to a `std::shared_ptr` of the RAII object's type
* Create objects with an existing unmanaged object and `std::make_shared` the provided `create` static functions
* Once a RAII object goes out of scope (e.g. `object.reset()`) then its resources are freed there and then
* The objects provide relevant functions, which call the unmanaged object functions
