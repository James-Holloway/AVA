function(set_assets TARGET_NAME BINRELDIR)
    set(options SYMLINK)
    set(oneValueArgs)
    set(multiValueArgs FILES)
    cmake_parse_arguments(ASSET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    list(LENGTH ASSET_FILES FILE_COUNT)

    if (FILE_COUNT EQUAL 0)
        return()
    endif ()

    set(ASSET_DIR "${CMAKE_CURRENT_BINARY_DIR}/${BINRELDIR}")

    # Create main Assets directory
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -E make_directory ${ASSET_DIR}
            COMMENT "Creating assets directory ${ASSET_DIR} [${TARGET_NAME}]"
    )

    foreach (ASSET_SOURCE IN LISTS ASSET_FILES)
        cmake_path(ABSOLUTE_PATH ASSET_SOURCE NORMALIZE)
        cmake_path(GET ASSET_SOURCE FILENAME ASSET_FILE_NAME)

        if (ASSET_SYMLINK)
            if (IS_DIRECTORY "${ASSET_SOURCE}")
                file(GLOB SUB_ASSETS "${ASSET_SOURCE}/*")
                foreach (SUB_ASSET ${SUB_ASSETS})
                    cmake_path(GET SUB_ASSET FILENAME SUB_ASSET_FILE_NAME)
                    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E create_symlink
                            "${SUB_ASSET}"
                            "${ASSET_DIR}/${SUB_ASSET_FILE_NAME}"
                            COMMENT "Creating symlink for asset ${SUB_ASSET_FILE_NAME} [${TARGET_NAME}]"
                    )
                endforeach (SUB_ASSET)

            else () # not directory
                add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E create_symlink
                        "${ASSET_SOURCE}"
                        "${ASSET_DIR}/${ASSET_FILE_NAME}"
                        COMMENT "Creating symlink for asset ${ASSET_FIlE_NAME} [${TARGET_NAME}]"
                )
            endif () # is directory

        else () # copy rather than symlink
            if (IS_DIRECTORY "${ASSET_SOURCE}")
                file(GLOB SUB_ASSETS "${ASSET_SOURCE}/*")
                foreach (SUB_ASSET ${SUB_ASSETS})
                    if (IS_DIRECTORY "${SUB_ASSET}")
                        set(COPY_ASSET_COMMAND copy_directory)
                    else () # not directory
                        set(COPY_ASSET_COMMAND copy_if_different)
                    endif ()

                    cmake_path(GET SUB_ASSET FILENAME SUB_ASSET_FILE_NAME)

                    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E ${COPY_ASSET_COMMAND}
                            "${SUB_ASSET}"
                            "${ASSET_DIR}/${SUB_ASSET_FILE_NAME}"
                            COMMENT "Copying asset ${SUB_ASSET_FILE_NAME} [${TARGET_NAME}]"
                    )
                endforeach ()

            else () # not directory
                add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${ASSET_SOURCE}"
                        "${ASSET_DIR}/${ASSET_FILE_NAME}"
                        COMMENT "Copying asset ${ASSET_FILE_NAME} [${TARGET_NAME}]"
                )
            endif () # is directory
        endif () # is symlink
    endforeach () # ASSET_SOURCE
endfunction()