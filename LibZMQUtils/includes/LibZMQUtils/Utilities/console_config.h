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
 * @brief This file contains global utility functions and classes for console interactions.
 *
 * The utilities of this module are used for creating examples demonstrating the use of the library. They provide
 * convenient ways to interact with the console. Please note that these utilities are specifically designed for
 * illustrative purposes and are not intended for real-world, production use. They may not have the robustness,
 * security, or optimizations necessary for production environments.
 *
 * @warning Exported only for examples demostration. Not use for production environments.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
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
#else
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#endif
#include <csignal>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <iostream>
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{
// =====================================================================================================================

#ifdef _WIN32

class ConsoleConfig
{
public:

    /// Exit callback alias.
    using ExitConsoleCallback = std::function<void()>;

    /**
     * @brief Access to the singleton instance
     */
    LIBZMQUTILS_EXPORT static ConsoleConfig& getInstance();

    /**
     * @brief Configure the console settings.
     *
     * This function allows you to configure various settings for the console, such as applying a Ctrl handler,
     * hiding the cursor, and disable the input processing.
     *
     * @param ctrl_hndlr If true, a Ctrl handler will be applied to capture Ctrl events.
     * @param hide_cursor If true, the cursor will be hidden in the console.
     * @param allow_in If false, the input processing will be disabled.
     *
     * @note Enabling Ctrl handler allows you to capture Ctrl events like Ctrl+C or Ctrl+Break.
     * @note Hiding the cursor can be useful for creating a cleaner console interface.
     */
    LIBZMQUTILS_EXPORT void configureConsole(bool ctrl_hndlr = false, bool hide_cursor = false, bool allow_in = false);

    /**
     * @brief Setter function for the exit callback.
     */
    LIBZMQUTILS_EXPORT void setExitCallback(const ExitConsoleCallback& exit_callback);

    /**
     * @brief Restore Console Configuration.
     *
     * This function restores the console configuration to its original state, including input mode and cursor settings.
     * Call this function when you no longer need the custom console settings applied by `ConsoleConfig`.
     *
     * @note Use this function to clean up and restore the console settings before exiting your application.
     */
    LIBZMQUTILS_EXPORT void restoreConsole();

    /**
     * @brief Wait for Console Close Signal.
     *
     * This function blocks the calling thread until a console close signal is received, typically triggered by
     * events like Ctrl+C, Ctrl+Break, or Ctrl+Close. It is useful for waiting until the user chooses to exit
     * the application gracefully.
     */
    LIBZMQUTILS_EXPORT void waitForClose();

    /**
     * @brief Get Console Close Status.
     *
     * @return Returns true if a console close signal has been received, indicating that the application should
     * prepare for exit. Returns false otherwise.
     */
    LIBZMQUTILS_EXPORT bool closeStatus();

    /**
     * @brief Custom Console Control Handler.
     *
     * This virtual function can be overridden by subclasses of `ConsoleConfig` to implement a custom control handler
     * for console events. When subclassing `ConsoleConfig` and providing your own implementation of this function, you
     * can define how your application should respond to specific console control events, such as Ctrl+C or Ctrl+Break.
     *
     * @param dw_ctrl_t The type of control event received.
     * @return Returns TRUE if the control event is handled by the custom handler, or FALSE if it's not handled.
     *
     * @note When you override this function, you gain control over how your application responds to console
     * control events, allowing you to implement custom behavior for events like Ctrl+C.
     *
     * Example usage in a subclass of `ConsoleConfig`:
     * @code{.cpp}
     *   class MyConsoleConfig : public ConsoleConfig {
     *   public:
     *     virtual BOOL WINAPI consoleCtrlHandler(DWORD dw_ctrl_t) override {
     *       if (dw_ctrl_t == CTRL_C_EVENT) {
     *         // Handle Ctrl+C event in a custom way.
     *         // ...
     *         return TRUE; // Event is handled.
     *       }
     *       // Call the base class handler for other events.
     *       return ConsoleConfig::consoleCtrlHandler(dw_ctrl_t);
     *     }
     *   };
     * @endcode
     */
    virtual LIBZMQUTILS_EXPORT BOOL WINAPI consoleCtrlHandler(DWORD dw_ctrl_t);

    /**
     * @brief Virtual destructor. Restores the default console configuration.
     */
    virtual LIBZMQUTILS_EXPORT ~ConsoleConfig();

    // Deleted constructors and assignment operators.
    ConsoleConfig(const ConsoleConfig &) = delete;
    ConsoleConfig(ConsoleConfig&&) = delete;
    ConsoleConfig& operator=(const ConsoleConfig &) = delete;
    ConsoleConfig& operator=(ConsoleConfig&&) = delete;

private:

    // Private constructor.
    ConsoleConfig();

    // Static console handler function trick.
    static BOOL WINAPI StaticConsoleCtrlHandler(DWORD dwCtrlType);

    // Configuration.
    ExitConsoleCallback exit_callback_;
    std::condition_variable close_cv_;
    std::atomic_bool close_flag_;
    std::mutex mtx_;
    std::mutex cv_mtx_;

    // Console realted members.
    HANDLE handle_stdin_;
    HANDLE handle_stdout_;
    DWORD orig_in_mode_;
    CONSOLE_CURSOR_INFO orig_cursor_info_;
};

#else

class LIBZMQUTILS_EXPORT ConsoleConfig
{
public:

    /// Exit callback alias.
    using ExitConsoleCallback = std::function<void()>;

    /**
     * @brief Access to the singleton instance
     */
    LIBZMQUTILS_EXPORT static ConsoleConfig& getInstance()
    {
        static ConsoleConfig instance;
        return instance;
    }

    /**
     * @brief Configure the console settings.
     *
     * This function allows you to configure various settings for the console, such as applying a Ctrl handler,
     * hiding the cursor, and disable the input processing.
     *
     * @param ctrl_hndlr If true, a Ctrl handler will be applied to capture Ctrl events.
     * @param hide_cursor If true, the cursor will be hidden in the console.
     * @param allow_in If false, the input processing will be disabled.
     *
     * @note Enabling Ctrl handler allows you to capture Ctrl events like Ctrl+C or Ctrl+Break.
     * @note Hiding the cursor can be useful for creating a cleaner console interface.
     */
    void configureConsole(bool apply_ctrl_handler = false, bool hide_cursor = false, bool allow_in = true)
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

     /**
     * @brief Setter function for the exit callback.
     */
    void setExitCallback(const ExitConsoleCallback& exit_callback)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        this->exit_callback_ = exit_callback;
    }

    /**
     * @brief Restore Console Configuration.
     *
     * This function restores the console configuration to its original state, including input mode and cursor settings.
     * Call this function when you no longer need the custom console settings applied by `ConsoleConfig`.
     *
     * @note Use this function to clean up and restore the console settings before exiting your application.
     */
    void restoreConsole()
    {
        std::lock_guard<std::mutex> lock(mtx_);
        tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios_);
    }

        /**
     * @brief Wait for Console Close Signal.
     *
     * This function blocks the calling thread until a console close signal is received, typically triggered by
     * events like Ctrl+C, Ctrl+Break, or Ctrl+Close. It is useful for waiting until the user chooses to exit
     * the application gracefully.
     */
    void waitForClose() {
        std::unique_lock<std::mutex> lock(cv_mtx_);
        close_cv_.wait(lock, [this]{ return close_flag_.load(); });
    }

        /**
     * @brief Get Console Close Status.
     *
     * @return Returns true if a console close signal has been received, indicating that the application should
     * prepare for exit. Returns false otherwise.
     */
    bool closeStatus() const
        {
            return close_flag_.load();
        }


    /**
     * @brief Virtual destructor. Restores the default console configuration.
     */
        virtual ~ConsoleConfig() {
            restoreConsole();
        }

        // Deleted constructors and assignment operators.
        ConsoleConfig(const ConsoleConfig &) = delete;
        ConsoleConfig(ConsoleConfig&&) = delete;
        ConsoleConfig& operator=(const ConsoleConfig &) = delete;
        ConsoleConfig& operator=(ConsoleConfig&&) = delete;

    private:

        // Private constructor.
        ConsoleConfig(){}

        // Static console handler function trick.
        static void StaticSignalHandler(int signum)
        {
            ConsoleConfig& instance = getInstance();
            instance.signalHandler(signum);
        }

        void signalHandler(int signum)
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

        // Configuration.
        ExitConsoleCallback exit_callback_;
        std::condition_variable close_cv_;
        std::atomic_bool close_flag_;
        std::mutex mtx_;
        std::mutex cv_mtx_;
        struct termios orig_termios_;
};

#endif

}} // END NAMESPACES.
// =====================================================================================================================
