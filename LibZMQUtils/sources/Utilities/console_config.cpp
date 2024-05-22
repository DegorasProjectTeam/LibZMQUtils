/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
 *   A modern open-source and cross-platform C++ library with high-level utilities based on the well-known ZeroMQ      *
 *   open-source universal messaging library. Includes a suite of modules that encapsulates the ZMQ communication      *
 *   patterns as well as automatic binary serialization capabilities, specially designed for system infraestructure.   *
 *   The library is suited for the quick and easy integration of new and old systems and can be used in different      *
 *   sectors and disciplines seeking robust messaging and serialization solutions.                                     *
 *                                                                                                                     *
 *   Developed as free software within the context of the Degoras Project for the Satellite Laser Ranging Station      *
 *   (SFEL) at the Spanish Navy Observatory (ROA) in San Fernando, Cádiz. The library is open for use by other SLR     *
 *   stations and organizations, so we warmly encourage you to give it a try and feel free to contact us anytime!      *
 *                                                                                                                     *
 *   Copyright (C) 2024 Degoras Project Team                                                                           *
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
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
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

void ConsoleConfig::configureConsole(bool apply_ctrl_handler, bool hide_cursor, bool allow_in)
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
    if(!allow_in)
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

void ConsoleConfig::configureConsole(bool apply_ctrl_handler, bool hide_cursor, bool allow_in)
{
    std::lock_guard<std::mutex> lock(mtx_);

    // Store current settings
    tcgetattr(STDIN_FILENO, &orig_termios_);

    struct termios new_termios = orig_termios_;

    if (hide_cursor)
    {
        std::cout << "\033[?25l"; // ANSI escape code to hide cursor
    }

    if (!allow_in)
    {
        new_termios.c_lflag &= ~ICANON;  // Disable canonical mode
        new_termios.c_lflag &= ~ECHO;    // Disable echo
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    if (apply_ctrl_handler)
    {
        struct sigaction sa;
        sa.sa_handler = StaticSignalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);
    }
}

void ConsoleConfig::setExitCallback(const ExitConsoleCallback &exit_callback)
{
    std::lock_guard<std::mutex> lock(mtx_);
    this->exit_callback_ = exit_callback;
}

void ConsoleConfig::restoreConsole()
{
    std::lock_guard<std::mutex> lock(mtx_);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios_);
}

void ConsoleConfig::waitForClose()
{
    std::unique_lock<std::mutex> lock(cv_mtx_);
    close_cv_.wait(lock, [this]{ return close_flag_.load(); });
}

bool ConsoleConfig::closeStatus() const
{
    return close_flag_.load();
}

ConsoleConfig::~ConsoleConfig()
{
    restoreConsole();
}

void ConsoleConfig::StaticSignalHandler(int signum)
{
    ConsoleConfig& instance = getInstance();
    instance.signalHandler(signum);
}

void ConsoleConfig::signalHandler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM)
    {
        close_flag_.store(true);
        if (exit_callback_) {
            exit_callback_();
        }
        close_cv_.notify_all();
    }
}

#endif

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
