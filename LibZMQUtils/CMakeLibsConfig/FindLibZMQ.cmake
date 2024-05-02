# **********************************************************************************************************************
# Updated 24/04/2024
# **********************************************************************************************************************

# **********************************************************************************************************************

# Unset previous final values.
# You can use this values in your CMakeLists once setted.
# --
unset(LIBZMQ_FOUND CACHE)          # Library found flag.
unset(LIBZMQ_LIBRARIES CACHE)      # List of libraries (full path with names).
unset(LIBZMQ_INCLUDES CACHE)       # List of includes (full path with names).
unset(LIBZMQ_LIBRARY_DIRS CACHE)   # List of libraries directories (without names).
unset(LIBZMQ_INCLUDE_DIRS CACHE)   # List of includes directories (without names).

# Unset previous auxiliar values.
# --
unset(LIBZMQ_INCLUDE_PATH CACHE)
unset(LIBZMQ_BINDING_PATH CACHE)
unset(LIBZMQ_BINDINGADDON_PATH CACHE)
unset(LIBZMQ_INCLUDE_FILE CACHE)
unset(LIBZMQ_BINDING_FILE CACHE)
unset(LIBZMQ_BINDINGADDON_FILE CACHE)
unset(LIBZMQ_LIBRARY_PATH CACHE)
unset(LIBSODIUM_LIBRARY_PATH CACHE)
unset(LIBZMQ_LIBRARY_FILE CACHE)
unset(LIBSODIUM_LIBRARY_FILE CACHE)
# **********************************************************************************************************************

macro(macro_search_file_private file_name current_path result_var)

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
                macro_search_file_private(${file_name} "${current_path}/${child}" ${result_var})
                if(${result_var})
                    break()  # Stop if the file has been found
                endif()
            endif()
        endforeach()
    endif()
endmacro()

# **********************************************************************************************************************

macro(macro_search_file_in_paths_private file_name paths result_var_path result_var_file)
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
            macro_search_file_private("${actual_file_name}" "${dir}" local_result)
        endif()
        if(local_result)
            string(REGEX REPLACE "/$" "" local_result "${local_result}")
            set(${result_var_file} "${local_result}/${file_name}")
            # Determine if the sub_path should be removed from the result
            string(REGEX REPLACE "/${sub_path}$" "" trimmed_path "${local_result}")
            set(${result_var_path} ${trimmed_path})  # Just set the directory path
            break()  # Stop if the file has been found
        endif()
    endforeach()
endmacro()

# **********************************************************************************************************************

# Find the ZMQ library.

# Log.
message(STATUS "Configuring LibZMQ...")

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
    macro_search_file_in_paths_private("zmq.h" "${SEARCH_PATHS}" LIBZMQ_INCLUDE_PATH LIBZMQ_INCLUDE_FILE)
    macro_search_file_in_paths_private("zmq.hpp" "${SEARCH_PATHS}" LIBZMQ_BINDING_PATH LIBZMQ_BINDING_FILE)
    macro_search_file_in_paths_private("zmq_addon.hpp" "${SEARCH_PATHS}" LIBZMQ_BINDINGADDON_PATH LIBZMQ_BINDINGADDON_FILE)

    # Find the ZMQ library and libsodium.
    macro_search_file_in_paths_private("libzmq-v142-mt-4_3_4.dll" "${SEARCH_PATHS}" LIBZMQ_LIBRARY_PATH LIBZMQ_LIBRARY_FILE)
    macro_search_file_in_paths_private("libsodium.dll" "${SEARCH_PATHS}" LIBSODIUM_LIBRARY_PATH LIBSODIUM_LIBRARY_FILE)

    # Checks.
    if(LIBZMQ_INCLUDE_FILE AND LIBZMQ_BINDING_FILE AND LIBZMQ_BINDINGADDON_FILE AND LIBZMQ_LIBRARY_FILE AND LIBSODIUM_LIBRARY_FILE)

        set(LIBZMQ_FOUND TRUE)
        set(LIBZMQ_LIBRARIES ${LIBZMQ_LIBRARY_FILE} ${LIBSODIUM_LIBRARY_FILE})
        set(LIBZMQ_LIBRARY_DIRS ${LIBZMQ_LIBRARY_PATH} ${LIBSODIUM_LIBRARY_PATH})
        set(LIBZMQ_INCLUDES ${LIBZMQ_INCLUDE_FILE} ${LIBZMQ_BINDING_FILE} ${LIBZMQ_BINDINGADDON_FILE})
        set(LIBZMQ_INCLUDE_DIRS ${LIBZMQ_INCLUDE_PATH} ${LIBZMQ_BINDING_PATH} ${LIBZMQ_BINDINGADDON_PATH})

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
    macro_search_file_in_paths_private("zmq.h" "${SEARCH_PATHS}" LIBZMQ_INCLUDE_PATH LIBZMQ_INCLUDE_FILE)
    macro_search_file_in_paths_private("zmq.hpp" "${SEARCH_PATHS}" LIBZMQ_BINDING_PATH LIBZMQ_BINDING_FILE)
    macro_search_file_in_paths_private("zmq_addon.hpp" "${SEARCH_PATHS}" LIBZMQ_BINDINGADDON_PATH LIBZMQ_BINDINGADDON_FILE)

    # Find the ZMQ library.
    macro_search_file_in_paths_private("libzmq.so" "${SEARCH_PATHS}" LIBZMQ_LIBRARY_PATH LIBZMQ_LIBRARY_FILE)

    # Checks.
    if(LIBZMQ_INCLUDE_FILE AND LIBZMQ_BINDING_FILE AND LIBZMQ_BINDINGADDON_FILE AND LIBZMQ_LIBRARY_FILE)

        set(LIBZMQ_FOUND TRUE)
        set(LIBZMQ_LIBRARIES ${LIBZMQ_LIBRARY_FILE})
        set(LIBZMQ_LIBRARY_DIRS ${LIBZMQ_LIBRARY_PATH})
        set(LIBZMQ_INCLUDES ${LIBZMQ_INCLUDE_FILE} ${LIBZMQ_BINDING_FILE} ${LIBZMQ_BINDINGADDON_FILE})
        set(LIBZMQ_INCLUDE_DIRS ${LIBZMQ_INCLUDE_PATH} ${LIBZMQ_BINDING_PATH} ${LIBZMQ_BINDINGADDON_PATH})

    else()
        message(FATAL_ERROR "  Could not find LibZMQ library.")
    endif()

endif()

# Logs.
message(STATUS "  LibZMQ information:")
message(STATUS "    LibZMQ found: ${LIBZMQ_FOUND}")
message(STATUS "    LibZMQ libraries: ${LIBZMQ_LIBRARIES}" )
message(STATUS "    LibZMQ libraries dirs: ${LIBZMQ_LIBRARY_DIRS}")
message(STATUS "    LibZMQ includes: ${LIBZMQ_INCLUDES}" )
message(STATUS "    LibZMQ includes dirs: ${LIBZMQ_INCLUDE_DIRS}")

# Mark as advanced.
mark_as_advanced(LIBZMQ_LIBRARIES LIBZMQ_INCLUDES LIBZMQ_LIBRARY_DIRS LIBZMQ_INCLUDE_DIRS)

# **********************************************************************************************************************
