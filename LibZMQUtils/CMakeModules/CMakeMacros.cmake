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

# **********************************************************************************************************************

MACRO(macro_prepare_install_dir bin_dir lib_dir base_dir)

    # Set the installation path.
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX ${base_dir} CACHE PATH "..." FORCE)
    endif()

    message(STATUS "${Green}Setting base installation directory: ${base_dir} ${ColourReset}")

    # Add properties for clean process.
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_INSTALL_PREFIX}/)

    # Define the install directories.
    if (WIN32 OR UNIX)
        # Base prefix.
        if(MINGW)
            set(${bin_dir} ${CMAKE_INSTALL_PREFIX}/bin/mingw-x86_64)
            set(${lib_dir} ${CMAKE_INSTALL_PREFIX}/lib/mingw-x86_64)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(${bin_dir} ${CMAKE_INSTALL_PREFIX}/bin/gcc-x86_64)
            set(${lib_dir} ${CMAKE_INSTALL_PREFIX}/lib/gcc-x86_64)
        elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            set(${bin_dir} ${CMAKE_INSTALL_PREFIX}/bin/msvc-x86_64)
            set(${lib_dir} ${CMAKE_INSTALL_PREFIX}/lib/msvc-x86_64)
        else()
            message(FATAL_ERROR "Compiler not supported by default.")
        endif()
        # Prefix for debug and release.
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(${lib_dir} ${INSTALL_LIB}_debug)
            set(${bin_dir} ${INSTALL_BIN}_debug)
        else()
            set(${lib_dir} ${INSTALL_LIB}_release)
            set(${bin_dir} ${INSTALL_BIN}_release)
        endif()
    else()
        message(FATAL_ERROR "Operating system not supported by default.")
    endif()

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_get_files_without_extension result curdir)
    file(GLOB_RECURSE FILES ${curdir})
    foreach(FILE ${FILES})
        get_filename_component(FILENAME ${FILE} NAME)
        if(${FILENAME} MATCHES "^[^.]+$")
            list(APPEND FILELIST ${FILE})
        endif()
    endforeach()
    set(${result} ${FILELIST})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_subdir_list result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
        list(APPEND dirlist ${child})
    endif()
    endforeach()
    set(${result} ${dirlist})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_add_subdirs curdir)
    macro_subdir_list(subdirs ${curdir})
    foreach(subdir ${subdirs})
      add_subdirectory(${subdir})
    endforeach()
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_setup_launcher LAUNCHER_NAME)

    message(STATUS "${Green}Preparing executable: ${LAUNCHER_NAME} ${ColourReset}")

    # Link the current dirs.
    if (WIN32)
        link_directories(${CMAKE_CURRENT_BINARY_DIR})
    elseif(UNIX)
        link_directories(${CMAKE_CURRENT_BINARY_DIR}/../lib)
    endif()

    add_executable(${LAUNCHER_NAME} ${SOURCES} ${HEADERS} ${TEMPLTS} ${ALIAS} ${EXTERN} ${RESOURCES})

    if(LIBRARIES)
        message(STATUS "${Green}  Linking general libraries for the executable. ${ColourReset}")
        target_link_libraries(${LAUNCHER_NAME} PRIVATE ${LIBRARIES})
    endif()

    if(LIBRARIES_OPTIMIZED)
        message(STATUS "${Green}  Linking optimized libraries for the executable. ${ColourReset}")
        foreach(LIBOPT ${LIBRARIES_OPTIMIZED})
            target_link_libraries(${LAUNCHER_NAME} optimized ${LIBOPT})
        endforeach(LIBOPT)
    endif(LIBRARIES_OPTIMIZED)

    if(LIBRARIES_DEBUG)
        message(STATUS "${Green}  Linking debug libraries for the executable. ${ColourReset}")
        foreach(LIBDEBUG ${LIBRARIES_DEBUG})
            target_link_libraries(${LAUNCHER_NAME} debug ${LIBDEBUG})
        endforeach(LIBDEBUG)
    endif(LIBRARIES_DEBUG)

    # Set target properties.
    message(STATUS "${Green}  Setting target properties. ${ColourReset}")
    set_target_properties(${LAUNCHER_NAME} PROPERTIES PROJECT_LABEL "Launcher ${LAUNCHER_NAME}")
    set_target_properties(${LAUNCHER_NAME} PROPERTIES DEBUG_OUTPUT_NAME "${LAUNCHER_NAME}${CMAKE_DEBUG_POSTFIX}")
    set_target_properties(${LAUNCHER_NAME} PROPERTIES RELEASE_OUTPUT_NAME "${LAUNCHER_NAME}${CMAKE_RELEASE_POSTFIX}")
    set_target_properties(${LAUNCHER_NAME} PROPERTIES RELWITHDEBINFO_OUTPUT_NAME "${LAUNCHER_NAME}${CMAKE_RELWITHDEBINFO_POSTFIX}")
    set_target_properties(${LAUNCHER_NAME} PROPERTIES MINSIZEREL_OUTPUT_NAME "${LAUNCHER_NAME}${CMAKE_MINSIZEREL_POSTFIX}")
    set_target_properties(${LAUNCHER_NAME} PROPERTIES FOLDER CMAKE_RUNTIME_OUTPUT_DIRECTORY)

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_deploy_launcher LAUNCHER_NAME INSTALL_PATH)

    message(STATUS "${Green}Preparing the deployment of the executable: ${LAUNCHER_NAME} ${ColourReset}")

    #Deploy binary files
    install(TARGETS ${LAUNCHER_NAME}
            RUNTIME DESTINATION ${INSTALL_PATH})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_deploy_runtime_artifacts LAUNCHER_NAME INSTALL_PATH DEPENDENCY_SET)

    message(STATUS "${Green}Preparing the runtime artifacts of the executable: ${LAUNCHER_NAME} ${ColourReset}")

    # Installation process for windows into installation dir.
    if(WIN32)
        install(IMPORTED_RUNTIME_ARTIFACTS ${LAUNCHER_NAME}
                RUNTIME_DEPENDENCY_SET ${DEPENDENCY_SET}
                DESTINATION ${INSTALL_PATH})
    endif()

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_setup_deploy_launcher LAUNCHER_NAME INSTALL_PATH DEPENDENCY_SET)

    macro_setup_launcher("${LAUNCHER_NAME}")
    macro_deploy_launcher("${LAUNCHER_NAME}" "${INSTALL_PATH}")
    macro_deploy_runtime_artifacts("${LAUNCHER_NAME}" "${INSTALL_PATH}" "${DEPENDENCY_SET}")

ENDMACRO()

# **********************************************************************************************************************
