# **********************************************************************************************************************
# Updated 11/03/2024
# **********************************************************************************************************************

# **********************************************************************************************************************

# Note:Add all resources as last arguments.
MACRO(macro_setup_launcher launcher_name lib_opt lib_deb)

    # Combine all additional arguments into a single list of resources.
    set(resources ${ARGN})

    # Log.
    message(STATUS "Setup executable: ${launcher_name}")
    message(STATUS "  Build folder: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

    # Link the current dirs.
    if (WIN32)
        link_directories(${CMAKE_CURRENT_BINARY_DIR})
    elseif(UNIX)
        link_directories(${CMAKE_CURRENT_BINARY_DIR}/../lib)
    endif()

    # Add the executable.
    add_executable(${launcher_name} ${resources})

    # Link the external libraries.
    message(STATUS "  Linking optimized libraries: ${lib_opt}")
    message(STATUS "  Linking debug libraries: ${lib_deb}")
    target_link_libraries(${launcher_name} PRIVATE optimized "${lib_opt}" debug "${lib_deb}")

    # Set target properties.
    set_target_properties(${launcher_name} PROPERTIES PROJECT_LABEL "Launcher ${launcher_name}")
    set_target_properties(${launcher_name} PROPERTIES DEBUG_OUTPUT_NAME "${launcher_name}${CMAKE_DEBUG_POSTFIX}")
    set_target_properties(${launcher_name} PROPERTIES RELEASE_OUTPUT_NAME "${launcher_name}${CMAKE_RELEASE_POSTFIX}")
    set_target_properties(${launcher_name} PROPERTIES RELWITHDEBINFO_OUTPUT_NAME "${launcher_name}${CMAKE_RELWITHDEBINFO_POSTFIX}")
    set_target_properties(${launcher_name} PROPERTIES MINSIZEREL_OUTPUT_NAME "${launcher_name}${CMAKE_MINSIZEREL_POSTFIX}")
    set_target_properties(${launcher_name} PROPERTIES FOLDER CMAKE_RUNTIME_OUTPUT_DIRECTORY)

ENDMACRO()

# **********************************************************************************************************************
