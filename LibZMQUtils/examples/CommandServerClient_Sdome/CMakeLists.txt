
set(APP_FOLDER "CommandServerClient_Sdome")
set(APP_NAME_SERVER "CommandServer_Sdome")
set(APP_NAME_CLIENT "CommandClient_Sdome")

include_directories(${CMAKE_SOURCE_DIR}/examples/CommandServerClient_Sdome)

set(LIBRARIES LibZMQUtils)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Examples/${APP_FOLDER})

if (WIN32)
    link_directories(${CMAKE_CURRENT_BINARY_DIR})
endif (WIN32)

if (UNIX)
    link_directories(${CMAKE_CURRENT_BINARY_DIR}/../lib)
endif (UNIX)

FILE(GLOB_RECURSE SOURCES SdomeServer/*.cpp SdomeController/*.cpp ExampleServerSdome.cpp)
FILE(GLOB_RECURSE HEADERS SdomeServer/*.h SdomeController/*.h)

macro_setup_launcher(${APP_NAME_SERVER})

FILE(GLOB_RECURSE SOURCES SdomeClient/*.cpp ExampleClientSdome.cpp)
FILE(GLOB_RECURSE HEADERS SdomeClient/*.h )

macro_setup_launcher(${APP_NAME_CLIENT})
