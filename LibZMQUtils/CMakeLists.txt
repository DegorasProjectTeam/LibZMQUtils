# **********************************************************************************************************************
#   LibZMQUtils (ZMQ Utilitites Library): A libre library with ZMQ related useful utilities.
#
#   Copyright (C) 2023 Degoras Project Team
#                      < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >
#                      < Jesús Relinque Madroñal >
#
#   This file is part of LibZMQUtils.
#
#   Licensed under the European Union Public License (EUPL), Version 1.2 or subsequent versions of the EUPL license
#   as soon they will be approved by the European Commission (IDABC).
#
#   This project is free software: you can redistribute it and/or modify it under the terms of the EUPL license as
#   published by the IDABC, either Version 1.2 or, at your option, any later version.
#
#   This project is distributed in the hope that it will be useful. Unless required by applicable law or agreed to in
#   writing, it is distributed on an "AS IS" basis, WITHOUT ANY WARRANTY OR CONDITIONS OF ANY KIND; without even the
#   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the EUPL license to check specific
#   language governing permissions and limitations and more details.
#
#   You should use this project in compliance with the EUPL license. You should have received a copy of the license
#   along with this project. If not, see the license at < https://eupl.eu/ >.
# **********************************************************************************************************************

# **********************************************************************************************************************
# Updated 27/09/2023
# **********************************************************************************************************************

# Cmake minimum.
cmake_minimum_required(VERSION 3.21)

# Check version.
if (NOT ${CMAKE_VERSION} VERSION_LESS "3.1.0")
    cmake_policy(SET CMP0054 NEW)
endif()

#Configure the build type.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Library names.
set(LIB_FULL_NAME LibZMQUtils)
set(LIB_SHRT_NAME ZMQUtils)
set(LIB_ZMQ_UTILS ${LIB_FULL_NAME})

# Project name.
project(${LIB_FULL_NAME})

# Check the architecture.
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Architecture not supported by default.")
endif()

# Set path to additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH})
include(CMakeMacros)

# Set the version.
set(MAJOR_VERSION 1)
set(MINOR_VERSION 2)
set(PATCH_VERSION 4)
set(VERSION ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})

# Output directories.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# External dependencies.
set(ZMQ_DIR ${CMAKE_SOURCE_DIR}/external/zmq/)

# Other variables setting.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(SHOW_EXTERNALS TRUE)
set(LIB_DEPS_SET ${LIB_FULL_NAME}_deps)

# Define the install directory for each compiler and operating system.
macro_prepare_install_dir(INSTALL_BIN INSTALL_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../product)

# Includes.
include_directories(${CMAKE_SOURCE_DIR}/includes)
include_directories(${ZMQ_DIR}/includes)

# Get the headers, sources and template files.
file(GLOB_RECURSE HEADERS ${CMAKE_SOURCE_DIR}/includes/${LIB_FULL_NAME}/*.h)
file(GLOB_RECURSE TEMPLTS ${CMAKE_SOURCE_DIR}/includes/${LIB_FULL_NAME}/*.tpp)
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/sources/*.cpp)
if(SHOW_EXTERNALS)
    file(GLOB_RECURSE EXTERN ${ZMQ_DIR}/includes/zmq/*.h)
endif()

# Get the include aliases files.
macro_get_files_without_extension(ALIAS ${CMAKE_SOURCE_DIR}/includes/${LIB_FULL_NAME}/*)

# Create a custom group for the templates, aliases and externals.
source_group("Alias Files" FILES ${ALIAS})
source_group("Header Files" FILES ${TEMPLTS})
source_group("External Libs" FILES ${EXTERN})

# Add definitions and the library.
string(TOUPPER ${LIB_FULL_NAME} LIB_FULL_NAME_UPPER)
add_definitions(-D${LIB_FULL_NAME_UPPER}_LIBRARY)
add_library(${LIB_FULL_NAME} SHARED ${SOURCES} ${HEADERS} ${EXTERN} ${TEMPLTS} ${ALIAS}
    includes/LibZMQUtils/Global/constants.h)
target_compile_definitions(${LIB_FULL_NAME} PRIVATE -D${LIB_FULL_NAME_UPPER}_LIBRARY)

# Add the includes.
target_include_directories(${LIB_FULL_NAME} PRIVATE
                           ${CMAKE_SOURCE_DIR}/includes
                           ${ZMQ_DIR}/includes)

# Configure the compilers.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -O3 -fopenmp")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${LIB_FULL_NAME} PRIVATE /std:c++17 /openmp)
else()
    message(FATAL_ERROR "Compiler not supported by default.")
endif()

# In mingw better do static linking of the libgcc, libwinpthread and libstd.
if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static -lpthread")
    target_link_libraries(${LIB_FULL_NAME} PRIVATE -static-libgcc -static-libstdc++ -static -lpthread)
endif()

# Link libraries. For win32 only.
if (WIN32)

    set(ZMQ_LIBS_DIR ${ZMQ_DIR}/libs/x86_64)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

        target_link_libraries(${LIB_FULL_NAME} PUBLIC
                              ${ZMQ_LIBS_DIR}/libsodium.dll
                              ${ZMQ_LIBS_DIR}/libzmq-v142-mt-4_3_4.dll
                              ws2_32
                              iphlpapi)

    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

        target_link_libraries(${LIB_FULL_NAME} PUBLIC
                              ${ZMQ_LIBS_DIR}/libzmq-v142-mt-4_3_4.lib
                              ws2_32
                              iphlpapi)
    else()
        message(FATAL_ERROR "Compiler not supported by default.")
    endif()

endif()

# Add the diferents subprojects directories.
add_subdirectory(examples)
add_subdirectory(testing)

# Installation process for windows.
if(WIN32)

    # Install the include files of LibZMQUtils.
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/includes/${LIB_FULL_NAME}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/includes
            PATTERN "*.txt" EXCLUDE)

    # Install the include files of ZMQ.
    install(DIRECTORY ${ZMQ_DIR}/includes/zmq
            DESTINATION ${CMAKE_INSTALL_PREFIX}/includes)

    # Install the binaries  of LibZMQUtils to the specified install directory.
    install(TARGETS ${LIB_FULL_NAME}
            LIBRARY DESTINATION ${INSTALL_LIB}
            RUNTIME DESTINATION ${INSTALL_LIB})

    # Install the binaries of ZMQ to the specified install directory.
    install(DIRECTORY ${ZMQ_LIBS_DIR}/
            DESTINATION ${INSTALL_LIB}
            FILES_MATCHING PATTERN "*.dll")

    # Install the lib files for msvc.
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

        # ZMQ libs
        install(DIRECTORY ${ZMQ_LIBS_DIR}/
                DESTINATION ${INSTALL_LIB}
                FILES_MATCHING PATTERN "*.lib")

        # Install the LibZMQUtils.lib file to the specified library directory.
        install(FILES ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${LIB_FULL_NAME}.lib
                DESTINATION ${INSTALL_LIB})

    endif()

    # Runtime artifacts.
    install(IMPORTED_RUNTIME_ARTIFACTS ${LIB_FULL_NAME}
            RUNTIME_DEPENDENCY_SET ${LIB_DEPS_SET}
            DESTINATION ${INSTALL_BIN})

    # Install the external dependencies.
    install(RUNTIME_DEPENDENCY_SET ${LIB_DEPS_SET}
            PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
            DIRECTORIES ${ZMQ_LIBS_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            DESTINATION ${INSTALL_BIN})

endif()

# Enable testing.
enable_testing()

# **********************************************************************************************************************
