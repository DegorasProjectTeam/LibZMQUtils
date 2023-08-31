
set(APP_FOLDER "CommandServerClient_OldSFELDome")
set(APP_NAME_SERVER "CommandServer_OldSFELDome")
set(APP_NAME_CLIENT "CommandClient_OldSFELDome")

include_directories(${CMAKE_SOURCE_DIR}/examples/CommandServerClient_OldSFELDome)

set(LIBRARIES LibZMQUtils)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Examples/${APP_FOLDER})

if (WIN32)
    link_directories(${CMAKE_CURRENT_BINARY_DIR})
endif (WIN32)

if (UNIX)
    link_directories(${CMAKE_CURRENT_BINARY_DIR}/../lib)
endif (UNIX)

FILE(GLOB_RECURSE SOURCES ExampleServerOldSFELDome.cpp)
#FILE(GLOB_RECURSE HEADERS AmelasServer/*.h AmelasController/*.h)

macro_setup_launcher(${APP_NAME_SERVER})

FILE(GLOB_RECURSE SOURCES ExampleClientOldSFELDome.cpp)
#FILE(GLOB_RECURSE HEADERS AmelasClient/*.h )

macro_setup_launcher(${APP_NAME_CLIENT})
