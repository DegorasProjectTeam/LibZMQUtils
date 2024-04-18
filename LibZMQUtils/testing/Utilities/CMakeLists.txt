# **********************************************************************************************************************
# LIBZMQUTILS TESTS CMAKELIST
# **********************************************************************************************************************

# ----------------------------------------------------------------------------------------------------------------------
# CONFIGURATION

# Configure the compilers.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wpedantic -Wall -Wextra -O0 -fopenmp")
    else()
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -m64 -O3 -pthread -fopenmp")
    endif()
else()
    message(FATAL_ERROR "Compiler not supported by default.")
endif()

# Add the tests in the subdirectories.
# Setup the basic module unit tests.
macro_setup_lib_basic_unit_tests("${CMAKE_CURRENT_SOURCE_DIR}"
                                 "${GLOBAL_LIBZMQUTILS_TESTS_INSTALL_PATH}"
                                 "${IGNORED_PATHS}")



# **********************************************************************************************************************
