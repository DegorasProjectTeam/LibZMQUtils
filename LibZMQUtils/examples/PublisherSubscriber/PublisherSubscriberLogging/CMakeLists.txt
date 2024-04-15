# **********************************************************************************************************************
# LIBZMQUTILS EXAMPLE CMAKELIST
# **********************************************************************************************************************

# ----------------------------------------------------------------------------------------------------------------------
# CONFIGURATION

# Config.
set(MODULE_NAME PublisherSubscriber)
set(EXAMPLE_NAME PublisherSubscriberLogging)
# --
set(EXAMPLE_DIR ${CMAKE_SOURCE_DIR}/examples/${MODULE_NAME}/${EXAMPLE_NAME})
set(EXAMPLE_FULL_NAME ${MODULE_NAME}_${EXAMPLE_NAME})
set(EXAMPLE_INSTALL_PATH ${GLOBAL_LIBZMQUTILS_EXAMPLES_INSTALL_PATH}/${EXAMPLE_FULL_NAME})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Examples/${EXAMPLE_FULL_NAME})
set(INTERFACE_LOGGER "LoggerCommon")
set(APP_PUBLISHER "Example_Logger_Publisher")
set(APP_SUBSCRIBER "Example_Logger_Subscriber")

# Auxiliar interface library.
add_library(${INTERFACE_LOGGER} INTERFACE)
target_sources(${INTERFACE_LOGGER} INTERFACE ${EXAMPLE_DIR}/sources/LoggerCommon/logger_common.cpp)
target_include_directories(${INTERFACE_LOGGER} INTERFACE
                           ${EXAMPLE_DIR}/includes/LoggerCommon)

# Add the interface library to the libraries list.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   macro_global_add_libs_debug("${INTERFACE_LOGGER}")
else()
   macro_global_add_libs_optimized("${INTERFACE_LOGGER}")
endif()

# ----------------------------------------------------------------------------------------------------------------------
# SUBSCRIBER

# Get the header files for the subscriber.
file(GLOB_RECURSE HEADERS
        ${EXAMPLE_DIR}/includes/LoggerSubscriber/*.h
        ${EXAMPLE_DIR}/includes/LoggerCommon/*.h)

# Get the template files for subscriber.
file(GLOB_RECURSE TEMPLTS
        ${EXAMPLE_DIR}/includes/LoggerSubscriber/*.tpp
        ${EXAMPLE_DIR}/includes/LoggerCommon/*.tpp)

# Get the source files for subscriber.
file(GLOB_RECURSE SOURCES
        ${EXAMPLE_DIR}/sources/ExampleLoggerSubscriber.cpp
        ${EXAMPLE_DIR}/sources/LoggerSubscriber/*.cpp
        ${EXAMPLE_DIR}/sources/LoggerCommon/*.cpp)

# Create a custom group for the templates.
source_group("Header Files" FILES ${TEMPLTS})

# Setup the launcher.
macro_setup_launcher("${APP_SUBSCRIBER}"
                     "${MODULES_GLOBAL_LIBS_OPTIMIZED}"
                     "${MODULES_GLOBAL_LIBS_DEBUG}"
                     ${HEADERS} ${TEMPLTS} ${SOURCES})

# Include directories.
target_include_directories(${APP_SUBSCRIBER} PRIVATE ${CMAKE_SOURCE_DIR}/examples/${MODULE_NAME}/${EXAMPLE_NAME})

# ----------------------------------------------------------------------------------------------------------------------
# PUBLISHER

# Get the header files for the client.
file(GLOB_RECURSE HEADERS
        ${EXAMPLE_DIR}/includes/LoggerPublisher/*.h
        ${EXAMPLE_DIR}/includes/LoggerCommon/*.h)

# Get the template files for client.
file(GLOB_RECURSE TEMPLTS
        ${EXAMPLE_DIR}/includes/LoggerPublisher/*.tpp
        ${EXAMPLE_DIR}/includes/LoggerCommon/*.tpp)

# Get the source files for client.
file(GLOB_RECURSE SOURCES
        ${EXAMPLE_DIR}/sources/ExampleLoggerPublisher.cpp
        ${EXAMPLE_DIR}/sources/LoggerPublisher/*.cpp
        ${EXAMPLE_DIR}/sources/LoggerCommon/*.cpp)

# Create a custom group for the templates and external.
source_group("Header Files" FILES ${TEMPLTS})

# Setup the client launcher.
macro_setup_launcher("${APP_PUBLISHER}"
                     "${MODULES_GLOBAL_LIBS_OPTIMIZED}"
                     "${MODULES_GLOBAL_LIBS_DEBUG}"
                     ${HEADERS} ${TEMPLTS} ${SOURCES})

# Include directories.
target_include_directories(${APP_PUBLISHER} PRIVATE ${CMAKE_SOURCE_DIR}/examples/${MODULE_NAME}/${EXAMPLE_NAME})

# ----------------------------------------------------------------------------------------------------------------------
# INSTALLATION PROCESS

# Prepare the external library search folders.
set(ext_libs_loc "${MODULES_GLOBAL_INSTALL_LIB_PATH}"
                 "${MODULES_GLOBAL_INSTALL_BIN_PATH}")

# Install the launcher for publisher.
macro_install_launcher("${APP_PUBLISHER}"
                       "${EXAMPLE_INSTALL_PATH}")

# Install the launcher for subscriber.
macro_install_launcher("${APP_PUBLISHER}"
                       "${EXAMPLE_INSTALL_PATH}")

# Install runtime artifacts for publisher.
macro_install_runtime_artifacts("${APP_PUBLISHER}"
                               "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                               "${EXAMPLE_INSTALL_PATH}")

# Install runtime artifacts for subscriber.
macro_install_runtime_artifacts("${APP_SUBSCRIBER}"
                                "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                                "${EXAMPLE_INSTALL_PATH}")

# Install the runtime dependencies for publisher.
macro_install_runtime_deps("${APP_PUBLISHER}"
                            "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                            "${ext_libs_loc}"
                            "${EXAMPLE_INSTALL_PATH}"
                            "" "")

# Install the runtime dependencies for subscriber.
macro_install_runtime_deps("${APP_SUBSCRIBER}"
                            "${MODULES_GLOBAL_MAIN_DEP_SET_NAME}"
                            "${ext_libs_loc}"
                            "${EXAMPLE_INSTALL_PATH}"
                            "" "")

# ----------------------------------------------------------------------------------------------------------------------

# **********************************************************************************************************************
