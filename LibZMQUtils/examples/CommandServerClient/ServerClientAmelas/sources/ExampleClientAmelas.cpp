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
 * @example ExampleClientAmelas.cpp
 *
 * @brief This file serves as a program example of how to use the `AmelasClient` class.
 *
 * This program initializes an instance of the `AmelasClient` class to interact with an `AmelasServer`.
 *
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/Utilities>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "includes/AmelasController/amelas_controller_data.h"
#include "includes/AmelasControllerServer/amelas_controller_server_data.h"
#include "includes/AmelasControllerClient/amelas_controller_client.h"
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
using zmqutils::reqrep::CommandType;
using zmqutils::reqrep::CommandReply;
using zmqutils::reqrep::ServerCommand;
using zmqutils::reqrep::CommandClientBase;
using zmqutils::reqrep::RequestData;
using zmqutils::reqrep::OperationResult;
using amelas::communication::AmelasControllerClient;
using amelas::communication::AmelasServerCommand;
using amelas::controller::AltAzPos;
using amelas::controller::AmelasError;
// ---------------------------------------------------------------------------------------------------------------------

// Helper class to parse the terminal commands.
class AmelasClientCmdParser
{
public:

    AmelasClientCmdParser(AmelasControllerClient &client) : client_(client)
    {}

    zmqutils::reqrep::OperationResult parseCommand(const std::string &command)
    {
        CommandType command_id;
        auto tokens = zmqutils::internal_helpers::strings::split<std::vector<std::string>>(command, " ", false);

        if (tokens.empty())
        {
            std::cout << "Not a valid command." << std::endl;
            return OperationResult::UNKNOWN_COMMAND;
        }

        try
        {
            command_id = static_cast<CommandType>(std::stoi(tokens.front()));
        }
        catch (...)
        {
            std::cout << "Not a valid command." << std::endl;
            return OperationResult::UNKNOWN_COMMAND;
        }

        if (!this->client_.validateCommand(command_id))
        {
            std::cout << "Not implemented command." << std::endl;
            return OperationResult::NOT_IMPLEMENTED;
        }

        std::vector<std::string> params;
        std::move(tokens.begin() + 1, tokens.end(), std::back_inserter(params));

        return this->executeCommand(command_id, params);
    }

    OperationResult executeCommand(CommandType command_id, const std::vector<std::string> &params)
    {
        OperationResult res;

        if (command_id == static_cast<CommandType>(ServerCommand::REQ_CONNECT))
        {
            std::cout << "Sending REQ_CONNECT command." << std::endl;
            res = this->client_.doConnect();
        }
        else if (command_id == static_cast<CommandType>(ServerCommand::REQ_DISCONNECT))
        {
            std::cout << "Sending REQ_DISCONNECT command" << std::endl;
            res = this->client_.doDisconnect();
        }
        else if (command_id == static_cast<CommandType>(ServerCommand::REQ_ALIVE))
        {
            std::cout << "Sending REQ_ALIVE command." << std::endl;
            res = this->client_.doAlive();
        }
        else if (command_id == static_cast<CommandType>(ServerCommand::REQ_GET_SERVER_TIME))
        {
            std::cout << "Sending REQ_GET_SERVER_TIME command." << std::endl;
            std::string datetime;
            res = this->client_.doGetServerTime(datetime);

            if(res == OperationResult::COMMAND_OK)
                std::cout << "GET_SERVER_TIME command executed succesfully. "
                          << "Server time is: " << datetime << std::endl;
            else
                std::cout << "GET_SERVER_TIME command failed." << std::endl;
        }
        else if (command_id == static_cast<CommandType>(ServerCommand::REQ_PING))
        {
            std::cout << "Sending REQ_PING command." << std::endl;
            std::chrono::milliseconds elapsed_time;
            res = this->client_.doPing(elapsed_time);

            if(res == OperationResult::COMMAND_OK)
                std::cout << "PING command executed succesfully. ";

            else
                std::cout << "PING command failed. ";

            std::cout << "Elapsed time is: " << std::to_string(elapsed_time.count()) << " ms." << std::endl;
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_GET_HOME_POSITION))
        {
            std::cout << "Sending GET_HOME_POSITION command." << std::endl;
            AltAzPos pos;
            AmelasError error = AmelasError::INVALID_ERROR;
            res = this->client_.getHomePosition(pos, error);
            this->processGetHomePosition(res, pos, error);
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_SET_HOME_POSITION))
        {
            std::cout << "Sending SET_HOME_POSITION command." << std::endl;
            AltAzPos pos;
            AmelasError error = AmelasError::INVALID_ERROR;
            bool params_valid = this->parseAltAz(params, pos);
            if (params_valid)
                res = this->client_.setHomePosition(pos, error);
            else
                res = OperationResult::BAD_PARAMETERS;
            this->processSetHomePosition(res, error);
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_DO_OPEN_SEARCH_TELESCOPE))
        {
            std::cout << "Sending REQ_DO_OPEN_SEARCH_TELESCOPE command." << std::endl;
            AmelasError error = AmelasError::INVALID_ERROR;
            res = this->client_.doOpenSearchTelescope(error);
            this->processDoOpenSearchTelescope(res, error);
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_DO_EXAMPLE_NOT_IMP))
        {
            std::cout << "Sending REQ_DO_EXAMPLE_NOT_IMP command." << std::endl;
            AmelasError error = AmelasError::INVALID_ERROR;
            res = this->client_.doExampleNotImp(error);
            this->processDoExampleNotImp(res, error);
        }
        else
        {
            res = OperationResult::NOT_IMPLEMENTED;
        }
        return res;

    }

private:

    bool parseAltAz(const std::vector<std::string> &params, AltAzPos &pos)
    {
        if (params.size() != 2)
            return false;

        bool valid_params = true;

        try
        {
            pos.az = std::stod(params[0]);
            pos.el = std::stod(params[1]);
        }
        catch (...)
        {
            valid_params = false;
        }

        return valid_params;

    }

    void processGetHomePosition(zmqutils::reqrep::OperationResult res,
                                const amelas::controller::AltAzPos &pos,
                                amelas::controller::AmelasError error)
    {
        if (OperationResult::COMMAND_OK == res)
        {
            if (AmelasError::SUCCESS == error)
                std::cout << "GET_HOME_POSITION command executed succesfully. "
                          << "Position is, Az: " << pos.az << ", El: " << pos.el << std::endl;
            else
                std::cout << "GET_HOME_POSITION command failed. Controller error code is: "
                          << static_cast<int>(error) << std::endl;
        }
        else
        {
            std::cerr << "GET_HOME_POSITION command failed. Operation error code is: "
                      << static_cast<int>(res) << std::endl;
        }
    }

    void processSetHomePosition(zmqutils::reqrep::OperationResult res,
                                amelas::controller::AmelasError error)
    {
        if (OperationResult::COMMAND_OK == res)
        {
            if (AmelasError::SUCCESS == error)
                std::cout << "SET_HOME_POSITION command executed succesfully." << std::endl;
            else
                std::cerr << "SET_HOME_POSITION command failed. Bad position. Controller Error Code is: "
                          << static_cast<int>(error) << std::endl;
        }
        else
        {
            std::cerr << "SET_HOME_POSITION command failed. Operation error code is: "
                      << static_cast<int>(res) << std::endl;
        }
    }

    void processDoOpenSearchTelescope(zmqutils::reqrep::OperationResult res,
                                      amelas::controller::AmelasError error)
    {
        if (OperationResult::COMMAND_OK == res)
        {
            if (AmelasError::SUCCESS == error)
                std::cout << "REQ_DO_OPEN_SEARCH_TELESCOPE command executed succesfully." << std::endl;
            else
                std::cerr << "REQ_DO_OPEN_SEARCH_TELESCOPE command failed. Controller Error Code is: "
                          << static_cast<int>(error) << std::endl;
        }
        else
        {
            std::cerr << "REQ_DO_OPEN_SEARCH_TELESCOPE command failed. Operation error code is: "
                      << static_cast<int>(res) << std::endl;
        }
    }

    void processDoExampleNotImp(zmqutils::reqrep::OperationResult res,
                                amelas::controller::AmelasError error)
    {
        if (OperationResult::COMMAND_OK == res)
        {
            if (AmelasError::SUCCESS == error)
                std::cout << "REQ_DO_EXAMPLE_NOT_IMP command executed succesfully." << std::endl;
            else
                std::cerr << "REQ_DO_EXAMPLE_NOT_IMP command failed. Controller Error Code is: "
                          << static_cast<int>(error) << std::endl;
        }
        else
        {
            std::cerr << "REQ_DO_EXAMPLE_NOT_IMP command failed. Operation error code is: "
                      << static_cast<int>(res) << std::endl;
        }
    }

    AmelasControllerClient& client_;
};

// ---------------------------------------------------------------------------------------------------------------------

/**
 * @brief Main entry point of the program `ExampleClientAmelas`.
 */
int main(int, char**)
{
    // Configure the console.
    zmqutils::utils::ConsoleConfig& console_cfg = zmqutils::utils::ConsoleConfig::getInstance();
    console_cfg.configureConsole(true, false, true);

    // Configuration variables.
    std::string endpoint = "tcp://127.0.0.1:9999";          // Client endpoint.
    bool enable_alive_callbacks = false;                    // Disable or enable the alive callbacks.
    std::chrono::milliseconds alive_timeout_ms = 2000ms;    // Timeout to consider a client dead.
    std::chrono::milliseconds alive_period_ms = 1000ms;     // Timeout to consider a client dead.

    // Instanciate the client.
    AmelasControllerClient client(endpoint, "AMELAS EXAMPLE CLIENT", "1.7.6", "This is the AMELAS client.");

    // Prepare the auxiliar testing parser.
    AmelasClientCmdParser client_parser(client);

    // Configure the client.
    client.setAliveCallbacksEnabled(enable_alive_callbacks);
    client.setServerAliveTimeout(alive_timeout_ms);
    client.setSendAlivePeriod(alive_period_ms);

    // Set the exit callback to the console handler for safety.
    console_cfg.setExitCallback(
            [&client]()
            {
                std::cout << std::endl;
                std::cout << "Stopping the client..." << std::endl;
                client.stopClient();
            });

    // Start the client.
    bool started = client.startClient();

    // Check if all ok.
    if(!started)
    {
        std::cout<<"Unable to start the client.";
        return 1;
    }

    //client.startAutoAlive();
    std::string command;

    // Infinite loop for test.
    while(!console_cfg.closeStatus())
    {
        // Get the command and parameters.
        std::cout<<"------------------------------------------------------"<<std::endl;
        std::cout<<"-- Basic Commands --"<<std::endl;
        std::cout<<"- REQ_CONNECT:          0"<<std::endl;
        std::cout<<"- REQ_DISCONNECT:       1"<<std::endl;
        std::cout<<"- REQ_ALIVE:            2"<<std::endl;
        std::cout<<"- REQ_GET_SERVER_TIME:  3"<<std::endl;
        std::cout<<"- REQ_PING:             4"<<std::endl;
        std::cout<<"-- Specific Commands --"<<std::endl;
        std::cout<<"- REQ_SET_HOME_POSITION:        51 az el"<<std::endl;
        std::cout<<"- REQ_GET_HOME_POSITION:        52"<<std::endl;
        std::cout<<"- REQ_DO_OPEN_SEARCH_TELESCOPE: 53"<<std::endl;
        std::cout<<"- REQ_DO_EXAMPLE_NOT_IMP:       54"<<std::endl;
        std::cout<<"-- Other --"<<std::endl;
        std::cout<<"- Client exit:             exit"<<std::endl;
        std::cout<<"- Enable auto-alive:       auto_alive_en"<<std::endl;
        std::cout<<"- Disable auto-alive:      auto_alive_ds"<<std::endl;
        std::cout<<"- Enable auto-alive clbk:  auto_alive_clbk_en"<<std::endl;
        std::cout<<"- Disable auto-alive clbk: auto_alive_clbk_ds"<<std::endl;
        std::cout<<"------------------------------------------------------"<<std::endl;
        std::cout<<"Write a command: ";
        std::getline(std::cin, command);

        // Check for other commands.
        if(command == "exit")
        {
            std::cout << "Stopping the client..." << std::endl;
            client.stopClient();
            break;
        }
        else if(command == "auto_alive_en")
        {
            std::cout << "Enabling auto-alive..." << std::endl;
            client.doDisconnect();
            client.doConnect(true);
            continue;
        }
        else if(command == "auto_alive_ds")
        {
            std::cout << "Disabling auto-alive..." << std::endl;
            client.disableAutoAlive();
            continue;
        }
        else if(command == "auto_alive_clbk_en")
        {
            std::cout << "Enabling auto-alive clbk..." << std::endl;
            client.setAliveCallbacksEnabled(true);
            continue;
        }
        else if(command == "auto_alive_clbk_ds")
        {
            std::cout << "Disabling auto-alive clbk..." << std::endl;
            client.setAliveCallbacksEnabled(false);
            continue;
        }

        // Break if we want to close the example program.
        if(console_cfg.closeStatus() || std::cin.eof())
        {
            console_cfg.waitForClose();
            break;
        }

        // Parse the command.
        client_parser.parseCommand(command);
    }

    // Final log.
    std::cout << "Client stoped. All ok!!" << std::endl;

    // Restore the console.
    console_cfg.restoreConsole();

    // Return.
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
