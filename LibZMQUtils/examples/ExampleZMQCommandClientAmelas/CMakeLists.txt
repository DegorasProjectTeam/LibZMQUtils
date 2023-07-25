
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

set(APP_NAME "ExampleZMQCommanClientAmelas")

FILE(GLOB_RECURSE SOURCES *.cpp)
FILE(GLOB_RECURSE HEADERS *.h)

include_directories(${CMAKE_SOURCE_DIR}/examples/ExampleZMQCommanServerAmelas)
include_directories(${CMAKE_SOURCE_DIR}/examples/ExampleZMQCommanClientAmelas)

set(LIBRARIES LibZMQUtils)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Examples/${APP_NAME})

if (WIN32)
    link_directories(${CMAKE_CURRENT_BINARY_DIR})
endif (WIN32)

if (UNIX)
    link_directories(${CMAKE_CURRENT_BINARY_DIR}/../lib)
endif (UNIX)

# Get a list of all .cpp files in the same folder as CMakeLists.txt

# Create a test executable for each test source file
macro_setup_launcher(${APP_NAME})
