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
set(EXAMPLE_INSTALL_PATH ${GLOBAL_LIBDPSLR_EXAMPLES_INSTALL_PATH}/${EXAMPLE_FULL_NAME})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Examples/${EXAMPLE_FULL_NAME})
#set(INTERFACE_SERVER "AmelasServerInterface")
set(APP_PUBLISHER "Example_Logger_Publisher")
set(APP_SUBSCRIBER "Example_Logger_Subscriber")

# ----------------------------------------------------------------------------------------------------------------------
# SUBSCRIBER

# Get the header files for the subscriber.
file(GLOB_RECURSE HEADERS ${EXAMPLE_DIR}/includes/LoggerSubscriber/*.h)

# Get the template files for subscriber.
file(GLOB_RECURSE TEMPLTS ${EXAMPLE_DIR}/includes/LoggerSubscriber/*.tpp)

# Get the source files for subscriber.
file(GLOB_RECURSE SOURCES
    ${EXAMPLE_DIR}/sources/ExampleLoggerSubscriber.cpp
    ${EXAMPLE_DIR}/sources/LoggerSubscriber/*.cpp)

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
file(GLOB_RECURSE HEADERS ${EXAMPLE_DIR}/includes/LoggerPublisher/*.h)

# Get the template files for client.
file(GLOB_RECURSE TEMPLTS ${EXAMPLE_DIR}/includes/LoggerPublisher/*.tpp)

# Get the source files for client.
file(GLOB_RECURSE SOURCES
    ${EXAMPLE_DIR}/sources/ExampleLoggerPublisher.cpp
    ${EXAMPLE_DIR}/sources/LoggerPublisher/*.cpp)

# Create a custom group for the templates and external.
source_group("Header Files" FILES ${TEMPLTS})

# Setup the client launcher.
macro_setup_launcher("${APP_PUBLISHER}"
                     "${MODULES_GLOBAL_LIBS_OPTIMIZED}"
                     "${MODULES_GLOBAL_LIBS_DEBUG}"
                     ${HEADERS} ${TEMPLTS} ${SOURCES})

# Include directories.
target_include_directories(${APP_PUBLISHER} PRIVATE ${CMAKE_SOURCE_DIR}/examples/${MODULE_NAME}/${EXAMPLE_NAME})

# **********************************************************************************************************************
