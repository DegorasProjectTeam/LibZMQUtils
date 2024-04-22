# **********************************************************************************************************************
# Updated 22/04/2024
# **********************************************************************************************************************

# **********************************************************************************************************************

MACRO(macro_cmakemodules_init version)

    # Set the version
    set(MODULES_GLOBAL_CMAKEMODULES_VERSION "24.04.22")

    # Check the CMakeModules version.
    if(NOT ${version} EQUAL ${MODULES_GLOBAL_CMAKEMODULES_VERSION})
        message(FATAL_ERROR "CMakeModules version (${MODULES_GLOBAL_CMAKEMODULES_VERSION}) not match: ${version}")
    endif()

    # CMake includes
    include(CMakePackageConfigHelpers)

    # Includes.
    include(CMakeMacrosInstall)
    include(CMakeMacrosLauncher)
    include(CMakeMacrosUtils)
    include(CMakeMacrosProjects)
    include(CMakeMacrosLibraries)

    # Default global variables values.
    macro_global_set_main_dep_set_name("")
    macro_global_set_libs_debug("")
    macro_global_set_libs_optimized("")
    macro_global_set_install_bin_path("")
    macro_global_set_install_lib_path("")
    macro_global_set_install_share_path("")
    macro_global_set_install_include_path("")
    macro_global_set_install_include_path("")
    macro_global_set_libs_full_paths("")
    macro_global_set_libs_folders("")
    macro_global_set_launcher_dirs("")
    macro_global_set_launcher_deploys_dirs("")
    macro_global_set_show_externals(FALSE)
    macro_global_set_force_install_dir(TRUE)
    macro_global_set_install_runtime_deps(FALSE)
    macro_global_set_install_runtime_artifacts(TRUE)
    macro_global_set_exclude_so_libraries(TRUE)

ENDMACRO()

# **********************************************************************************************************************

FUNCTION(macro_global_set_launcher_deploys_dirs launcher_deploy_dir)

    set(MODULES_GLOBAL_LAUNCHER_DEPLOYS_DIRS ${launcher_deploy_dir}
        CACHE STRING "Global list of launchers deploys dirs" FORCE)

ENDFUNCTION()

# **********************************************************************************************************************

FUNCTION(macro_global_add_launcher_deploys_dirs launcher_deploys_dirs)
    string(REGEX REPLACE "/$" "" launcher_dirs_rep "${launcher_deploys_dirs}")
    # Check if already in the list
    if(NOT "${MODULES_GLOBAL_LAUNCHER_DEPLOYS_DIRS};" MATCHES "${launcher_dirs_rep};")
        # Conditionally append the semicolon only if the list is not empty
        if(MODULES_GLOBAL_LAUNCHER_DEPLOYS_DIRS STREQUAL "")
            set(MODULES_GLOBAL_LAUNCHER_DEPLOYS_DIRS "${launcher_dirs_rep}" CACHE STRING "Global list of launchers deploys dirs" FORCE)
        else()
            set(MODULES_GLOBAL_LAUNCHER_DEPLOYS_DIRS "${MODULES_GLOBAL_LAUNCHER_DEPLOYS_DIRS};${launcher_dirs_rep}"
                CACHE STRING "Global list of launchers deploys dirs" FORCE)
        endif()
    endif()
ENDFUNCTION()

# **********************************************************************************************************************

FUNCTION(macro_global_add_launcher_dirs launcher_dirs)
    string(REGEX REPLACE "/$" "" launcher_dirs_rep "${launcher_dirs}")
    # Check if already in the list
    if(NOT ";${MODULES_GLOBAL_LAUNCHER_DIRS};" MATCHES ";${launcher_dirs_rep};")
        # Conditionally append the semicolon only if the list is not empty
        if(MODULES_GLOBAL_LAUNCHER_DIRS STREQUAL "")
            set(MODULES_GLOBAL_LAUNCHER_DIRS "${launcher_dirs_rep}"
                CACHE STRING "Global list of launchers" FORCE)
        else()
            set(MODULES_GLOBAL_LAUNCHER_DIRS "${MODULES_GLOBAL_LAUNCHER_DIRS};${launcher_dirs_rep}"
                CACHE STRING "Global list of launchers" FORCE)
        endif()
    endif()
ENDFUNCTION()

# **********************************************************************************************************************

FUNCTION(macro_global_set_launcher_dirs launcher_dir)

    set(MODULES_GLOBAL_LAUNCHER_DIRS ${launcher_dir} CACHE STRING "Global list of launchers" FORCE)

ENDFUNCTION()

# **********************************************************************************************************************

FUNCTION(macro_global_set_libs_folders libs_folders)

    set(MODULES_GLOBAL_LIBS_FOLDERS ${libs_folders} CACHE STRING "Global list of folder with libraries" FORCE)

ENDFUNCTION()

# **********************************************************************************************************************

FUNCTION(macro_global_set_libs_full_paths libs_dirs)

    set(MODULES_GLOBAL_LIBS_FULL_PATHS ${libs_dirs} CACHE STRING "Global list of libraries" FORCE)

ENDFUNCTION()

# **********************************************************************************************************************

FUNCTION(macro_global_add_libs_auto lib_names)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MODULES_GLOBAL_LIBS_DEBUG ${MODULES_GLOBAL_LIBS_DEBUG} ${lib_names})
else()
    set(MODULES_GLOBAL_LIBS_OPTIMIZED ${MODULES_GLOBAL_LIBS_OPTIMIZED} ${lib_names})
endif()

ENDFUNCTION()

# **********************************************************************************************************************

FUNCTION(macro_global_add_libs_folders)
    foreach(arg IN LISTS ARGN)
        foreach(path IN LISTS arg)
            if(NOT ";${MODULES_GLOBAL_LIBS_FOLDERS};" MATCHES ";${path};")
                # Check if the list is empty before appending
                if("${MODULES_GLOBAL_LIBS_FOLDERS}" STREQUAL "")
                    set(MODULES_GLOBAL_LIBS_FOLDERS "${path}" CACHE STRING "Global list of folder with libraries" FORCE)
                else()
                    set(MODULES_GLOBAL_LIBS_FOLDERS "${MODULES_GLOBAL_LIBS_FOLDERS};${path}" CACHE STRING "Global list of folder with libraries" FORCE)
                endif()
            endif()
        endforeach()
    endforeach()
ENDFUNCTION()

# **********************************************************************************************************************

FUNCTION(macro_global_add_libs_full_paths)
    foreach(arg IN LISTS ARGN)
        foreach(path IN LISTS arg)
            if(NOT ";${MODULES_GLOBAL_LIBS_FULL_PATHS};" MATCHES ";${path};")
                # Check if the list is empty before appending
                if("${MODULES_GLOBAL_LIBS_FULL_PATHS}" STREQUAL "")
                    set(MODULES_GLOBAL_LIBS_FULL_PATHS "${path}" CACHE STRING "Global list of libraries" FORCE)
                else()
                    set(MODULES_GLOBAL_LIBS_FULL_PATHS "${MODULES_GLOBAL_LIBS_FULL_PATHS};${path}" CACHE STRING "Global list of libraries" FORCE)
                endif()
            endif()
        endforeach()
    endforeach()
ENDFUNCTION()

# **********************************************************************************************************************

MACRO(macro_global_set_main_dep_set_name dep_set_name)

    set(MODULES_GLOBAL_MAIN_DEP_SET_NAME ${dep_set_name})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_libs_debug lib_names)

    set(MODULES_GLOBAL_LIBS_DEBUG ${lib_names})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_libs_optimized lib_names)

    set(MODULES_GLOBAL_LIBS_OPTIMIZED ${lib_names})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_add_libs_debug lib_names)

set(MODULES_GLOBAL_LIBS_DEBUG ${MODULES_GLOBAL_LIBS_DEBUG} ${lib_names})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_add_libs_optimized lib_names)

set(MODULES_GLOBAL_LIBS_OPTIMIZED ${MODULES_GLOBAL_LIBS_OPTIMIZED} ${lib_names})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_bin_path path)
    set(MODULES_GLOBAL_INSTALL_BIN_PATH ${path})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_lib_path path)
    set(MODULES_GLOBAL_INSTALL_LIB_PATH ${path})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_share_path path)
    set(MODULES_GLOBAL_INSTALL_SHARE_PATH ${path})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_include_path path)
    set(MODULES_GLOBAL_INSTALL_INCLUDE_PATH ${path})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_show_externals enabled)
    set(MODULES_GLOBAL_SHOW_EXTERNALS ${enabled})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_force_install_dir enabled)
    set(MODULES_GLOBAL_FORCE_INSTALL_DIR ${enabled})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_runtime_deps enabled)
    set(MODULES_GLOBAL_INSTALL_RUNTIME_DEPS ${enabled} )
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_runtime_artifacts enabled)
    set(MODULES_GLOBAL_INSTALL_RUNTIME_ARTIFACTS ${enabled})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_exclude_so_libraries enabled)
    set(MODULES_GLOBAL_EXCLUDE_SO_LIBS ${enabled})
ENDMACRO()

# **********************************************************************************************************************
