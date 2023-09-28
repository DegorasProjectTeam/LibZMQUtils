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
 * @brief This file contains the implementation of the utility functions and classes for console interactions.
 *
 * The utilities of this module are used for creating examples demonstrating the use of the library. They provide
 * convenient ways to interact with the console. Please note that these utilities are specifically designed for
 * illustrative purposes and are not intended for real-world, production use. They may not have the robustness,
 * security, or optimizations necessary for production environments.
 *
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// LIBDPSLR INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Utilities/console_config.h>
#include <iostream>
#include <ostream>
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{

#ifdef _WIN32

ConsoleConfig::ConsoleConfig() :
    exit_callback_(nullptr),
    close_flag_(false),
    handle_stdin_(GetStdHandle(STD_INPUT_HANDLE)),
    handle_stdout_(GetStdHandle(STD_OUTPUT_HANDLE))
{}

BOOL WINAPI ConsoleConfig::StaticConsoleCtrlHandler(DWORD dwCtrlType)
{
    return ConsoleConfig::getInstance().consoleCtrlHandler(dwCtrlType);
}

ConsoleConfig &ConsoleConfig::getInstance()
{
    // Guaranteed to be destroyed, instantiated on first use.
    static ConsoleConfig instance;
    return instance;
}

void ConsoleConfig::configureConsole(bool apply_ctrl_handler, bool hide_cursor, bool input_proc)
{
    // Safe lock.
    std::lock_guard<std::mutex> lock(this->mtx_);

    GetConsoleCursorInfo(handle_stdout_, &this->orig_cursor_info_);
    GetConsoleMode(handle_stdin_, &this->orig_in_mode_);

    // Add the console control handler.
    // Note: The handler will work async.
    if(apply_ctrl_handler)
        SetConsoleCtrlHandler(StaticConsoleCtrlHandler, TRUE);

    // Disable the input proccesing.
    if(input_proc)
    {
        DWORD mode = this->orig_in_mode_ & ~static_cast<DWORD>(ENABLE_LINE_INPUT);
        SetConsoleMode(this->handle_stdin_, mode);
    }

    // Hide the console cursor.
    if(hide_cursor)
    {
        CONSOLE_CURSOR_INFO cursorInfo = orig_cursor_info_;
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(this->handle_stdout_, &cursorInfo);
    }
}

void ConsoleConfig::setExitCallback(const ExitConsoleCallback &exit_callback)
{
    // Safe lock.
    std::lock_guard<std::mutex> lock(this->mtx_);

    this->exit_callback_ = exit_callback;
}

ConsoleConfig::~ConsoleConfig()
{
    this->restoreConsole();
}

void ConsoleConfig::restoreConsole()
{
    // Safe lock.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Restore original input and output modes
    SetConsoleMode(this->handle_stdin_, this->orig_in_mode_);
    SetConsoleCursorInfo(this->handle_stdout_, &this->orig_cursor_info_);
}

BOOL ConsoleConfig::consoleCtrlHandler(DWORD dw_ctrl_t)
{
    // Safe lock.
    std::lock_guard<std::mutex> lock(this->mtx_);

    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2,2), &wsa_data);

    if (dw_ctrl_t == CTRL_C_EVENT || dw_ctrl_t == CTRL_BREAK_EVENT || dw_ctrl_t == CTRL_CLOSE_EVENT)
    {
        // Update the closing flag.
        ConsoleConfig::close_flag_ = true;

        // Call the exit callback
        if(this->exit_callback_)
            this->exit_callback_();

        // Notify with the cv and return.
        this->close_cv_.notify_all();

        return TRUE;
    }
    return FALSE;
}

void ConsoleConfig::waitForClose()
{
    // Safe lock.
    std::unique_lock<std::mutex> lock(this->cv_mtx_);
    this->close_cv_.wait(lock, [this]{ return this->close_flag_.load(); });
}

bool ConsoleConfig::closeStatus()
{
    return ConsoleConfig::close_flag_;
}

#else

// TODO

#endif

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
