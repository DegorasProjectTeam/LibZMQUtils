# **********************************************************************************************************************
# LIBZMQUTILS TESTS CMAKELIST
# **********************************************************************************************************************

# ----------------------------------------------------------------------------------------------------------------------
# CONFIGURATION

# Set the installation example path.
set(GLOBAL_LIBZMQUTILS_TESTS_INSTALL_PATH ${MODULES_GLOBAL_INSTALL_BIN_PATH}/testing)

# ----------------------------------------------------------------------------------------------------------------------
# ADD THE MODULES (SIMPLE AND COMPLEX TESTS)

# Add all the Modules.
add_subdirectory(Utilities)
add_subdirectory(CommandServerClient)
add_subdirectory(PublisherSubscriber)

# **********************************************************************************************************************
