# **********************************************************************************************************************
# Updated 11/03/2024
# **********************************************************************************************************************

# **********************************************************************************************************************

MACRO(macro_setup_default_install_dir base_dir)

    # Log
    message(STATUS "Preparing the default installation directory... ")

    # Resolve the path
    get_filename_component(real_base_dir "${base_dir}" REALPATH)

    # Set the installation path.
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR ${MODULES_GLOBAL_FORCE_INSTALL_DIR})
        message(STATUS "  Force the base installation directory: ${RESOLVED_PATH}")
        set(CMAKE_INSTALL_PREFIX ${real_base_dir} CACHE PATH "..." FORCE)
    endif()

    # Add properties for clean process.
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_INSTALL_PREFIX})

    # Compose the folder name
    macro_compose_current_architecture_folder_name(INSTALL_FOLDER)

    # Define the install directories.
    set(BIN_DIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_FOLDER}/bin)
    set(LIB_DIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_FOLDER}/lib)
    set(SHA_DIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_FOLDER}/share)
    set(INC_DIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_FOLDER}/include)

    # Specific SO configurations.
    if(WIN32)
    #
    # Nothing specific.
    #
    elseif(OS_NAME STREQUAL "Linux/Unix")

        # Update RPATH.
        message(STATUS "  Updating RPATH...")

         # Use, i.e. don't skip the full RPATH for the build tree
        set(CMAKE_SKIP_BUILD_RPATH FALSE)

        # When building, don't use the install RPATH already (but later on when installing).
        set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
        set(CMAKE_INSTALL_RPATH ${BIN_DIR})

        # Add the automatically determined parts of the RPATH which point to
        # directories outside the build tree.
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    else()
        message(FATAL_ERROR "  Operating system not supported by default.")
    endif()

    # Setup the global variables.
    macro_global_set_install_bin_path(${BIN_DIR})
    macro_global_set_install_lib_path(${LIB_DIR})
    macro_global_set_install_share_path(${SHA_DIR})
    macro_global_set_install_include_path(${INC_DIR})

    # Logs.
    message(STATUS "  Base dir:      ${real_base_dir}")
    message(STATUS "  Binaries dir:  ${BIN_DIR}")
    message(STATUS "  Libraries dir: ${LIB_DIR}")
    message(STATUS "  Shared dir:    ${SHA_DIR}")
    message(STATUS "  Includes dir:  ${INC_DIR}")

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_install_runtime_artifacts target dependency_set install_path)

    if(${MODULES_GLOBAL_INSTALL_RUNTIME_ARTIFACTS})

        # Log.
        message(STATUS "Installing runtime artifacts for: ${target}")

        # Installation process into installation dir.
        install(IMPORTED_RUNTIME_ARTIFACTS ${target}
                RUNTIME_DEPENDENCY_SET ${dependency_set}
                DESTINATION ${install_path})

    endif()

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_install_runtime_deps target dependency_set ext_deps_dirs bin_dest pre_exc_regexes post_exc_regexes)

    if(${MODULES_GLOBAL_INSTALL_RUNTIME_DEPS})

        # Log
        message(STATUS "Installing external dependencies for: ${target}")
        message(STATUS "  External dependencies paths: ${ext_deps_dirs}")
        message(STATUS "  Installation path: ${bin_dest}")
        message(STATUS "  Bin installation path: ${CMAKE_CURRENT_BINARY_DIR}")

        if(${MODULES_GLOBAL_EXCLUDE_SO_LIBS})

            # Exclude SO dll and libs.
            if(WIN32)

                message(STATUS "  Excluding Windows SO libraries...")
                set(PRE_EXC ${pre_exc_regexes} "api-ms-" "ext-ms-")
                set(POST_EXC ${post_exc_regexes} ".*system32/.*\\.dll")

            elseif(OS_NAME STREQUAL "Linux/Unix")

                message(STATUS "  Excluding Unix SO libraries...")
                set(POST_EXC ${post_exc_regexes} "/lib" "usr/lib" "/lib64" "/usr/lib64")

            endif()

        else()
            set(PRE_EXC ${pre_exc_regexes})
            set(POST_EXC ${post_exc_regexes})
        endif()

        # Mandatory SO dll exclusion.
        if(WIN32)
            set(PRE_EXC ${pre_exc_regexes} "api-ms-" "ext-ms-")
        endif()

        # Install runtime dependencies for the set.
        install(RUNTIME_DEPENDENCY_SET ${dependency_set}
                PRE_EXCLUDE_REGEXES ${PRE_EXC}
                POST_EXCLUDE_REGEXES ${POST_EXC}
                DIRECTORIES ${ext_deps_dirs}
                DESTINATION ${bin_dest})
    endif()

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_install_launcher target bin_dest)

    # Log.
    message(STATUS "Installing executable: ${target}")
    message(STATUS "  Deployment destination: ${bin_dest}")

    #Deploy binary files
    install(TARGETS ${target}
            RUNTIME DESTINATION ${bin_dest})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_install_lib lib_name inc_path inc_dest lib_dest bin_dest arch_dest sha_dest)

    # Log information
    message(STATUS "Installing library: ${lib_name}")
    message(STATUS "  Includes destination: ${inc_dest}")
    message(STATUS "  Library destination: ${lib_dest}")
    message(STATUS "  Binary destination: ${bin_dest}")
    message(STATUS "  Archive destination: ${arch_dest}")
    message(STATUS "  Shared destination: ${sha_dest}")

    # Config and version file names
    set(CONFIG_FILE_NAME "${lib_name}Config.cmake")
    set(VERSION_FILE_NAME "${lib_name}ConfigVersion.cmake")

    # Set target properties for include directories
    target_include_directories(${lib_name} PUBLIC
        $<BUILD_INTERFACE:${inc_path}>
        $<INSTALL_INTERFACE:${inc_dest}>)

    # Install include files
    install(DIRECTORY ${inc_path}
            DESTINATION ${inc_dest}
            PATTERN "*.txt" EXCLUDE)

    # Get the version of the library
    get_target_property(EXTRACTED_VERSION ${lib_name} VERSION)

    # Write the version to the package file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${VERSION_FILE_NAME}"
        VERSION ${EXTRACTED_VERSION}
        COMPATIBILITY SameMajorVersion)

    # Install the library (and binaries if applicable)
    install(TARGETS ${lib_name}
            EXPORT ${lib_name}Targets
            LIBRARY DESTINATION ${lib_dest}
            ARCHIVE DESTINATION ${arch_dest}
            RUNTIME DESTINATION ${bin_dest}
            INCLUDES DESTINATION ${inc_dest})

    # Export the target
    install(EXPORT ${lib_name}Targets
            FILE ${lib_name}Targets.cmake
            NAMESPACE ${lib_name}::
            DESTINATION ${sha_dest}/cmake)

    # Manually create the Config.cmake file
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE_NAME}"
         "include(\${CMAKE_CURRENT_LIST_DIR}/${lib_name}Targets.cmake)\n")

    # Check if the ".in" template file exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${lib_name}Config.cmake.in")

        # Configure the "Config.cmake" file from the template
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${lib_name}Config.cmake.in"
                       "${CMAKE_CURRENT_BINARY_DIR}/${lib_name}Config.cmake" @ONLY)

    else()

        # Configuration without template.
        configure_file("${CMAKE_CURRENT_BINARY_DIR}/${lib_name}Config.cmake" @ONLY)

    endif()

    # Install the manually created Config.cmake file
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE_NAME}"
                  "${CMAKE_CURRENT_BINARY_DIR}/${VERSION_FILE_NAME}"
                  "${CMAKE_CURRENT_BINARY_DIR}/${lib_name}Config.cmake"
            DESTINATION ${sha_dest}/cmake)

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_default_library_installation lib_name lib_includes_dir ext_deps_dirs)

    # Set the external dependencies dir.
    set(external_deps_search_dirs ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${ext_deps_dirs})

    # Default installation process for windows.
    if(WIN32)

        # Install the library.
        macro_install_lib("${lib_name}"
                          "${lib_includes_dir}"
                          "${MODULES_GLOBAL_INSTALL_INCLUDE_PATH}"
                          "${MODULES_GLOBAL_INSTALL_BIN_PATH}"
                          "${MODULES_GLOBAL_INSTALL_LIB_PATH}"
                          "${MODULES_GLOBAL_INSTALL_LIB_PATH}"
                          "${MODULES_GLOBAL_INSTALL_SHARE_PATH}")

        # Install the runtime artifacts.
        macro_install_runtime_artifacts("${lib_name}"
                                        "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                                        "${MODULES_GLOBAL_INSTALL_BIN_PATH}")

        # Install external dependencies.
        macro_install_runtime_deps("${lib_name}"
                                   "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                                   "${external_deps_search_dirs}"
                                   "${MODULES_GLOBAL_INSTALL_BIN_PATH}"
                                   "" "")

    elseif(OS_NAME STREQUAL "Linux/Unix")

        # Install the library.
        macro_install_lib("${lib_name}" "${lib_includes_dir}"
                          "${MODULES_GLOBAL_INSTALL_INCLUDE_PATH}"
                          "${MODULES_GLOBAL_INSTALL_LIB_PATH}"
                          "${MODULES_GLOBAL_INSTALL_LIB_PATH}"
                          "${MODULES_GLOBAL_INSTALL_LIB_PATH}"
                          "${MODULES_GLOBAL_INSTALL_SHARE_PATH}")

        # Install the runtime artifacts.
        macro_install_runtime_artifacts("${lib_name}"
                                        "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                                        "${MODULES_GLOBAL_INSTALL_BIN_PATH}")

        # Install external dependencies.
        macro_install_runtime_deps("${lib_name}"
                                   "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                                   "${external_deps_search_dirs}"
                                   "${MODULES_GLOBAL_INSTALL_BIN_PATH}"
                                   "" "")

    else()
        message(FATAL_ERROR "Operating system not supported by default.")
    endif()

ENDMACRO()

# **********************************************************************************************************************
