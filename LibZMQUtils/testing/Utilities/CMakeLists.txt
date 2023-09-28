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

# Check external variables.
if (NOT DEFINED LIB_ZMQ_UTILS AND NOT DEFINED LIB_DEPS_SET)
    message(FATAL_ERROR "Variables definition missing.")
endif()

# Add the tests in the subdirectories.
macro_add_subdirs(${CMAKE_CURRENT_SOURCE_DIR})

# **********************************************************************************************************************
# Configure each simple test.

# Setup the common libraries.
set(LIBRARIES ${LIB_ZMQ_UTILS})

# List of tests.
set(TEST_NAMES
    "UnitTest_BinarySerializer")

# Loop through the test names and configure each test
foreach(TEST_NAME ${TEST_NAMES})
    set(SOURCES "${TEST_NAME}.cpp")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Testing/Utilities/${TEST_NAME})
    # Call the macro to set up the test
    macro_setup_launcher("${TEST_NAME}" "${LIBS}" "" "")
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# **********************************************************************************************************************
