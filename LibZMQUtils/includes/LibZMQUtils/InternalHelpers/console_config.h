/***********************************************************************************************************************
 *   LibZMQUtils (ZMQ Utilitites Library): A libre library with ZMQ related useful utilities.                          *
 *                                                                                                                     *
 *   Copyright (C) 2023 Degoras Project Team                                                                           *
 *                      < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >                             *
 *                      < Jesús Relinque Madroñal >                                                                    *
 *                                                                                                                     *
 *   This file is part of LibZMQUtils.                                                                                 *
 *                                                                                                                     *
 *   Licensed under the European Union Public License (EUPL), Version 1.2 or subsequent versions of the EUPL license   *
 *   as soon they will be approved by the European Commission (IDABC).                                                 *
 *                                                                                                                     *
 *   This project is free software: you can redistribute it and/or modify it under the terms of the EUPL license as    *
 *   published by the IDABC, either Version 1.2 or, at your option, any later version.                                 *
 *                                                                                                                     *
 *   This project is distributed in the hope that it will be useful. Unless required by applicable law or agreed to in *
 *   writing, it is distributed on an "AS IS" basis, WITHOUT ANY WARRANTY OR CONDITIONS OF ANY KIND; without even the  *
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the EUPL license to check specific   *
 *   language governing permissions and limitations and more details.                                                  *
 *                                                                                                                     *
 *   You should use this project in compliance with the EUPL license. You should have received a copy of the license   *
 *   along with this project. If not, see the license at < https://eupl.eu/ >.                                         *
 **********************************************************************************************************************/

/** ********************************************************************************************************************
 * @file console_config.h
 *
 * @brief This file contains utility functions and classes for console interactions.
 *
 * The utilities of this module are used for creating examples demonstrating the use of the library. They provide
 * convenient ways to interact with the console. Please note that these utilities are specifically designed for
 * illustrative purposes and are not intended for real-world, production use. They may not have the robustness,
 * security, or optimizations necessary for production environments.
 *
 * @warning Exported only for examples demostration.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#ifdef _WIN32
#include <Windows.h>
#endif
#include <csignal>
#include <atomic>
#include <functional>
#include <condition_variable>
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace internal_helpers{
// =====================================================================================================================

#ifdef _WIN32

class ConsoleConfig
{
public:

    using ExitConsoleCallback = std::function<void()>;


    LIBZMQUTILS_EXPORT ConsoleConfig(bool apply_ctrl_handler = false, bool hide_cursor = false, bool input_proc = false);

    // Setter function for exit callback
    LIBZMQUTILS_EXPORT static void setExitCallback(const ExitConsoleCallback& exit_callback);

    LIBZMQUTILS_EXPORT ~ConsoleConfig();

    LIBZMQUTILS_EXPORT void restoreConsole();

    // Signal handler for safety ending.
    LIBZMQUTILS_EXPORT static BOOL WINAPI consoleCtrlHandler(DWORD dw_ctrl_t);

    LIBZMQUTILS_EXPORT static void waitForClose();

    static inline std::condition_variable gCloseCv;
    static inline std::atomic_bool gCloseFlag;
    static inline std::mutex gMtx;

private:

    // External exit callback.
    static inline ExitConsoleCallback exit_callback_;

    HANDLE hStdin_ = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hStdout_ = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD originalInputMode_;
    CONSOLE_CURSOR_INFO originalCursorInfo_;
};

#else

class LIBZMQUTILS_EXPORT ConsoleConfig
{
    // TODO
};

#endif

}} // END NAMESPACES.
// =====================================================================================================================
