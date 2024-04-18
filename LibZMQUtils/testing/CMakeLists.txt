# **********************************************************************************************************************
# LIBZMQUTILS TESTS CMAKELIST
# **********************************************************************************************************************

# ----------------------------------------------------------------------------------------------------------------------
# CONFIGURATION

# Set the installation example path.
set(GLOBAL_LIBZMQUTILS_TESTS_INSTALL_PATH ${MODULES_GLOBAL_INSTALL_BIN_PATH}/testing)

# Ignore paths for basic unit test:
set(IGNORED_PATHS
    "AlgorithmsSLR/UnitTest_PredictorSLR/*"
    "AlgorithmsSLR/ComplexTest_GeneratePasses/*")

# ----------------------------------------------------------------------------------------------------------------------
# ADD THE MODULES (SIMPLE AND COMPLEX TESTS)

# Add all the Modules.
add_subdirectory(Utilities)

# **********************************************************************************************************************
