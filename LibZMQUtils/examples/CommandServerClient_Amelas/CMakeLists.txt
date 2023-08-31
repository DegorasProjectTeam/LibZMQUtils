
set(APP_FOLDER "CommandServerClient_Amelas")
set(APP_NAME_SERVER "CommandServer_Amelas")
set(APP_NAME_CLIENT "CommandClient_Amelas")

include_directories(${CMAKE_SOURCE_DIR}/examples/CommandServerClient_Amelas)

set(LIBRARIES LibZMQUtils)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Examples/${APP_FOLDER})

if (WIN32)
    link_directories(${CMAKE_CURRENT_BINARY_DIR})
endif (WIN32)

if (UNIX)
    link_directories(${CMAKE_CURRENT_BINARY_DIR}/../lib)
endif (UNIX)

FILE(GLOB_RECURSE SOURCES AmelasServer/*.cpp AmelasController/*.cpp ExampleServerAmelas.cpp)
FILE(GLOB_RECURSE HEADERS AmelasServer/*.h AmelasController/*.h)

macro_setup_launcher(${APP_NAME_SERVER})

FILE(GLOB_RECURSE SOURCES AmelasClient/*.cpp ExampleClientAmelas.cpp)
FILE(GLOB_RECURSE HEADERS AmelasClient/*.h )

macro_setup_launcher(${APP_NAME_CLIENT})
