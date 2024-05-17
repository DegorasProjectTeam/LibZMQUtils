/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *
 *   ExamplesLibZMQUtils related project.                                                                            *
 *                                                                                                        *
 *   A modern open-source C++ library with high-level utilities based on the well-known ZeroMQ open-source universal   *
 *   messaging library. Includes custom command based server-client and publisher-subscriber with automatic binary     *
 *   serialization capabilities, specially designed for system infraestructure. Developed as a free software under the *
 *   context of Degoras Project for the Spanish Navy Observatory SLR station (SFEL) in San Fernando and, of course,    *
 *   for any other station that wants to use it!                                                                       *
 *                                                                                                                     *
 *   Copyright (C) 2024 Degoras Project Team                                                                           *
 *                      < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >                             *
 *                      < Jesús Relinque Madroñal >                                                                    *                                                            *
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
 * @example ExampleServerAmelas.cpp
 *
 * @brief This file serves as a program example of how to use the AmelasServer and AmelasController classes.
 *
 * This program initializes an instance of the AmelasServer class and sets it up to interact with an instance of
 * the AmelasController class. The server is set up to respond to client requests by invoking callback methods on the
 * controller. The program will run indefinitely until the user hits ctrl-c.
 *
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
#include <limits>
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/Utilities>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "includes/AmelasControllerServer/amelas_controller_server.h"
#include "includes/AmelasController/amelas_controller.h"
#include "includes/AmelasControllerServer/amelas_controller_server_data.h"
// =====================================================================================================================

/**
 * @brief Main entry point of the program ExampleServerAmelas.
 *
 * Initializes an AmelasController and AmelasServer, then enters an infinite loop where it listens for client requests
 * and processes them using the server. If the user hits ctrl-c, the server is shut down and the program exits.
 */
int main(int, char**)
{
    // Nampesaces.
    using amelas::communication::AmelasControllerServer;
    using amelas::communication::AmelasServerCommand;
    using amelas::controller::AmelasController;
    // Callbacks
    using amelas::controller::SetHomePositionFunction;
    using amelas::controller::SetHomePositionFunctionInArgs;
    using amelas::controller::SetHomePositionFunctionOutArgs;
    using amelas::controller::GetHomePositionFunction;
    using amelas::controller::GetHomePositionFunctionInArgs;
    using amelas::controller::GetHomePositionFunctionOutArgs;
    using amelas::controller::DoOpenSearchTelescopeFunction;
    using amelas::controller::DoOpenSearchTelescopeFunctionInArgs;
    using amelas::controller::DoOpenSearchTelescopeFunctionOutArgs;

    // Configure the console.
    zmqutils::utils::ConsoleConfig& console_cfg = zmqutils::utils::ConsoleConfig::getInstance();
    console_cfg.configureConsole(true, true, false);

    // Configuration variables.
    std::string ip = "*";                                       // Server Ip.
    unsigned port = 9999;                                       // Server connection port.
    bool client_status_check = true;                            // Disable or enable the clients alive status.
    unsigned max_client_connections = 2;                        // Maximum number of client connections.
    unsigned recconn_attempts = 2;                              // Reconnection attempts for the server.
    std::chrono::milliseconds alive_timeout_ms = 10000ms;       // Timeout to consider a client dead.

    // Instantiate the Amelas controller.
    AmelasController amelas_controller;

    // Instantiate the server.
    AmelasControllerServer amelas_server(port, ip, "AMELAS EXAMPLE SERVER", "1.7.6", "This is the AMELAS server.");

    // Configure the server.
    amelas_server.setClientStatusCheck(client_status_check);
    amelas_server.setClientAliveTimeout(alive_timeout_ms);
    amelas_server.setReconectionAttempts(recconn_attempts);
    amelas_server.setMaxNumberOfClients(max_client_connections);

    // ---------------------------------------

    // Set the controller callbacks in the server.

    amelas_server.registerCbAndReqProcFunc<SetHomePositionFunction,
                                           SetHomePositionFunctionInArgs,
                                           SetHomePositionFunctionOutArgs>
        (AmelasServerCommand::REQ_SET_HOME_POSITION,
         &amelas_controller,
         &AmelasController::setHomePosition);

    amelas_server.registerCbAndReqProcFunc<GetHomePositionFunction,
                                       GetHomePositionFunctionInArgs,
                                       GetHomePositionFunctionOutArgs>
        (AmelasServerCommand::REQ_GET_HOME_POSITION,
         &amelas_controller,
         &AmelasController::getHomePosition);

    amelas_server.registerCbAndReqProcFunc<DoOpenSearchTelescopeFunction,
                                       DoOpenSearchTelescopeFunctionInArgs,
                                       DoOpenSearchTelescopeFunctionOutArgs>
        (AmelasServerCommand::REQ_DO_OPEN_SEARCH_TELESCOPE,
         &amelas_controller,
         &AmelasController::doOpenSearchTelescope);

    // ---------------------------------------

    // Start the server.
    bool started = amelas_server.startServer();

    // Check if the server starts ok.
    if(!started)
    {
        // Log.
        console_cfg.restoreConsole();
        std::cout << "Server start failed!! Press Enter to exit!" << std::endl;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.clear();
        return 1;
    }

    // Wait for closing as an infinite loop until ctrl-c.
    console_cfg.waitForClose();

    // Log.
    std::cout << "Stopping the server..." << std::endl;

    // Stop the server.
    amelas_server.stopServer();

    // Final log.
    std::cout << "Server stoped. All ok!!" << std::endl;

    // Restore the console.
    console_cfg.restoreConsole();

    // Return.
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
