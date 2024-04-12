# **********************************************************************************************************************
# Updated 11/03/2024
# **********************************************************************************************************************

# **********************************************************************************************************************

MACRO(macro_setup_base_project project_name project_version project_build_mode)

    # Check version.
    if (NOT ${CMAKE_VERSION} VERSION_LESS "3.1.0")
        cmake_policy(SET CMP0054 NEW)
    endif()

    # Project name.
    project(${project_name} VERSION ${project_version})

    # Log.
    message(STATUS "Preparing project: ${project_name} - ${project_version}")

    #Configure the build type.
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE ${project_build_mode})
    endif()

    # Output directories.
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    # Check system architectrue.
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SYS_ARCH "64 bits")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(SYS_ARCH "32 bits")
    endif()

    # Logs.
    if(WIN32)
        set(OS_NAME "Windows")
    elseif(APPLE)
        set(OS_NAME "Apple")
    elseif(UNIX AND NOT APPLE)
        set(OS_NAME "Linux/Unix")
    else()
        set(OS_NAME "Unknown")
    endif()
    message(STATUS "  Project Name: ${CMAKE_PROJECT_NAME}")
    message(STATUS "  Project Version: ${CMAKE_PROJECT_VERSION}")
    message(STATUS "  C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    message(STATUS "  Operating System: ${OS_NAME}")
    message(STATUS "  Processor Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    message(STATUS "  System Architecture: ${SYS_ARCH}")
    message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
    message(STATUS "  Build Archive: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    message(STATUS "  Build Library: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    message(STATUS "  Build Runtime: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    message(STATUS "  Force installation path: ${MODULES_GLOBAL_FORCE_INSTALL_DIR}")
    message(STATUS "  Install external dependencies: ${MODULES_GLOBAL_INSTALL_EXT_DEPS}")
    message(STATUS "  Install runtime artifacts: ${MODULES_GLOBAL_INSTALL_RUNTIME_ARTIFACTS}")

ENDMACRO()

# **********************************************************************************************************************

# Function to search for packages.
MACRO(macro_find_package_default package version version_mode extra_search_paths extra_search_patterns)

    # Compose the configuration folder.
    macro_compose_current_architecture_folder_name(CONFIG_FOLDER)

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
            "external/${package}"                 # In the same project.
            "../${package}"                       # As subproject installation
            "../../${package}"                    # As subproject installation
            "C:/${package}"                       # In standard root folder.
            "C:/Program Files/${package}"         # Standard installation.
            "C:/Program Files (x86)/${package}"   # Standard installation.
        )
    else()
        set(SEARCH_PATHS
            ${extra_search_paths}
            "../${package}"              # As subproject installation
            "../../${package}"           # As subproject installation
            "/usr/local/${package}"      # Standard local installation
            "/usr/lib/${package}"        # Standard library path
            "/usr/lib/cmake/${package}"  # Standard library path
            "lib/cmake/${package}"       # Standard library path
            "lib/${package}"             # Standard library path
            "/opt/${package}"            # Optional software installations
            "~/${package}"               # Home directory installation
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
                         break()
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
