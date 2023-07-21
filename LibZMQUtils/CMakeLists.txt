# Cmake minimum.
cmake_minimum_required(VERSION 2.9)

if (NOT ${CMAKE_VERSION} VERSION_LESS "3.1.0")
    CMAKE_POLICY(SET CMP0054 NEW)
endif()

#Configure the build type.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Configure the version.
set(MAJOR_VERSION 1)
set(MINOR_VERSION 0)
set(PATCH_VERSION 0)
set(VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})

# Project name.
project(LibZMQUtils)

# Other configs.
set(SHORT_NAME ZMQUtils)
set(CMAKE_COLOR_MAKEFILE ON)

# Set diferent paths for build and deployment.
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -O3")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})
include(CMakeMacros)

# Get the include aliases files.
macro_get_files_without_extension(ALIAS ${CMAKE_SOURCE_DIR}/includes/LibZMQUtils/*)

# Create a custom group for the templates and aliases.
source_group("Alias Files" FILES ${ALIAS})

# Get the headers, sources and template files.
FILE(GLOB_RECURSE HEADERS ${CMAKE_SOURCE_DIR}/includes/LibZMQUtils/*.h)
FILE(GLOB_RECURSE TEMPLTS ${CMAKE_SOURCE_DIR}/includes/LibZMQUtils/*.tpp)
FILE(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/sources/*.cpp)

include_directories(${CMAKE_SOURCE_DIR}/includes)
include_directories("external/zmq/includes")

# Add definitions and the library.
add_definitions(-DLIBZMQUTILS_LIBRARY)
add_library(LibZMQUtils SHARED ${SOURCES} ${HEADERS} ${ALIAS})

# Link libraries. For win32 only.
if (WIN32)
    target_link_libraries(LibZMQUtils PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/zmq/libs/x86_64/libsodium.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/external/zmq/libs/x86_64/libzmq-v142-mt-4_3_4.dll
        ws2_32
        iphlpapi
    )
endif()

# Add the diferents subprojects directories.
add_subdirectory(examples)
