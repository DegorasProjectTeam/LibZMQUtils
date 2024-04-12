# **********************************************************************************************************************
# Updated 11/03/2024
# **********************************************************************************************************************

# **********************************************************************************************************************

MACRO(macro_get_files_without_extension result curdir)
    file(GLOB_RECURSE FILES ${curdir})
    foreach(FILE ${FILES})
        get_filename_component(FILENAME ${FILE} NAME)
        if(${FILENAME} MATCHES "^[^.]+$")
            list(APPEND FILELIST ${FILE})
        endif()
    endforeach()
    set(${result} ${FILELIST})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_subdir_list result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
        list(APPEND dirlist ${child})
    endif()
    endforeach()
    set(${result} ${dirlist})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_add_subdirs curdir)
    macro_subdir_list(subdirs ${curdir})
    foreach(subdir ${subdirs})
        # Check if CMakeLists.txt exists in the subdir.
        if(EXISTS "${curdir}/${subdir}/CMakeLists.txt")
            add_subdirectory(${subdir})
        endif()
    endforeach()
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_add_subdirs_recursive curdir ignore_paths)
    # Get a list of all subdirectories in the current directory
    macro_subdir_list(subdirs ${curdir})

    foreach(subdir ${subdirs})
        # Full path of the current subdir
        set(full_subdir_path "${curdir}/${subdir}")

        # Calculate relative path from the base directory (CMAKE_CURRENT_SOURCE_DIR)
        file(RELATIVE_PATH relative_subdir_path "${CMAKE_CURRENT_SOURCE_DIR}" ${full_subdir_path})

        # Check if current subdir's relative path is in the ignore list
        set(IGNORE_SUBDIR FALSE)
        foreach(IGNORE_PATH ${ignore_paths})
            # Check if the relative subdir path matches the ignore pattern
            if("${relative_subdir_path}" MATCHES "${IGNORE_PATH}")
                set(IGNORE_SUBDIR TRUE)
                message(STATUS "Ignoring subdir: ${relative_subdir_path}")
                break()  # Exit the inner loop if we find a match
            endif()
        endforeach()

        if(NOT IGNORE_SUBDIR)
            # Check if CMakeLists.txt exists in the current subdir
            if(EXISTS "${full_subdir_path}/CMakeLists.txt")
                add_subdirectory(${full_subdir_path})
            endif()

            # Recursively call this macro for each subdir, regardless of CMakeLists.txt existence
            macro_add_subdirs_recursive("${full_subdir_path}" "${ignore_paths}")
        endif()
    endforeach()
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_compose_current_architecture_folder_name result_var)

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

# **********************************************************************************************************************
