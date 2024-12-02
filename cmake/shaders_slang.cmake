function(compile_shaders_slang TARGET_NAME BINRELOUTDIR)

    set(SHADER_SOURCE_FILES ${ARGN})

    list(LENGTH SHADER_SOURCE_FILES FILE_COUNT)
    if (FILE_COUNT EQUAL 0)
        return()
    endif ()

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${BINRELOUTDIR}")

    set(SHADER_COMMANDS)
    set(SHADER_OUTPUTS)

    set(SLANG_SHADER_FLAGS
            -profile sm_6_6+spirv_1_5
            -target spirv
            -emit-spirv-directly
            -force-glsl-scalar-layout
            -matrix-layout-column-major
            -fvk-use-entrypoint-name # don't emit main as entrypoint but instead use named entrypoints
            -g3 # Debug level 3
            -O0 # Default optimization breaks pointers?
    )

    foreach (SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
        cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
        cmake_path(GET SHADER_SOURCE FILENAME SHADER_NAME)

        list(APPEND SHADER_COMMANDS COMMAND)
        list(APPEND SHADER_COMMANDS slangc) # Requires Vulkan or slangc to be on the path
        list(APPEND SHADER_COMMANDS "${SHADER_SOURCE}")
        list(APPEND SHADER_COMMANDS ${SLANG_SHADER_FLAGS})
        list(APPEND SHADER_COMMANDS "-o")
        list(APPEND SHADER_COMMANDS "${CMAKE_CURRENT_BINARY_DIR}/${BINRELOUTDIR}/${SHADER_NAME}.spv")

        list(APPEND SHADER_PRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${BINRELOUTDIR}/${SHADER_NAME}.spv")
    endforeach ()

    add_custom_target("${TARGET_NAME}SlangShaders"
            COMMAND ${SHADER_COMMANDS}
            COMMENT "Compiling Slang Shaders [${TARGET_NAME}]"
            SOURCES ${SHADER_SOURCE_FILES}
            BYPRODUCTS ${SHADER_OUTPUTS})
    add_dependencies(${TARGET_NAME} "${TARGET_NAME}SlangShaders")

endfunction()