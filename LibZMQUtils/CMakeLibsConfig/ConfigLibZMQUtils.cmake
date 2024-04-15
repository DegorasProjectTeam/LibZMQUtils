# **********************************************************************************************************************
# Updated 15/04/2024
# **********************************************************************************************************************

# **********************************************************************************************************************

# Macro to search for LibZMQUtils
MACRO(macro_configure_libzmqutils_default version version_mode extra_search_paths extra_search_patterns)

    # Log.
    message(STATUS "Configuring LibZMQUtils...")

    # Setup the find package config.
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

    # Configure extra things if neccesary.
    # ...

    # Find the package.
    macro_find_package_default("LibZMQUtils" "${version}" "${version_mode}" "${extra_search_paths}" "${extra_search_patterns}")

    # Logs.
    get_target_property(LibZMQUtils_INCLUDES LibZMQUtils::LibZMQUtils INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(LibZMQUtils_LINK_LIBS LibZMQUtils::LibZMQUtils INTERFACE_LINK_LIBRARIES)
    get_target_property(LibZMQUtils_LOCATION LibZMQUtils::LibZMQUtils LOCATION)
    get_target_property(LibZMQUtils_LOCATION_DEBUG LibZMQUtils::LibZMQUtils IMPORTED_LOCATION_DEBUG)
    get_target_property(LibZMQUtils_LOCATION_RELEASE LibZMQUtils::LibZMQUtils IMPORTED_LOCATION_RELEASE)
    message(STATUS "  LibZMQUtils::LibZMQUtils information:")
    message(STATUS "    Library version: ${LibZMQUtils_VERSION}")
    message(STATUS "    Configuration path: ${LibZMQUtils_DIR}")
    message(STATUS "    Include directories: ${LibZMQUtils_INCLUDES}")
    message(STATUS "    Interface link libraries: ${LibZMQUtils_LINK_LIBS}")
    message(STATUS "    DLL location: ${LibZMQUtils_LOCATION}")
    message(STATUS "    DLL location debug: ${LibZMQUtils_LOCATION_DEBUG}")
    message(STATUS "    DLL location release: ${LibZMQUtils_LOCATION_RELEASE}")

ENDMACRO()

# **********************************************************************************************************************

# Macro to link target to LibZMQUtils
MACRO(macro_link_libzmqutils_default target visibility)

    message(STATUS "Linking LibZMQUtils to target ${target} with ${visibility} visibility")

    target_link_libraries(${target} ${visibility} LibZMQUtils::LibZMQUtils)

    if(MODULES_GLOBAL_SHOW_EXTERNALS)

        # LibZMQUtils
        get_target_property(LibZMQUtils_INCLUDES LibZMQUtils::LibZMQUtils INTERFACE_INCLUDE_DIRECTORIES)
        file(GLOB_RECURSE EXTERNAL_HEADERS ${LibZMQUtils_INCLUDES}/*)
        target_sources(${target} ${visibility} ${EXTERNAL_HEADERS})


    endif()

ENDMACRO()

# **********************************************************************************************************************
