# **********************************************************************************************************************
# Updated 15/03/2024
# **********************************************************************************************************************

macro(macro_search_file file_name current_path result_var)
    # Initial search for the file in the current directory
    file(GLOB_RECURSE found_files RELATIVE "${current_path}" "${current_path}/*/${file_name}")

    if(found_files)
        list(GET found_files 0 first_found_file)
        get_filename_component(first_found_dir "${first_found_file}" DIRECTORY)
        set(${result_var} "${current_path}/${first_found_dir}")
    else()
        # Recursively search in subdirectories
        file(GLOB children RELATIVE "${current_path}" "${current_path}/*")
        foreach(child IN LISTS children)
            if(IS_DIRECTORY "${current_path}/${child}")
                macro_search_file(${file_name} "${current_path}/${child}" ${result_var})
                if(${result_var})
                    break()  # Stop if the file has been found
                endif()
            endif()
        endforeach()
    endif()
endmacro()

# **********************************************************************************************************************

macro(macro_search_file_in_paths file_name paths result_var append_filename)
    # Split file_name into directory and the actual file name
    string(FIND "${file_name}" "/" last_slash REVERSE)
    if(last_slash GREATER -1)
        string(SUBSTRING "${file_name}" 0 ${last_slash} sub_path)
        string(SUBSTRING "${file_name}" ${last_slash} -1 actual_file_name)
    else()
        set(sub_path "")
        set(actual_file_name "${file_name}")
    endif()

    set(local_result)
    foreach(dir ${paths})
        if(IS_DIRECTORY ${dir})
            macro_search_file("${file_name}" "${dir}" local_result)
        endif()
        if(local_result)
            # Check if the filename should be appended to the result
            if(${append_filename})
                set(${result_var} "${local_result}/${file_name}")  # Append filename to path
            else()
                # Determine if the sub_path should be removed from the result
                string(REGEX REPLACE "/${sub_path}$" "" trimmed_path "${local_result}")
                set(${result_var} ${trimmed_path})  # Just set the directory path
            endif()
            break()  # Stop if the file has been found
        endif()
    endforeach()
endmacro()

# **********************************************************************************************************************

MACRO(macro_search_file_in_folder_list file_name paths result)

    message(STATUS "HERE: ${paths}" )

    # Ensure paths are treated as a list
    set(path_list "${paths}")
    if(NOT "${paths}" MATCHES ";")
        set(path_list "${paths}")
    endif()

    foreach(dir IN LISTS path_list)
        message(STATUS "    Candidate: ${dir}" )

         if(IS_DIRECTORY ${dir})
             macro_search_file(${file_name} ${dir} result)
         endif()
         if(result)
             message(STATUS "    Found 1: ${result}" )
             break()  # Stop if the file has been found
         endif()
     endforeach()

ENDMACRO()

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
