# **********************************************************************************************************************
# Updated 15/04/2024
# **********************************************************************************************************************

# **********************************************************************************************************************

macro(macro_search_file file_name current_path result_var)

    # Initial search for the file in the current directory
    file(GLOB_RECURSE found_files RELATIVE "${current_path}" "${current_path}/*/${file_name}")
    if(found_files)
    else()
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

# Find the ZMQ library.

# Log.
message(STATUS "Configuring LibZMQ...")

# Unset previous.
unset(LIBZMQ_INCLUDE_DIR CACHE)
unset(LIBZMQ_INCLUDE_DIR_BINDING CACHE)
unset(LIBZMQ_INCLUDE_DIR_BINDING_ADDON CACHE)
unset(LIBZMQ_LIBRARY CACHE)
unset(LIBSODIUM_LIBRARY CACHE)
unset(LIBZMQ_LIBRARY_PATH CACHE)
unset(LIBSODIUM_LIBRARY_PATH CACHE)
unset(LIBZMQ_FOUND CACHE)
unset(LIBZMQ_LIBRARIES CACHE)
unset(LIBZMQ_INCLUDE_DIRS CACHE)
unset(LIBZMQ_LIBRARY_DIRS CACHE)

# Find for Windows plattform.
if(WIN32)

    # GLOB for directories that contain 'LibZMQ' in their names under 'C:/'
    file(GLOB LIBZMQ_CANDIDATE_DIRS
        "../LibZMQ*"
        "../../LibZMQ*"
        "C:/LibZMQ*"
        "C:/Program Files/LibZMQ*"
        "C:/Program Files (x86)/LibZMQ*")

    # List of paths to search.
    set(HARDCODED_PATHS
        "C:/LibZMQ/include"               # In standard root folder.
        "C:/Program Files/LibZMQ"         # Standard installation.
        "C:/Program Files (x86)/LibZMQ"   # Standard installation.
        NO_DEFAULT_PATH)

    # Check if a forced search path is provided externally
    if(LIBZMQ_FORCED_SEARCH_PATHS)
        message(STATUS "Forced LibZMQ search path is set to: ${LIBZMQ_FORCED_SEARCH_PATHS}")
        set(SEARCH_PATHS ${LIBZMQ_FORCED_SEARCH_PATHS})
    else()
       # Combine both lists into one if forced path is not set
       set(SEARCH_PATHS ${LIBZMQ_CANDIDATE_DIRS} ${HARDCODED_PATHS})
    endif()

    # Search for includes.
    macro_search_file_in_paths("LibZMQ/zmq.h" "${SEARCH_PATHS}" LIBZMQ_INCLUDE_DIR FALSE)
    macro_search_file_in_paths("LibZMQ/zmq.hpp" "${SEARCH_PATHS}" LIBZMQ_INCLUDE_DIR_BINDING FALSE)
    macro_search_file_in_paths("LibZMQ/zmq_addon.hpp" "${SEARCH_PATHS}" LIBZMQ_INCLUDE_DIR_BINDING_ADDON FALSE)

    # Find the ZMQ library and libsodium.
    macro_search_file_in_paths("libzmq-v142-mt-4_3_4.dll" "${SEARCH_PATHS}" LIBZMQ_LIBRARY TRUE)
    macro_search_file_in_paths("libsodium.dll" "${SEARCH_PATHS}" LIBSODIUM_LIBRARY TRUE)
    macro_search_file_in_paths("libzmq-v142-mt-4_3_4.dll" "${SEARCH_PATHS}" LIBZMQ_LIBRARY_PATH FALSE)
    macro_search_file_in_paths("libsodium.dll" "${SEARCH_PATHS}" LIBSODIUM_LIBRARY_PATH FALSE)

    # Checks.
    if(LIBZMQ_INCLUDE_DIR AND LIBZMQ_LIBRARY AND LIBSODIUM_LIBRARY AND LIBZMQ_INCLUDE_DIR AND
            LIBZMQ_INCLUDE_DIR_BINDING AND LIBZMQ_INCLUDE_DIR_BINDING_ADDON)

        set(LIBZMQ_FOUND TRUE)
        set(LIBZMQ_LIBRARIES ${LIBZMQ_LIBRARY} ${LIBSODIUM_LIBRARY})
        set(LIBZMQ_LIBRARY_DIRS ${LIBZMQ_LIBRARY_PATH} ${LIBSODIUM_LIBRARY_PATH})
        set(LIBZMQ_INCLUDE_DIRS ${LIBZMQ_INCLUDE_DIR})

    else()
        message(FATAL_ERROR "  Could not find LibZMQ library.")
    endif()

else()

    # GLOB for directories that contain 'LibZMQ' in their names under 'C:/'
    file(GLOB LIBZMQ_CANDIDATE_DIRS
        "../LibZMQ*"
        "../../LibZMQ*"
        "~/LibZMQ*")

    # List of paths to search.
    set(HARDCODED_PATHS
        "/usr/include"
        "/usr/local"
        "/usr/lib"
        NO_DEFAULT_PATH)

    # Check if a forced search path is provided externally
    if(LIBZMQ_FORCED_SEARCH_PATHS)
        message(STATUS "Forced LibZMQ search path is set to: ${LIBZMQ_FORCED_SEARCH_PATHS}")
        set(SEARCH_PATHS ${LIBZMQ_FORCED_SEARCH_PATHS})
    else()
       # Combine both lists into one if forced path is not set
       set(SEARCH_PATHS ${LIBZMQ_CANDIDATE_DIRS} ${HARDCODED_PATHS})
    endif()

    # Search for includes.
    macro_search_file_in_paths("LibZMQ/zmq.h" "${SEARCH_PATHS}" LIBZMQ_INCLUDE_DIR FALSE)
    macro_search_file_in_paths("LibZMQ/zmq.hpp" "${SEARCH_PATHS}" LIBZMQ_INCLUDE_DIR_BINDING FALSE)
    macro_search_file_in_paths("LibZMQ/zmq_addon.hpp" "${SEARCH_PATHS}" LIBZMQ_INCLUDE_DIR_BINDING_ADDON FALSE)

    # Find the ZMQ library and libsodium.
    macro_search_file_in_paths("libzmq.so" "${SEARCH_PATHS}" LIBZMQ_LIBRARY TRUE)
    macro_search_file_in_paths("libzmq.so" "${SEARCH_PATHS}" LIBZMQ_LIBRARY_PATH FALSE)

    # Checks.
    if(LIBZMQ_INCLUDE_DIR AND LIBZMQ_LIBRARY AND LIBZMQ_INCLUDE_DIR AND
       LIBZMQ_INCLUDE_DIR_BINDING AND LIBZMQ_INCLUDE_DIR_BINDING_ADDON)

        set(LIBZMQ_FOUND TRUE)
        set(LIBZMQ_LIBRARIES ${LIBZMQ_LIBRARY})
        set(LIBZMQ_LIBRARY_DIRS ${LIBZMQ_LIBRARY_PATH})
        set(LIBZMQ_INCLUDE_DIRS ${LIBZMQ_INCLUDE_DIR})

    else()
        message(FATAL_ERROR "  Could not find LibZMQ library.")
    endif()

endif()

# Logs.
message(STATUS "  LibZMQ information:")
message(STATUS "    LibZMQ libraries: ${LIBZMQ_LIBRARIES}" )
message(STATUS "    LibZMQ libraries dir: ${LIBZMQ_LIBRARY_DIRS}" )
message(STATUS "    LibZMQ includes dir: ${LIBZMQ_INCLUDE_DIRS}" )

# Mark as advanced.
mark_as_advanced(LIBZMQ_INCLUDE_DIRS LIBZMQ_LIBRARIES)
