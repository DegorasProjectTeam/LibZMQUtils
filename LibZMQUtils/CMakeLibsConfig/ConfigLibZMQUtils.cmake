# **********************************************************************************************************************
#    LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
#                                                                                                                      *
#    A modern open-source and cross-platform C++ library with high-level utilities based on the well-known ZeroMQ      *
#    open-source universal messaging library. Includes a suite of modules that encapsulates the ZMQ communication      *
#    patterns as well as automatic binary serialization capabilities, specially designed for system infraestructure.   *
#    The library is suited for the quick and easy integration of new and old systems and can be used in different      *
#    sectors and disciplines seeking robust messaging and serialization solutions.                                     *
#                                                                                                                      *
#    Developed as free software within the context of the Degoras Project for the Satellite Laser Ranging Station      *
#    (SFEL) at the Spanish Navy Observatory (ROA) in San Fernando, Cádiz. The library is open for use by other SLR     *
#    stations and organizations, so we warmly encourage you to give it a try and feel free to contact us anytime!      *
#                                                                                                                      *
#    Copyright (C) 2024 Degoras Project Team                                                                           *
#                       < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >                             *
#                       < Jesús Relinque Madroñal >                                                                    *
#                                                                                                                      *
#    This file is part of LibZMQUtils.                                                                                 *
#                                                                                                                      *
#    Licensed under the European Union Public License (EUPL), Version 1.2 or subsequent versions of the EUPL license   *
#    as soon they will be approved by the European Commission (IDABC).                                                 *
#                                                                                                                      *
#    This project is free software: you can redistribute it and/or modify it under the terms of the EUPL license as    *
#    published by the IDABC, either Version 1.2 or, at your option, any later version.                                 *
#                                                                                                                      *
#    This project is distributed in the hope that it will be useful. Unless required by applicable law or agreed to in *
#    writing, it is distributed on an "AS IS" basis, WITHOUT ANY WARRANTY OR CONDITIONS OF ANY KIND; without even the  *
#    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the EUPL license to check specific   *
#    language governing permissions and limitations and more details.                                                  *
#                                                                                                                      *
#    You should use this project in compliance with the EUPL license. You should have received a copy of the license   *
#    along with this project. If not, see the license at < https://eupl.eu/ >.                                         *
# **********************************************************************************************************************

# **********************************************************************************************************************
# Updated 22/05/2024
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
    macro_find_package_default_private("LibZMQUtils" "${version}" "${version_mode}" "${extra_search_paths}" "${extra_search_patterns}")

    # Set direct access to the library useful properties.
    get_target_property(LibZMQUtils_INCLUDES LibZMQUtils::LibZMQUtils INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(LibZMQUtils_LINK_LIBS LibZMQUtils::LibZMQUtils INTERFACE_LINK_LIBRARIES)
    get_target_property(LibZMQUtils_LOCATION LibZMQUtils::LibZMQUtils LOCATION)
    get_target_property(LibZMQUtils_LOCATION_DEBUG LibZMQUtils::LibZMQUtils IMPORTED_LOCATION_DEBUG)
    get_target_property(LibZMQUtils_LOCATION_RELEASE LibZMQUtils::LibZMQUtils IMPORTED_LOCATION_RELEASE)
    get_filename_component(LibZMQUtils_LOCATION_DIR "${LibZMQUtils_LOCATION}" DIRECTORY)

    # Set the external dependecies common location.
    set(LibZMQUtils_LOCATION_DEPENDENCIES ${LIBZMQ_LIBRARIES})
    set(LibZMQUtils_LOCATION_DEPENDENCIES_DIR ${LIBZMQ_LIBRARY_DIRS})

    # Set the external dependecies location for debug and release.
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(LibZMQUtils_LOCATION_DEPENDENCIES_DEBUG ${LIBZMQ_LIBRARIES})
    else()
        set(LibZMQUtils_LOCATION_DEPENDENCIES_RELEASE ${LIBZMQ_LIBRARIES})
    endif()

    # Logs.
    message(STATUS "  LibZMQUtils::LibZMQUtils information:")
    message(STATUS "    Library version: ${LibZMQUtils_VERSION}")
    message(STATUS "    Configuration path: ${LibZMQUtils_DIR}")
    message(STATUS "    Include directories: ${LibZMQUtils_INCLUDES}")
    message(STATUS "    Interface link libraries: ${LibZMQUtils_LINK_LIBS}")
    message(STATUS "    Lib location dir: ${LibZMQUtils_LOCATION_DIR}")
    message(STATUS "    Lib location: ${LibZMQUtils_LOCATION}")
    message(STATUS "    Lib location debug: ${LibZMQUtils_LOCATION_DEBUG}")
    message(STATUS "    Lib location release: ${LibZMQUtils_LOCATION_RELEASE}")
    message(STATUS "    Dependencies dir: ${LibZMQUtils_LOCATION_DEPENDENCIES_DIR}")
    message(STATUS "    Dependencies location: ${LibZMQUtils_LOCATION_DEPENDENCIES}")
    message(STATUS "    Dependencies location release: ${LibZMQUtils_LOCATION_DEPENDENCIES_RELEASE}")
    message(STATUS "    Dependencies location debug: ${LibZMQUtils_LOCATION_DEPENDENCIES_DEBUG}")

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
# **********************************************************************************************************************

MACRO(macro_compose_current_architecture_folder_name_private result_var)

    # Check if 64-bit or 32-bit
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH "x86_64")
    else()
        set(ARCH "x86")
    endif()

    # Check if debug or release.
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(BUILD_TYPE "debug")
    else()
        set(BUILD_TYPE "release")
    endif()

    # Get the compiler version.
    set(COMP_V "${CMAKE_CXX_COMPILER_VERSION}")

    # Check the compiler name.
    if(MINGW)
        set(COMP_N "mingw")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(COMP_N "msvc")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(COMP_N "gnu")
    else()
        message(FATAL_ERROR "Compiler not supported by default.")
    endif()

    # Construct the configuration folder name
    set(${result_var} "${COMP_N}-${ARCH}-${COMP_V}-${BUILD_TYPE}")

ENDMACRO()

# Function to search for packages.
MACRO(macro_find_package_default_private package version version_mode extra_search_paths extra_search_patterns)

    # Compose the configuration folder.
    macro_compose_current_architecture_folder_name_private(CONFIG_FOLDER)

    # Convert ${package} to lowercase
    string(TOLOWER ${package} PACKAGE_LOWERCASE)

    # List of CMake configuration file patterns.
    set(CONFIG_FILE_PATTERNS
        "${package}Config.cmake"
        "${package}-config.cmake"
        "${PACKAGE_LOWERCASE}Config.cmake"
        "${PACKAGE_LOWERCASE}-config.cmake"
    )

    # Log.
    message(STATUS "Searching for package configuration file: ${package}")
    message(STATUS "  Determined architecture folder: ${CONFIG_FOLDER}")

    # Installation folder pattern.
    set(SEARCH_PATTERNS
        ${extra_search_patterns}
        "${CONFIG_FOLDER}/cmake/"
        "${CONFIG_FOLDER}/share/"
        "${CONFIG_FOLDER}/share/cmake/"
        "*product*/${CONFIG_FOLDER}"
        "*product*/${CONFIG_FOLDER}/share/"
        "*product*/${CONFIG_FOLDER}/share/cmake"
        )

    # List of possible relative paths where package might be installed.
    if(WIN32)
        set(SEARCH_PATHS
            ${extra_search_paths}
            "external/*${package}*"                 # In the same project.
            "../*${package}*"                       # As subproject installation
            "../../*${package}*"                    # As subproject installation
            "C:/*${package}*"                       # In standard root folder.
            "C:/Program Files/*${package}*"         # Standard installation.
            "C:/Program Files (x86)/*${package}*"   # Standard installation.
        )
    else()
        set(SEARCH_PATHS
            ${extra_search_paths}
            "../"              # As subproject installation
            "../../"           # As subproject installation
            "/usr/local/"      # Standard local installation
            "/usr/lib/"        # Standard library path
            "/usr/lib/cmake/"  # Standard library path
            "lib/cmake/"       # Standard library path
            "lib/"             # Standard library path
            "/opt/"            # Optional software installations
            "~/"               # Home directory installation
            )
    endif()

    # List to store all found paths
    set(FOUND_PATHS "")

    # Search for library.
    foreach(PATH IN LISTS SEARCH_PATHS)
        foreach(PATTERN IN LISTS SEARCH_PATTERNS)

            # Get the matchs.
            set(CURRENT_SEARCH "${PATH}/${PATTERN}")
            file(GLOB PATTERN_DIRS "${CURRENT_SEARCH}")

            # Iterate through the list of paths
            foreach(MATCH IN LISTS PATTERN_DIRS)
                get_filename_component(MATCH "${MATCH}" ABSOLUTE)

                # Check if any of the lowercase CMake configuration files exist in the directory.
                 foreach(CONFIG_PATTERN IN LISTS CONFIG_FILE_PATTERNS)
                     message(STATUS "  Checking: ${MATCH}/${CONFIG_PATTERN}")
                     if(EXISTS "${MATCH}/${CONFIG_PATTERN}")
                         list(APPEND FOUND_PATHS "${MATCH}")
                     endif()
                 endforeach()

            endforeach()
        endforeach()
    endforeach()

    # Log all matches
    foreach(MATCH IN LISTS FOUND_PATHS)
        message(STATUS "  Matches: ${MATCH}")
    endforeach()

    # Check if any matches were found
    if(NOT FOUND_PATHS)
        message(FATAL_ERROR "  The package was not found: ${package}")
    else()

        # Check if version is specified and update the find_package command accordingly
        if(version STREQUAL "")
            message(STATUS "  Ignoring version.")
            find_package(${package} REQUIRED PATHS ${FOUND_PATHS} NO_DEFAULT_PATH)
        else()
            message(STATUS "Using version: ${version} with mode: ${version_mode}")
            find_package(${package} ${version} ${version_mode} REQUIRED PATHS ${FOUND_PATHS})
        endif()

        if(NOT ${package}_FOUND)
             message(FATAL_ERROR "  Configuration file for ${package} not found.")
        else()
            # Set the found configuration path.
            set(${package}_CONFIG_PATH ${FOUND_PATHS})
            # Log all found package details
            message(STATUS "  Package '${package}' found.")
            message(STATUS "  Package version: ${${package}_VERSION}")
            message(STATUS "  Config dir: ${${package}_DIR}")
        endif()

    endif()

ENDMACRO()

# **********************************************************************************************************************
# **********************************************************************************************************************
