# **********************************************************************************************************************
# LIBZMQUTILS EXAMPLE CMAKELIST
# **********************************************************************************************************************

# ----------------------------------------------------------------------------------------------------------------------
# CONFIGURATION

# Config.
set(MODULE_NAME CommandServerClient)
set(EXAMPLE_NAME ServerClientAmelas)
# --
set(EXAMPLE_DIR ${CMAKE_SOURCE_DIR}/examples/${MODULE_NAME}/${EXAMPLE_NAME})
set(EXAMPLE_FULL_NAME ${EXAMPLE_NAME})
set(EXAMPLE_INSTALL_PATH ${GLOBAL_LIBZMQUTILS_EXAMPLES_INSTALL_PATH}/${EXAMPLE_FULL_NAME})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Examples/${EXAMPLE_FULL_NAME})
set(INTERFACE_SERVER "AmelasServerInterface")
set(APP_SERVER "Example_CommandServer_Amelas")
set(APP_CLIENT "Example_CommandClient_Amelas")

# ----------------------------------------------------------------------------------------------------------------------
# AMELAS SERVER

# Get the header files for the server.
file(GLOB_RECURSE HEADERS
    ${EXAMPLE_DIR}/includes/AmelasController/*.h
    ${EXAMPLE_DIR}/includes/AmelasControllerServer/*.h)

# Get the template files for server.
file(GLOB_RECURSE TEMPLTS
    ${EXAMPLE_DIR}/includes/AmelasController/*.tpp
    ${EXAMPLE_DIR}/includes/AmelasControllerServer/*.tpp)

# Get the source files for server.
file(GLOB_RECURSE SOURCES
    ${EXAMPLE_DIR}/sources/ExampleServerAmelas.cpp
    ${EXAMPLE_DIR}/sources/AmelasController/*.cpp
    ${EXAMPLE_DIR}/sources/AmelasControllerServer/*.cpp)

# Create a custom group for the templates.
source_group("Header Files" FILES ${TEMPLTS})

# Setup the launcher.
macro_setup_launcher("${APP_SERVER}"
                     "${MODULES_GLOBAL_LIBS_OPTIMIZED}"
                     "${MODULES_GLOBAL_LIBS_DEBUG}"
                     ${HEADERS} ${TEMPLTS} ${SOURCES})

# Include directories.
target_include_directories(${APP_SERVER} PRIVATE ${CMAKE_SOURCE_DIR}/examples/${MODULE_NAME}/${EXAMPLE_NAME})

# In mingw better do static linking of the libgcc, libwinpthread and libstd.
if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static -lpthread")
    target_link_libraries(${APP_SERVER} PRIVATE -static-libgcc -static-libstdc++ -static -lpthread)
endif()

# Link against LibZMQUtils.
# NOTE You can comment this and link all launcher using global variable.
#macro_link_libzmqutils_default(${APP_SERVER} PUBLIC)

# ----------------------------------------------------------------------------------------------------------------------
# INTERFACE LIBRARY

# Auxiliar interface library.
add_library(${INTERFACE_SERVER} INTERFACE)
target_sources(${INTERFACE_SERVER} INTERFACE ${EXAMPLE_DIR}/sources/AmelasController/common.cpp)
target_include_directories(${INTERFACE_SERVER} INTERFACE ${EXAMPLE_DIR}/includes/)

# Add the interface library to the libraries list.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   macro_global_add_libs_debug("${INTERFACE_SERVER}")
else()
   macro_global_add_libs_optimized("${INTERFACE_SERVER}")
endif()

# ----------------------------------------------------------------------------------------------------------------------
# AMELAS CLIENT

# Get the header files for the client.
file(GLOB_RECURSE HEADERS ${EXAMPLE_DIR}/includes/AmelasControllerClient/*.h)

# Get the template files for client.
file(GLOB_RECURSE TEMPLTS ${EXAMPLE_DIR}/includes/AmelasControllerClient/*.tpp)

# Get the source files for client.
file(GLOB_RECURSE SOURCES
    ${EXAMPLE_DIR}/sources/ExampleClientAmelas.cpp
    ${EXAMPLE_DIR}/sources/AmelasControllerClient/*.cpp)

# Create a custom group for the templates and external.
source_group("Header Files" FILES ${TEMPLTS})

# Setup the client launcher.
macro_setup_launcher("${APP_CLIENT}"
                     "${MODULES_GLOBAL_LIBS_OPTIMIZED}"
                     "${MODULES_GLOBAL_LIBS_DEBUG}"
                     ${HEADERS} ${TEMPLTS} ${SOURCES})

# Include directories.
target_include_directories(${APP_CLIENT} PRIVATE ${CMAKE_SOURCE_DIR}/examples/${MODULE_NAME}/${EXAMPLE_NAME})

# In mingw better do static linking of the libgcc, libwinpthread and libstd.
if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static -lpthread")
    target_link_libraries(${APP_CLIENT} PRIVATE -static-libgcc -static-libstdc++ -static -lpthread)
endif()

# Link against LibZMQUtils.
# NOTE You can comment this and link all launcher using global variable.
#macro_link_libzmqutils_default(${APP_CLIENT} PUBLIC)

# ----------------------------------------------------------------------------------------------------------------------
# INSTALLATION PROCESS

# Prepare the external library search folders.
set(ext_libs_loc "${MODULES_GLOBAL_INSTALL_LIB_PATH}" "${MODULES_GLOBAL_INSTALL_BIN_PATH}")

# Install the launcher for server.
macro_install_launcher("${APP_SERVER}"
                       "${EXAMPLE_INSTALL_PATH}")

# Install the launcher for client.
macro_install_launcher("${APP_CLIENT}"
                       "${EXAMPLE_INSTALL_PATH}")

# Install runtime artifacts for server.
macro_install_runtime_artifacts("${APP_SERVER}"
                               "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                               "${EXAMPLE_INSTALL_PATH}")

# Install runtime artifacts for client.
macro_install_runtime_artifacts("${APP_CLIENT}"
                                "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                                "${EXAMPLE_INSTALL_PATH}")

# Install the runtime dependencies for server.
macro_install_runtime_deps("${APP_SERVER}"
                            "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                            "${MODULES_GLOBAL_LIBS_FOLDERS}"
                            "${EXAMPLE_INSTALL_PATH}"
                            "" "")

# Install the runtime dependencies for client.
macro_install_runtime_deps("${APP_CLIENT}"
                            "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                            "${MODULES_GLOBAL_LIBS_FOLDERS}"
                            "${EXAMPLE_INSTALL_PATH}"
                            "" "")

# ----------------------------------------------------------------------------------------------------------------------

# **********************************************************************************************************************
