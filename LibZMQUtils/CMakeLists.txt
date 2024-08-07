# **********************************************************************************************************************
#    LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
#                                                                                                                      *
#    A modern open-source and cross-platform C++ library with high-level utilities based on the well-known ZeroMQ      *
#    open-source universal messaging library. Includes a suite of modules that encapsulates the ZMQ communication      *
#    patterns as well as automatic binary serialization capabilities, specially designed for system infraestructure.   *
#    The library is suited for the quick and easy integration of new and old systems and can be used in different      *
#    sectors and disciplines seeking robust messaging and serialization solutions.                                     *
#                                                                                                                      *
#    Developed as free software within the context of the Degoras Project for the Satellite Laser Ranging Station      *
#    (SFEL) at the Spanish Navy Observatory (ROA) in San Fernando, Cádiz. The library is open for use by other SLR     *
#    stations and organizations, so we warmly encourage you to give it a try and feel free to contact us anytime!      *
#                                                                                                                      *
#    Copyright (C) 2024 Degoras Project Team                                                                           *
#                       < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >                             *
#                       < Jesús Relinque Madroñal >                                                                    *
#                                                                                                                      *
#    This file is part of LibZMQUtils.                                                                                 *
#                                                                                                                      *
#    Licensed under the European Union Public License (EUPL), Version 1.2 or subsequent versions of the EUPL license   *
#    as soon they will be approved by the European Commission (IDABC).                                                 *
#                                                                                                                      *
#    This project is free software: you can redistribute it and/or modify it under the terms of the EUPL license as    *
#    published by the IDABC, either Version 1.2 or, at your option, any later version.                                 *
#                                                                                                                      *
#    This project is distributed in the hope that it will be useful. Unless required by applicable law or agreed to in *
#    writing, it is distributed on an "AS IS" basis, WITHOUT ANY WARRANTY OR CONDITIONS OF ANY KIND; without even the  *
#    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the EUPL license to check specific   *
#    language governing permissions and limitations and more details.                                                  *
#                                                                                                                      *
#    You should use this project in compliance with the EUPL license. You should have received a copy of the license   *
#    along with this project. If not, see the license at < https://eupl.eu/ >.                                         *
# **********************************************************************************************************************

# **********************************************************************************************************************
# Updated 08/07/2024
# **********************************************************************************************************************

# **********************************************************************************************************************
# LIBZMQUTILS MAIN SHARED LIBRARY PROJECT CMAKELIST
# **********************************************************************************************************************

# ----------------------------------------------------------------------------------------------------------------------
# INITIALIZATION

# Cmake minimum and empty project for avoid warnings.
cmake_minimum_required(VERSION 3.21)
project(_)

# Set path to additional CMake modules.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${CMAKE_SOURCE_DIR}/CMakeModules
    ${CMAKE_SOURCE_DIR}/CMakeLibsConfig)

# Init CMake modules.
include(CMakeMacrosGlobals)
macro_cmakemodules_init("24.04.22")

# ----------------------------------------------------------------------------------------------------------------------
# BASIC PROJECT CONFIGURATION

# Base project configuration.
set(LIB_NAME LibZMQUtils)
set(LIB_VER 1.7.7)
set(LIB_BUILD_MODE Debug)
set(LIB_LANGUAGES CXX C)

# Installation dir.
if (WIN32)
    set(LIB_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../${LIB_NAME}_product_v${LIB_VER}")
else()
    set(LIB_INSTALL_DIR "/opt/${LIB_NAME}_product_v${LIB_VER}")
endif()

# Macros global configuration.
macro_global_set_show_externals(FALSE)
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

# WARNING QT CREATOR BUG
set(AVOID_QTCREATOR13_DLL_SEARCH_BUG TRUE)

# Specific configuration.
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
set(LIB_INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes/)
set(LIB_SOURCES_DIR ${CMAKE_SOURCE_DIR}/sources)

# Configure the LibZMQ package and set the located paths.
find_package(LibZMQ REQUIRED)

# Add external libraries to global scope.
macro_global_add_libs_full_paths("${LIBZMQ_LIBRARIES}")
macro_global_add_libs_folders("${LIBZMQ_LIBRARY_DIRS}")

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

# Link with LibZMQ.
target_link_libraries(${LIB_NAME} PUBLIC ${LIBZMQ_LIBRARIES})
target_include_directories(${LIB_NAME} PUBLIC ${LIBZMQ_INCLUDE_DIRS})

# Add external headers.
if(MODULES_GLOBAL_SHOW_EXTERNALS)
    target_sources(${LIB_NAME} PRIVATE ${LIBZMQ_INCLUDES})
endif()

# Specific links in Windows plattform.
if (WIN32)
    target_link_libraries(${LIB_NAME} PUBLIC ws2_32 iphlpapi)
endif()

# Include the shared library CMake configuration file for show in IDE only.
include(ConfigLibZMQUtils)

# ----------------------------------------------------------------------------------------------------------------------
# CONFIGURE THE COMPILERS

# Configure the compilers.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wpedantic -Wall -Wextra -O0 -fopenmp")
    else()
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wpedantic -Wall -Wextra -O3 -fopenmp")
    endif()
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

# Installation process.
macro_default_library_installation("${LIB_NAME}" "${LIB_INCLUDES_DIR}")

# Install additional files.
install(FILES "${CMAKE_SOURCE_DIR}/CMakeLibsConfig/FindLibZMQ.cmake"
        DESTINATION ${MODULES_GLOBAL_INSTALL_SHARE_PATH}/cmake)

# ----------------------------------------------------------------------------------------------------------------------
# CONFIGURE THE SUBPROJECTS

# Testing subproject.
add_subdirectory(testing)

# Example subproject.
add_subdirectory(examples)

# **********************************************************************************************************************

# WARNING Only for avoid the QTCREATOR 13 BUG WHEN EXECUTING LAUNCHERS.
if (WIN32 AND AVOID_QTCREATOR13_DLL_SEARCH_BUG)
    message(STATUS "Re-installing runtime deps due to QTCreator 13 bug...")
    message(STATUS "  Runtime deps: ${MODULES_GLOBAL_LIBS_FULL_PATHS}")
    message(STATUS "  Processing library deployment directory: ${MODULES_GLOBAL_INSTALL_BIN_PATH}")
    install(FILES ${MODULES_GLOBAL_LIBS_FULL_PATHS} DESTINATION "${MODULES_GLOBAL_INSTALL_BIN_PATH}")
    foreach(deploy_dir ${MODULES_GLOBAL_LAUNCHER_DEPLOYS_DIRS})
        message(STATUS "  Processing launchers deployment directory: ${deploy_dir}")
        install(FILES ${MODULES_GLOBAL_LIBS_FULL_PATHS} DESTINATION ${deploy_dir})
    endforeach()
endif()

# **********************************************************************************************************************
