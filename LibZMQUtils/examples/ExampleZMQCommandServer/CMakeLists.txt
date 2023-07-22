
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

set(APP_NAME "ExampleZMQCommandServer")

FILE(GLOB_RECURSE SOURCES *.cpp)
FILE(GLOB_RECURSE HEADERS *.h)


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

