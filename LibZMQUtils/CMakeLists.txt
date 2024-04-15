# **********************************************************************************************************************
#    LibZMQUtils (Degoras Project SLR Library).
#
#    A modern and efficient C++ base library for Satellite Laser Ranging (SLR) software and real-time hardware
#    related developments. Developed as a free software under the context of Degoras Project for the Spanish Navy
#    Observatory SLR station (SFEL) in San Fernando and, of course, for any other station that wants to use it!
#
#    Copyright (C) 2024 Degoras Project Team
#                       < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >
#                       < Jesús Relinque Madroñal >
#
#    This file is part of LibZMQUtils.
#
#    Licensed under the European Union Public License (EUPL), Version 1.2 or subsequent versions of the EUPL license
#    as soon they will be approved by the European Commission (IDABC).
#
#    This project is free software: you can redistribute it and/or modify it under the terms of the EUPL license as
#    published by the IDABC, either Version 1.2 or, at your option, any later version.
#
#    This project is distributed in the hope that it will be useful. Unless required by applicable law or agreed to in
#    writing, it is distributed on an "AS IS" basis, WITHOUT ANY WARRANTY OR CONDITIONS OF ANY KIND; without even the
#    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the EUPL license to check specific
#    language governing permissions and limitations and more details.
#
#    You should use this project in compliance with the EUPL license. You should have received a copy of the license
#    along with this project. If not, see the license at < https://eupl.eu/ >.
# **********************************************************************************************************************

# **********************************************************************************************************************
# Updated 11/03/2024
# **********************************************************************************************************************

# **********************************************************************************************************************
# LIBDEGORASSLR MAIN SHARED LIBRARY PROJECT CMAKELIST
# **********************************************************************************************************************

# ----------------------------------------------------------------------------------------------------------------------
# INITIALIZATION

# Cmake minimum and empty project for avoid warnings.
cmake_minimum_required(VERSION 3.21)
project(_)

# Set path to additional CMake modules.
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/CMakeModules
    ${CMAKE_SOURCE_DIR}/CMakeLibsConfig)

# Init CMake modules.
include(CMakeMacrosGlobals)
macro_cmakemodules_init()

# ----------------------------------------------------------------------------------------------------------------------
# BASIC PROJECT CONFIGURATION

# Base project configuration.
set(LIB_NAME LibZMQUtils)
set(LIB_VER 1.5.1)
set(LIB_BUILD_MODE Debug)
set(LIB_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../${LIB_NAME}_product_v${LIB_VER})
set(LIB_LANGUAGES CXX C)

# Macros global configuration.
macro_global_set_show_externals(TRUE)
macro_global_set_force_install_dir(TRUE)
macro_global_set_install_runtime_deps(TRUE)
macro_global_set_install_runtime_artifacts(TRUE)
macro_global_set_exclude_so_libraries(TRUE)

# Setup the base project.
macro_setup_base_project(${LIB_NAME} ${LIB_VER} ${LIB_BUILD_MODE} ${LIB_LANGUAGES})

# Other configurations.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "")

# Setup the install directory for each compiler and operating system.
macro_setup_default_install_dir(${LIB_INSTALL_DIR})

# ----------------------------------------------------------------------------------------------------------------------
# SPECIFIC PROJECT CONFIGURATION

# Specific configuration.
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
set(LIB_INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes/)
set(LIB_SOURCES_DIR ${CMAKE_SOURCE_DIR}/sources)

# Configure the ZMQ package.


# ----------------------------------------------------------------------------------------------------------------------
# SETUP THE SHARED LIBRARY

# Get the headers, sources and template files.
file(GLOB_RECURSE HEADERS ${CMAKE_SOURCE_DIR}/includes/${LIB_NAME}/*.h)
file(GLOB_RECURSE TEMPLTS ${CMAKE_SOURCE_DIR}/includes/${LIB_NAME}/*.tpp)
file(GLOB_RECURSE INL ${CMAKE_SOURCE_DIR}/includes/${LIB_NAME}/*.inl)
file(GLOB_RECURSE SOURCES1 ${CMAKE_SOURCE_DIR}/sources/*.cpp)
file(GLOB_RECURSE SOURCES2 ${CMAKE_SOURCE_DIR}/sources/AlgorithmsSLR/*.cpp)

# Get the include aliases files.
macro_get_files_without_extension(ALIAS ${CMAKE_SOURCE_DIR}/includes/${LIB_NAME}/*)

# Create a custom group for the templates and aliases.
source_group("Header Files" FILES ${TEMPLTS})
source_group("Alias Files" FILES ${ALIAS})

# Setup the shared library.
macro_setup_shared_lib("${LIB_NAME}" "${LIB_INCLUDES_DIR}" "${LIB_VER}"
                       ${SOURCES1} ${SOURCES2} ${HEADERS} ${TEMPLTS} ${INL} ${ALIAS})



# REMODELAR ================================================
# Link with ZMQ.
# External dependencies.
set(ZMQ_DIR ${CMAKE_SOURCE_DIR}/external/zmq/)
target_include_directories(${LIB_NAME} PRIVATE
                           ${ZMQ_DIR}/includes)
include_directories(${ZMQ_DIR}/includes)
# Link libraries. For win32 only.
if (WIN32)

    set(ZMQ_LIBS_DIR ${ZMQ_DIR}/libs/windows)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

        target_link_libraries(${LIB_NAME} PUBLIC
                              ${ZMQ_LIBS_DIR}/libsodium.dll
                              ${ZMQ_LIBS_DIR}/libzmq-v142-mt-4_3_4.dll
                              ws2_32
                              iphlpapi)
   else()
        message(FATAL_ERROR "Compiler not supported by default.")
    endif()

elseif(UNIX)
    set(ZMQ_LIBS_DIR ${ZMQ_DIR}/libs/unix)

    target_link_libraries(${LIB_FULL_NAME} PUBLIC
                          ${ZMQ_LIBS_DIR}/libzmq.so)
endif()
# ================================================





# Include the shared library CMake configuration file.
include(ConfigLibZMQUtils)

# ----------------------------------------------------------------------------------------------------------------------
# CONFIGURE THE COMPILERS

# Configure the compilers.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wpedantic -Wall -Wextra -O0 -fopenmp")
    else()
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -m64 -O3 -pthread -fopenmp")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    #target_compile_options(${LIB_NAME} PUBLIC)
    message(FATAL_ERROR "Compiler not supported by default.")
else()
    message(FATAL_ERROR "Compiler not supported by default.")
endif()

# In mingw better do static linking of the libgcc, libwinpthread and libstd.
if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static -lpthread")
    target_link_libraries(${LIB_NAME} PRIVATE -static-libgcc -static-libstdc++ -static -lpthread)
endif()

# ----------------------------------------------------------------------------------------------------------------------
# INSTALLATION PROCESS

# External libs locations.
#set(ext_libs_loc ${LibNovasCpp_LOCATION_DIR})

# Installation process.
macro_default_library_installation("${LIB_NAME}" "${LIB_INCLUDES_DIR}" "${ext_libs_loc}")

# ----------------------------------------------------------------------------------------------------------------------
# CONFIGURE THE SUBPROJECTS

# Testing subproject.
#add_subdirectory(testing)

# Example subproject.
add_subdirectory(examples)

# **********************************************************************************************************************
