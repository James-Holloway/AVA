function(compile_shaders_glslc TARGET_NAME BINRELOUTDIR)
    set(SHADER_SOURCE_FILES ${ARGN})

    list(LENGTH SHADER_SOURCE_FILES FILE_COUNT)
    if (FILE_COUNT EQUAL 0)
        return()
    endif ()

    set(SHADER_COMMANDS)
    set(SHADER_OUTPUTS)

    foreach (SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
        cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
        cmake_path(GET SHADER_SOURCE FILENAME SHADER_NAME)

        list(APPEND SHADER_COMMANDS COMMAND)
        list(APPEND SHADER_COMMANDS Vulkan::glslc)
        list(APPEND SHADER_COMMANDS "--target-env=vulkan1.2")
        list(APPEND SHADER_COMMANDS "-g") # Debug
        list(APPEND SHADER_COMMANDS "${SHADER_SOURCE}")
        list(APPEND SHADER_COMMANDS "-o")
        list(APPEND SHADER_COMMANDS "${CMAKE_CURRENT_BINARY_DIR}/${BINRELOUTDIR}/${SHADER_NAME}.spv")

        list(APPEND SHADER_PRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${BINRELOUTDIR}/${SHADER_NAME}.spv")
    endforeach ()

    add_custom_target("${TARGET_NAME}GLSLShaders"
            COMMAND ${SHADER_COMMANDS}
            COMMENT "Compiling GLSL Shaders [${TARGET_NAME}]"
            SOURCES ${SHADER_SOURCE_FILES}
            BYPRODUCTS ${SHADER_PRODUCTS})
    add_dependencies(${TARGET_NAME} "${TARGET_NAME}GLSLShaders")

endfunction()