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
 * @file custom_console.h
 * @brief This file contains utility functions and classes for console interactions.
 *
 * The utilities of this module are used for creating examples demonstrating the use of the library. They provide
 * convenient ways to interact with the console. Please note that these utilities are specifically designed for
 * illustrative purposes and are not intended for real-world, production use. They may not have the robustness,
 * security, or optimizations necessary for production environments.
 *
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2308.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/libzmqutils_global.h"
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#ifdef _WIN32
#include <Windows.h>
#endif
#include <iostream>
#include <chrono>
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

class LIBZMQUTILS_EXPORT ConsoleConfig
{
public:

    using ExitConsoleCallback = std::function<void()>;


    ConsoleConfig(bool apply_ctrl_handler = false, bool hide_cursor = false, bool input_proc = false)
    {
        // Set the global close flag to false.
        ConsoleConfig::gCloseFlag = false;

        GetConsoleCursorInfo(hStdout_, &originalCursorInfo_);
        GetConsoleMode(hStdin_, &originalInputMode_);

        // Add the console control handler.
        // Note: The handler will work async.
        if(apply_ctrl_handler)
            SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);

        // Disable the input proccesing.
        if(input_proc)
        {
            DWORD mode = originalInputMode_ & ~static_cast<DWORD>(ENABLE_LINE_INPUT);
            SetConsoleMode(hStdin_, mode);
        }

        // Hide the console cursor.
        if(hide_cursor)
        {
            CONSOLE_CURSOR_INFO cursorInfo = originalCursorInfo_;
            cursorInfo.bVisible = false;
            SetConsoleCursorInfo(hStdout_, &cursorInfo);
        }
    }

    // Setter function for exit callback
    static void setExitCallback(const ExitConsoleCallback& exit_callback)
    {
        ConsoleConfig::exit_callback_ = exit_callback;
    }

    ~ConsoleConfig()
    {
        restoreConsole();
    }

    void restoreConsole()
    {
        // Restore original input and output modes
        SetConsoleMode(hStdin_, originalInputMode_);
        SetConsoleCursorInfo(hStdout_, &originalCursorInfo_);
    }

    // Signal handler for safety ending.
    static BOOL WINAPI consoleCtrlHandler(DWORD dw_ctrl_t)
    {
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2,2), &wsa_data);

        std::lock_guard<std::mutex> lock(ConsoleConfig::gMtx);
        if (dw_ctrl_t == CTRL_C_EVENT || dw_ctrl_t == CTRL_BREAK_EVENT || dw_ctrl_t == CTRL_CLOSE_EVENT)
        {
            // Update the closing flag.
            ConsoleConfig::gCloseFlag = true;

            // Call the exit callback
            if(ConsoleConfig::exit_callback_)
                ConsoleConfig::exit_callback_();

            // Notify with the cv and return.
            ConsoleConfig::gCloseCv.notify_all();

            return TRUE;
        }
        return FALSE;
    }

    static void waitForClose()
    {
        std::unique_lock<std::mutex> lock(ConsoleConfig::gMtx);
        ConsoleConfig::gCloseCv.wait(lock, []{ return ConsoleConfig::gCloseFlag.load(); });
    }

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
