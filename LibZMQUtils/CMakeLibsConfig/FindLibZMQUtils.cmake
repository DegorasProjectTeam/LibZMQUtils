# **********************************************************************************************************************
# Updated 15/04/2024
# **********************************************************************************************************************

# **********************************************************************************************************************

macro(macro_search_file file_name current_path result_var)

    # Initial search for the file in the current directory
    file(GLOB_RECURSE found_files RELATIVE "${current_path}" "${current_path}/*/${file_name}")
    if(found_files)
    else()
        message(STATUS "  Internal search in: ${current_path}/${file_name}")
        file(GLOB_RECURSE found_files RELATIVE "${current_path}" "${current_path}/${file_name}")
    endif()

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
        if(actual_file_name MATCHES "^/")
            string(SUBSTRING ${actual_file_name} 1 -1 actual_file_name)
        endif()
        message(STATUS "  Search for: ${actual_file_name}")
    else()
        set(sub_path "")
        set(actual_file_name "${file_name}")
        message(STATUS "  Search for: ${file_name}")
    endif()

    set(local_result)
    foreach(dir ${paths})
        if(IS_DIRECTORY ${dir})
            macro_search_file("${actual_file_name}" "${dir}" local_result)
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

# Find the ZMQUtils library.

# Log.
message(STATUS "Configuring LibZMQUtils...")

# Unset previous.
unset(LIBZMQUTILS_INCLUDE_DIR CACHE)
unset(LIBZMQUTILS_LIBRARY CACHE)
unset(LIBZMQUTILS_LIBRARY_PATH CACHE)
unset(LIBZMQUTILS_FOUND CACHE)
unset(LIBZMQUTILS_LIBRARIES CACHE)
unset(LIBZMQUTILS_INCLUDE_DIRS CACHE)
unset(LIBZMQUTILS_LIBRARY_DIRS CACHE)

# Find for Windows plattform.
if(WIN32)

    # GLOB for directories that contain 'LibZMQUtils' in their names under 'C:/'
    file(GLOB LIBZMQUTILS_CANDIDATE_DIRS
        "../LibZMQUtils*"
        "../../LibZMQUtils*"
        "C:/LibZMQUtils*"
        "C:/Program Files/LibZMQUtils*"
        "C:/Program Files (x86)/LibZMQUtils*")

    # List of paths to search.
    set(HARDCODED_PATHS
        "C:/LibZMQUtils/include"               # In standard root folder.
        "C:/Program Files/LibZMQUtils"         # Standard installation.
        "C:/Program Files (x86)/LibZMQUtils"   # Standard installation.
        NO_DEFAULT_PATH)

    # Check if a forced search path is provided externally
    if(LIBZMQUTILS_FORCED_SEARCH_PATHS)
        message(STATUS "Forced LibZMQUtils search path is set to: ${LIBZMQUTILS_FORCED_SEARCH_PATHS}")
        set(SEARCH_PATHS ${LIBZMQUTILS_FORCED_SEARCH_PATHS})
    else()
       # Combine both lists into one if forced path is not set
       set(SEARCH_PATHS ${LIBZMQUTILS_CANDIDATE_DIRS} ${HARDCODED_PATHS})
    endif()

    # Find the ZMQUtils library.
    macro_search_file_in_paths("libZMQUtils.dll" "${SEARCH_PATHS}" LIBZMQUTILS_LIBRARY TRUE)
    macro_search_file_in_paths("libZMQUtils.dll" "${SEARCH_PATHS}" LIBZMQUTILS_LIBRARY_PATH FALSE)
	
	set(LIBZMQUTILS_INCLUDE_DIR "${LIBZMQUTILS_LIBRARY_PATH}/../include"}

    # Checks.
    if(LIBZMQUTILS_INCLUDE_DIR AND LIBZMQUTILS_LIBRARY)

        set(LIBZMQUTILS_FOUND TRUE)
        set(LIBZMQUTILS_LIBRARIES ${LIBZMQUTILS_LIBRARY})
        set(LIBZMQUTILS_LIBRARY_DIRS ${LIBZMQUTILS_LIBRARY_PATH})
        set(LIBZMQUTILS_INCLUDE_DIRS ${LIBZMQUTILS_INCLUDE_DIR})

    else()
        message(FATAL_ERROR "  Could not find LibZMQUtils library.")
    endif()

else()

    message(FATAL_ERROR "  SO not supported." )

endif()

# Logs.
message(STATUS "  LibZMQUtils information:")
message(STATUS "    LibZMQUtils libraries: ${LIBZMQUTILS_LIBRARIES}" )
message(STATUS "    LibZMQUtils libraries dir: ${LIBZMQUTILS_LIBRARY_DIRS}" )
message(STATUS "    LibZMQUtils includes dir: ${LIBZMQUTILS_INCLUDE_DIRS}" )

# Mark as advanced.
mark_as_advanced(LIBZMQUTILS_INCLUDE_DIRS LIBZMQUTILS_LIBRARIES)
