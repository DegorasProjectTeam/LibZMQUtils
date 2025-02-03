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
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{
// =====================================================================================================================

#ifdef _WIN32

class DebugConsole
{

private:
    HANDLE hChildStd_IN_Wr;  ///< Write handle for the child process's stdin
    PROCESS_INFORMATION piProcInfo; ///< Information about the child process
    bool isProcessOpen;
    std::string console_name_;

public:
    /// @brief Constructor initializes members
    DebugConsole(const std::string& console_name) :
        hChildStd_IN_Wr(nullptr),
        isProcessOpen(false), console_name_(console_name)
    {}

    /// @brief Destructor ensures cleanup
    ~DebugConsole() {
        closeProcess();
    }

    /// @brief Starts a new console process with customization
    /// @param customTitle Title of the console
    /// @return True if successful, false otherwise
    bool startProcess()
    {
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = nullptr;

        // Create an anonymous pipe for the child process's stdin
        HANDLE hChildStd_IN_Rd;
        if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0)) {
            std::cerr << "Failed to create pipe.\n";
            return false;
        }

        // Ensure the write handle is not inherited by the child process
        SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);

        // Set up the STARTUPINFO structure for the child process
        STARTUPINFOA siStartInfo;
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdInput = hChildStd_IN_Rd; // Redirect stdin
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        // Launch the child process using cmd.exe /K to keep the console open
        std::string command = "cmd /q /K"; // Keep the console running

        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
        if (!CreateProcessA(nullptr, const_cast<char*>(command.c_str()), nullptr, nullptr, TRUE,
                           CREATE_NEW_CONSOLE, nullptr, nullptr, &siStartInfo, &piProcInfo)) {
            std::cerr << "Failed to create child process.\n";
            CloseHandle(hChildStd_IN_Rd);
            return false;
        }

        CloseHandle(hChildStd_IN_Rd); // Close the read handle in the parent
        isProcessOpen = true;

        // Allow some time for the console to initialize
        Sleep(100);

        // Customize the child console
        if (!customizeConsole(this->console_name_))
        {
            std::cerr << "Failed to customize child console.\n";
            closeProcess();
            return false;
        }

        Sleep(200);


        return true;
    }

    bool sendString(const std::string& data)
    {
        if (!isProcessOpen) {
            std::cerr << "Process is not running.\n";
            return false;
        }

        std::string final_data = "@echo " + data + " \r\n";

        DWORD bytesWritten;
        if (!WriteFile(hChildStd_IN_Wr, final_data.c_str(), final_data.size(), &bytesWritten, nullptr)) {
            std::cerr << "Failed to write to child process.\n";
            return false;
        }

        return true;
    }

    /// @brief Sends data to the child process console
    /// @param data The message to send
    /// @return True if successful, false otherwise
    bool sendCommand(const std::string& cmd)
    {
        if (!isProcessOpen) {
            std::cerr << "Process is not running.\n";
            return false;
        }

        DWORD bytesWritten;
        if (!WriteFile(hChildStd_IN_Wr, cmd.c_str(), cmd.size(), &bytesWritten, nullptr)) {
            std::cerr << "Failed to write to child process.\n";
            return false;
        }

        return true;
    }

    /// @brief Closes the child process and resources
    void closeProcess() {
        if (isProcessOpen) {
            CloseHandle(hChildStd_IN_Wr);
            WaitForSingleObject(piProcInfo.hProcess, INFINITE);
            CloseHandle(piProcInfo.hProcess);
            CloseHandle(piProcInfo.hThread);
            isProcessOpen = false;
        }
    }

private:
    /// @brief Customizes the behavior of the child console
    /// @param customTitle The title to set for the child console
    /// @return True if successful, false otherwise
    bool customizeConsole(const std::string& customTitle)
    {
        std::string blockCommands =
            "@echo off & "
            "title " + customTitle + " & "
                            "prompt $G & "
                            "cls";

        sendCommand(blockCommands + "\r\n");
        // Hide the cursor in the child console
        sendCommand("echo \033[?25l\r\n");

            return true;
    }
};

class LIBZMQUTILS_EXPORT ConsoleProcess
{
private:
    HANDLE hChildStd_IN_Wr;  ///< Write handle for the child process's stdin
    PROCESS_INFORMATION piProcInfo; ///< Information about the child process
    bool isProcessOpen;

public:
    /// @brief Constructor initializes members
    ConsoleProcess() : hChildStd_IN_Wr(nullptr), isProcessOpen(false) {}

    /// @brief Destructor ensures cleanup
    ~ConsoleProcess() {
        closeProcess();
    }

    /// @brief Starts a new console process
    /// @param command Command to execute (child process)
    /// @return True if successful, false otherwise
    bool startProcess(const std::string& command)
    {
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE; // Handles are inheritable
        saAttr.lpSecurityDescriptor = nullptr;

        // Create an anonymous pipe for the child process's stdin
        HANDLE hChildStd_IN_Rd;

        if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0))
        {
            std::cerr << "Failed to create pipe.\n";
            return false;
        }

        // Ensure the write handle is not inherited by the child process
        SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);

        // Set up the STARTUPINFO structure for the child process
        STARTUPINFOA siStartInfo;
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdInput = hChildStd_IN_Rd; // Redirect stdin
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        // Create the child process
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
        if (!CreateProcessA(nullptr, const_cast<char*>(command.c_str()), nullptr, nullptr, TRUE,
                           CREATE_NEW_CONSOLE, nullptr, nullptr, &siStartInfo, &piProcInfo))
        {
            std::cerr << "Failed to create child process.\n";
            CloseHandle(hChildStd_IN_Rd);
            return false;
        }

        CloseHandle(hChildStd_IN_Rd); // Close the read handle in the parent
        isProcessOpen = true;


        return true;
    }

    /// @brief Sends data to the child process
    /// @param data Data to send
    /// @return True if successful, false otherwise
    bool sendData(const std::string& data) {
        if (!isProcessOpen) {
            std::cerr << "Process is not running.\n";
            return false;
        }

        DWORD bytesWritten;
        if (!WriteFile(hChildStd_IN_Wr, data.c_str(), data.size(), &bytesWritten, nullptr)) {
            std::cerr << "Failed to write to child process.\n";
            return false;
        }

        return true;
    }

    /// @brief Closes the child process and resources
    void closeProcess() {
        if (isProcessOpen) {
            CloseHandle(hChildStd_IN_Wr);
            WaitForSingleObject(piProcInfo.hProcess, INFINITE);
            CloseHandle(piProcInfo.hProcess);
            CloseHandle(piProcInfo.hThread);
            isProcessOpen = false;
        }
    }
};



#else


#endif

}} // END NAMESPACES.
// =====================================================================================================================
