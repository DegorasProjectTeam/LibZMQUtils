# **********************************************************************************************************************
# Updated 15/04/2024
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
        message(STATUS "Forced LibZMQ search path is set to: ${LIBZMQ_FORCED_SEARCH_PATH}")
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

    message(FATAL_ERROR "  SO not supported." )

endif()

# Logs.
message(STATUS "  LibZMQ information:")
message(STATUS "    LibZMQ libraries: ${LIBZMQ_LIBRARIES}" )
message(STATUS "    LibZMQ libraries dir: ${LIBZMQ_LIBRARY_DIRS}" )
message(STATUS "    LibZMQ includes dir: ${LIBZMQ_INCLUDE_DIRS}" )

# Mark as advanced.
mark_as_advanced(LIBZMQ_INCLUDE_DIRS LIBZMQ_LIBRARIES)
