# **********************************************************************************************************************
# Updated 11/03/2024
# **********************************************************************************************************************

# **********************************************************************************************************************

# Note: Add all resources as last arguments.
MACRO(macro_setup_shared_lib lib_name lib_includes_dir lib_version)

    # Combine all additional arguments into a single list of resources.
    set(resources ${ARGN})

    # Add definitions and the library.
    string(TOUPPER ${lib_name} LIB_NAME_UPPER)
    add_library(${lib_name} SHARED ${resources})
    set_target_properties(${lib_name} PROPERTIES VERSION ${lib_version})
    target_compile_definitions(${lib_name} PRIVATE -D${LIB_NAME_UPPER}_LIBRARY)
    include_directories(${lib_includes_dir})

    # Log.
    get_target_property(extracted_version ${lib_name} VERSION)
    message(STATUS "Setup shared library: ${lib_name}, Version: ${extracted_version}")

   # Append the new library to global.
   if(CMAKE_BUILD_TYPE STREQUAL "Debug")
       macro_global_add_libs_debug("${lib_name}")
   else()
       macro_global_add_libs_optimized("${lib_name}")
   endif()

   # Define the global dependency set name.
   macro_global_set_main_dep_set_name("${lib_name}_deps")

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_setup_lib_basic_examples examples_sources_path examples_install_path)

    # Log.
    message(STATUS "Setup library examples...")

    # Configure build path.
    set(APP_BUILD_FOLDER ${CMAKE_BINARY_DIR}/bin/Examples/)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_BUILD_FOLDER})

    # List of basic tests.
    #file(GLOB EXAMPLE_SOURCES "${examples_sources_path}/*.cpp")

    # List of basic tests.
    file(GLOB_RECURSE EXAMPLE_SOURCES RELATIVE "${examples_sources_path}" "${examples_sources_path}/*.cpp")

    # Loop through the example names and configure each basic example.
    foreach(EXAMPLE_SOURCE_FILE ${EXAMPLE_SOURCES})

        # Get the example name and source.
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE_FILE} NAME_WE)
        set(SOURCES ${examples_sources_path}/${EXAMPLE_SOURCE_FILE})

        # Setup the launcher.
        macro_setup_launcher("${EXAMPLE_NAME}"
                             "${MODULES_GLOBAL_LIBS_OPTIMIZED}"
                             "${MODULES_GLOBAL_LIBS_DEBUG}"
                             "${SOURCES}")

        # Include directories for the target.
        target_include_directories(${EXAMPLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/includes)

        # Install the launcher.
        macro_install_launcher(${EXAMPLE_NAME} ${examples_install_path})

        # Install runtime artifacts.
        macro_install_runtime_artifacts(${EXAMPLE_NAME}
                                        ${MODULES_GLOBAL_MAIN_DEP_SET_NAME}
                                        ${examples_install_path})

        # Install the runtime dependencies.
        macro_install_runtime_deps("${EXAMPLE_NAME}"
                                   "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                                   "${CMAKE_BINARY_DIR}/bin"
                                   "${examples_install_path}"
                                   "" "")

    endforeach()

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_setup_lib_basic_unit_tests tests_sources_path install_path ignore_paths)

    # Log.
    message(STATUS "Setup library unit tests...")

    # Configure build path.
    set(APP_BUILD_FOLDER ${CMAKE_BINARY_DIR}/bin/Tests/)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_BUILD_FOLDER})

    # Initialize an empty list for the final test sources.
    set(FILTERED_TESTS_SOURCES)

    # List of basic tests.
    file(GLOB_RECURSE TESTS_SOURCES RELATIVE "${tests_sources_path}" "${tests_sources_path}/*.cpp")

    # For simple test we will avoit the include the external resources.
    set(EXTERN)

    # Filter out ignored paths.
    foreach(SOURCE_PATH ${TESTS_SOURCES})

        message(STATUS "  Checking source: ${SOURCE_PATH}")

        set(IGNORE_FILE FALSE)
        foreach(IGNORE_PATH ${ignore_paths})
            if("${SOURCE_PATH}" MATCHES "${IGNORE_PATH}")
                set(IGNORE_FILE TRUE)
                message(STATUS "  Ignore path: ${SOURCE_PATH}")
            endif()
        endforeach()
        if(NOT IGNORE_FILE)
            list(APPEND FILTERED_TESTS_SOURCES ${SOURCE_PATH})
        endif()

    endforeach()

    # Prepare the external library search folders.
    set(ext_libs_loc
        ${MODULES_GLOBAL_INSTALL_LIB_PATH}
        ${MODULES_GLOBAL_INSTALL_BIN_PATH}
        ${CMAKE_BINARY_DIR}/bin)

    # Loop through the test names and configure each basic test.
    foreach(TESTS_SOURCE_FILE ${FILTERED_TESTS_SOURCES})

        # Get the test name and source.
        get_filename_component(TEST_NAME ${TESTS_SOURCE_FILE} NAME_WE)
        set(SOURCES ${tests_sources_path}/${TESTS_SOURCE_FILE})

        # Setup the launcher.
        macro_setup_launcher("${TEST_NAME}"
                             "${MODULES_GLOBAL_LIBS_OPTIMIZED}"
                             "${MODULES_GLOBAL_LIBS_DEBUG}"
                             "${SOURCES}")

        # Install the launcher.
        macro_install_launcher(${TEST_NAME} ${install_path})

        # Install runtime artifacts.
        macro_install_runtime_artifacts(${TEST_NAME}
                                        ${MODULES_GLOBAL_MAIN_DEP_SET_NAME}
                                        ${install_path})

        # Install the runtime dependencies.
        macro_install_runtime_deps("${EXAMPLE_NAME}"
                                   "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                                   "${ext_libs_loc}"
                                   "${install_path}"
                                   "" "")

    endforeach()

ENDMACRO()

# **********************************************************************************************************************
