# **********************************************************************************************************************
# Updated 11/03/2024
# **********************************************************************************************************************

# **********************************************************************************************************************

MACRO(macro_cmakemodules_init)

    # CMake includes
    include(CMakePackageConfigHelpers)

    # Includes.
    include(CMakeMacrosInstall)
    include(CMakeMacrosLauncher)
    include(CMakeMacrosUtils)
    include(CMakeMacrosProjects)
    include(CMakeMacrosLibraries)

    # Default global variables values.
    macro_global_set_main_dep_set_name("")
    macro_global_set_libs_debug("")
    macro_global_set_libs_optimized("")
    macro_global_set_install_bin_path("")
    macro_global_set_install_lib_path("")
    macro_global_set_install_share_path("")
    macro_global_set_install_include_path("")
    macro_global_set_install_include_path("")
    macro_global_set_show_externals(FALSE)
    macro_global_set_force_install_dir(TRUE)
    macro_global_set_install_runtime_deps(FALSE)
    macro_global_set_install_runtime_artifacts(TRUE)
    macro_global_set_exclude_so_libraries(TRUE)

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_main_dep_set_name dep_set_name)

    set(MODULES_GLOBAL_MAIN_DEP_SET_NAME ${dep_set_name})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_libs_debug lib_names)

    set(MODULES_GLOBAL_LIBS_DEBUG ${lib_names})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_libs_optimized lib_names)

    set(MODULES_GLOBAL_LIBS_OPTIMIZED ${lib_names})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_add_libs_debug lib_names)

set(MODULES_GLOBAL_LIBS_DEBUG ${MODULES_GLOBAL_LIBS_DEBUG} ${lib_names})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_add_libs_optimized lib_names)

set(MODULES_GLOBAL_LIBS_OPTIMIZED ${MODULES_GLOBAL_LIBS_OPTIMIZED} ${lib_names})

ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_bin_path path)
    set(MODULES_GLOBAL_INSTALL_BIN_PATH ${path})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_lib_path path)
    set(MODULES_GLOBAL_INSTALL_LIB_PATH ${path})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_share_path path)
    set(MODULES_GLOBAL_INSTALL_SHARE_PATH ${path})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_include_path path)
    set(MODULES_GLOBAL_INSTALL_INCLUDE_PATH ${path})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_show_externals enabled)
    set(MODULES_GLOBAL_SHOW_EXTERNALS ${enabled})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_force_install_dir enabled)
    set(MODULES_GLOBAL_FORCE_INSTALL_DIR ${enabled})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_runtime_deps enabled)
    set(MODULES_GLOBAL_INSTALL_RUNTIME_DEPS ${enabled} )
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_install_runtime_artifacts enabled)
    set(MODULES_GLOBAL_INSTALL_RUNTIME_ARTIFACTS ${enabled})
ENDMACRO()

# **********************************************************************************************************************

MACRO(macro_global_set_exclude_so_libraries enabled)
    set(MODULES_GLOBAL_EXCLUDE_SO_LIBS ${enabled})
ENDMACRO()

# **********************************************************************************************************************
