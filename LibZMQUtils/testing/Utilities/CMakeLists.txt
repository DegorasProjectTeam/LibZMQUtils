
macro_subdir_list(subdirs ${CMAKE_CURRENT_SOURCE_DIR})

foreach(subdir ${subdirs})
  add_subdirectory(${subdir})
endforeach()

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17  -pthread -O3 -fopenmp")
set(APP_NAME "UnitTest_BinarySerializer")
set(SOURCES "UnitTest_BinarySerializer.cpp")
set(LIBRARIES LibZMQUtils)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Testing/Utilities/${APP_NAME})

if (WIN32)
    link_directories(${CMAKE_CURRENT_BINARY_DIR})
endif (WIN32)

if (UNIX)
    link_directories(${CMAKE_CURRENT_BINARY_DIR}/../lib)
endif (UNIX)

macro_setup_launcher(${APP_NAME})

add_test(NAME ${APP_NAME} COMMAND ${APP_NAME})
