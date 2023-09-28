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

# Check external variables.
if (NOT DEFINED LIB_ZMQ_UTILS AND NOT DEFINED LIB_DEPS_SET AND NOT DEFINED INSTALL_BIN)
    message(FATAL_ERROR "Variables definition missing.")
endif()

# Configure names and build path.
set(APP_SERVER "Example_CommandServer_Amelas")
set(APP_CLIENT "Example_CommandClient_Amelas")
set(APP_BUILD_FOLDER ${CMAKE_BINARY_DIR}/bin/Examples/CommandServerClient_Amelas)

# Setup the libraries.
set(LIBRARIES ${LIB_ZMQ_UTILS})
set(LIBRARIES_OPTIMIZED "")
set(LIBRARIES_DEBUG "")

# Set the output directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_BUILD_FOLDER})

# Include directories.
include_directories(${CMAKE_SOURCE_DIR}/examples/CommandServerClient_Amelas)

# Get the headers, sources and template files for server.
file(GLOB_RECURSE SOURCES AmelasControllerServer/*.cpp AmelasController/*.cpp ExampleServerAmelas.cpp)
file(GLOB_RECURSE HEADERS AmelasControllerServer/*.h AmelasController/*.h)

# Setup the server launcher.
macro_setup_deploy_launcher("${APP_SERVER}" "${INSTALL_BIN}" "${LIB_DEPS_SET}")

# Get the headers, sources and template files for client.
file(GLOB_RECURSE SOURCES AmelasControllerClient/*.cpp ExampleClientAmelas.cpp)
file(GLOB_RECURSE HEADERS AmelasControllerClient/*.h )

# Setup the client launcher.
macro_setup_deploy_launcher("${APP_CLIENT}" "${INSTALL_BIN}" "${LIB_DEPS_SET}")

# **********************************************************************************************************************
