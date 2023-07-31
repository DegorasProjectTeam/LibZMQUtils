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
 * @file ExampleServerAmelas.cpp
 *
 * @brief This file serves as a program example of how to use the AmelasServer and AmelasController classes.
 *
 * This program initializes an instance of the AmelasServer class and sets it up to interact with an instance of
 * the AmelasController class. The server is set up to respond to client requests by invoking callback methods on the
 * controller. The program will run indefinitely until the user hits ctrl-c.
 *
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2307.1
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#ifdef _WIN32
#include <Windows.h>
#endif
#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <limits>
#include <any>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasServer/amelas_server.h"
#include "AmelasController/amelas_controller.h"
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------

// Global variables for safety ending.
volatile sig_atomic_t gSignInterrupt = 0;
std::condition_variable gExitCv;
std::mutex gMtx;

// Signal handler for safety ending.
#ifdef _WIN32
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    std::lock_guard<std::mutex> lock(gMtx);
    if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT)
    {
        if (!gSignInterrupt)
        {
            gSignInterrupt = 1;
            gExitCv.notify_all();
        }
        return TRUE;
    }
    return FALSE;
}
#else
// TODO
#endif

// Console configuration.
#ifdef _WIN32
void configConsole()
{
    // Set up the Windows Console Control Handler
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
    // Disable input proc.
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    mode &= ~ENABLE_LINE_INPUT;
    SetConsoleMode(hStdin, mode);
    // Hide the cursor.
    HANDLE myconsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO CURSOR;
    CURSOR.dwSize = 1;
    CURSOR.bVisible = false;
    SetConsoleCursorInfo(myconsole, &CURSOR);
}
#else
// TODO
void configConsole()
{

}
#endif

// ---------------------------------------------------------------------------------------------------------------------

/**
 * @brief Main entry point of the program.
 *
 * Initializes an AmelasController and AmelasServer, then enters an infinite loop where it listens for client requests
 * and processes them using the server. If the user hits ctrl-c, the server is shut down and the program exits.
 */
int main(int argc, char**argv)
{
    // Nampesaces.
    using namespace amelas::controller;
    using namespace amelas::cltsrv;
    using namespace zmqutils;

    // Configure the console.
    configConsole();

    // Configuration variables.
    unsigned port = 9999;
    bool client_status_check = false;

    // Instantiate the Amelas controller.
    AmelasController amelas_controller;

    // Instantiate the server.
    AmelasServer amelas_server(port);

    // Disable or enables the client status checking.
    amelas_server.setClientStatusCheck(client_status_check);

    // ---------------------------------------

    // Set the controller callbacks in the server.

    amelas_server.registerControllerCallback(AmelasServerCommand::REQ_SET_HOME_POSITION,
                                             &amelas_controller,
                                             &AmelasController::setHomePosition);

    amelas_server.registerControllerCallback(AmelasServerCommand::REQ_GET_HOME_POSITION,
                                             &amelas_controller,
                                             &AmelasController::getHomePosition);

    // ---------------------------------------

    // Start the server.
    bool started = amelas_server.startServer();

    // Check if the server starts ok.
    if(!started)
    {
        // Log.
        std::cout << "Server start failed!! Press Enter to exit!" << std::endl;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        return 1;
    }

    // Use the condition variable as an infinite loop until ctrl-c.
    std::unique_lock<std::mutex> lock(gMtx);
    gExitCv.wait(lock, [] { return gSignInterrupt == 1; });

    // Stop the server and wait the future.
    amelas_server.stopServer();

    // Final log.
    std::cout << "Server stoped. Press Enter to exit!" << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

    // Return.
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
