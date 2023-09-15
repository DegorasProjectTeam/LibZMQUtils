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
 * @example ExampleClientAmelas.cpp
 *
 * @brief This file serves as a program example of how to use the `AmelasClient` class.
 *
 * This program initializes an instance of the `AmelasClient` class to interact with an `AmelasServer`.
 *
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.1
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
#include <cstring>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Utils>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasController/common.h"
#include "AmelasControllerServer/common.h"
#include "AmelasControllerClient/amelas_controller_client.h"
// =====================================================================================================================


// TODO Remove
using namespace zmqutils;
using namespace amelas;
using namespace amelas::communication;
using namespace amelas::communication::common;
using namespace amelas::controller;


using zmqutils::common::CommandType;
using zmqutils::common::CommandReply;
using zmqutils::common::ServerCommand;
using zmqutils::utils::BinarySerializer;

void parseCommand(CommandClientBase &client, const std::string &command)
{
    zmqutils::common::ClientResult client_result = ClientResult::COMMAND_OK;

    char *command_str = new char[command.size()];
    std::copy(command.begin(), command.end(), command_str);

    char *token = std::strtok(command_str, " ");

    if (token)
    {
        CommandType command_id;

        try
        {
            command_id = static_cast<CommandType>(std::stoi(token));
        }
        catch (...)
        {
            std::cerr << "Failed at sending command." << std::endl;
            delete[] command_str;
            return;
        }

        RequestData command_msg(static_cast<ServerCommand>(command_id));

        bool valid = true;

        if (command_id == static_cast<CommandType>(ServerCommand::REQ_CONNECT))
        {
            std::cout << "Sending connect message" << std::endl;
        }
        else if (command_id == static_cast<CommandType>(ServerCommand::REQ_DISCONNECT))
        {
            std::cout << "Sending disconnect message" << std::endl;
        }
        else if (command_id == static_cast<CommandType>(ServerCommand::REQ_ALIVE))
        {
            std::cout << "Sending keepalive command." << std::endl;
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_GET_DATETIME))
        {
            std::cout << "Get datetime command not implemented yet." << std::endl;
            valid = false;
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_SET_DATETIME))
        {
            std::cout << "Set datetime command not implemented yet." << std::endl;
            valid = false;
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_GET_HOME_POSITION))
        {
            std::cout << "Sending get home position command." << std::endl;
        }
        else if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_SET_HOME_POSITION))
        {
            std::cout << "Sending set home position command." << std::endl;

            bool valid_params = true;
            double az = 0., el = 0.;
            char *param_token = std::strtok(nullptr, " ");

            try
            {
                az = std::stod(param_token);
            }
            catch (...)
            {
                std::cerr << "Bad parameter azimuth issued.";
                valid_params = false;
            }

            if (valid_params)
            {
                param_token = std::strtok(nullptr, " ");

                try
                {
                    el = std::stod(param_token);
                }
                catch (...)
                {
                    std::cerr << "Bad parameter elevation issued.";
                    valid_params = false;
                }
            }

            if (valid_params)
            {
                std::cout<<"Sending: " << az <<" "<<el<<std::endl;

                AltAzPos pos(az, el);

                BinarySerializer serializer;

                serializer.write(pos);

                std::cout<<serializer.toJsonString();

                command_msg.params_size = BinarySerializer::fastSerialization(command_msg.params, pos);

                std::cout<<std::endl;
            }
            else
            {
                std::cout<<"Sending invalid command: "<<std::endl;
                command_msg.params_size = BinarySerializer::fastSerialization(command_msg.params, az);

                valid_params = true;
            }

            valid = valid_params;

        }
        else
        {
            valid = false;
        }

        // TODO MOVE ALL OF THIS TO A SUBCLASS IN A PURE VIRTUAL. THE FUNCTION WILL RETURN ClientResult
        // TODO THE ERROR CONTROL MUST BE IN THE BASE CLIENT. THE SUBCLASS MUST CONTROL THE OUTPUT DATA AND CUSTOM ERRORS ONLY.
        // TODO DISABLE SEND WITH THIS WAY THE RESERVED COMMANDS.
        // TODO CREATE doConnect, doDisconnect, checkServerAlive
        // TODO CREATE IN THE CLIENT THE INTERNAL CALLBACKS LIKE THE SERVER.
        // TODO MOVE THE PROCESSING OF EACH COMPLEX RESPONSE TO A FUNCTION.

        if (valid)
        {
            // TODO MOVE ALL
            CommandReply reply;

            if(command_msg.command == ServerCommand::REQ_CONNECT)
            {
                client_result = client.doConnect();

                if (client_result == ClientResult::CLIENT_STOPPED)
                {
                    delete[] command_str;
                    return;
                }


            }
            else
                client_result = client.sendCommand(command_msg, reply);

            std::cerr << "Client Result: " << static_cast<int>(client_result)<<std::endl;

            if (client_result != ClientResult::COMMAND_OK)
            {
            }
            else
            {


                std::cout<<"Server result: "<<static_cast<int>(reply.result)<<std::endl;

                if(reply.result != ServerResult::COMMAND_OK)
                {
                    delete[] command_str;
                    return;
                }

                // Get the controller result.
                // TODO ERROR CONTROL

                if(command_id > static_cast<CommandType>(ServerCommand::END_BASE_COMMANDS))
                {
                    AmelasError error;

                    BinarySerializer ser(reply.params.get(), reply.params_size);
                    std::cout<<ser.toJsonString()<<std::endl;

                    ser.read(error);

                    std::cout<<"Controller error: "<<static_cast<int>(error)<<std::endl;
                }

                if (command_id == static_cast<CommandType>(AmelasServerCommand::REQ_GET_HOME_POSITION))
                {
                    try
                    {
                        AmelasError error;   // Trash. The controller error must be checked.
                        double az;
                        double el;

                        // Deserialize the parameters.
                        BinarySerializer::fastDeserialization(reply.params.get(), reply.params_size, error, az, el);

                        // Generate the struct.
                        std::cout<<"Az: "<<az<<std::endl;
                        std::cout<<"El: "<<el<<std::endl;
                    }
                    catch(...)
                    {
                        std::cout<<"BAD PARAMS"<<std::endl;
                        // RETURN BAD PARAMS
                        //result = ClientResult::
                    }
                }
            }
        }
        else
        {
            std::cerr << "Command is not implemented or valid" << std::endl;
        }

    }
    else
    {
        std::cerr << "Not a valid command" << std::endl;
    }


    delete[] command_str;
}

/**
 * @brief Main entry point of the program `ExampleClientAmelas`.
 */
int main(int, char**)
{
    // Configure the console.
    zmqutils::utils::ConsoleConfig& console_cfg = zmqutils::utils::ConsoleConfig::getInstance();
    console_cfg.configureConsole(true, false, false);

    // Configuration variables.
    unsigned port = 9999;
    std::string ip = "127.0.0.1";

    std::string endpoint = "tcp://" + ip + ":" + std::to_string(port);
    
    AmelasControllerClient client(endpoint, "AMELAS EXAMPLE CLIENT");

    // Configure the client.
    client.setAliveCallbacksEnabled(false);

    // Set the exit callback to the console handler for safety.
    console_cfg.setExitCallback(
            [&client]()
            {
                std::cout << std::endl;
                std::cout << "Stopping the client..." << std::endl;
                client.stopClient();
            });

    bool started = client.startClient();

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
        std::cout<<"-- Commands --"<<std::endl;
        std::cout<<"- REQ_CONNECT:    0"<<std::endl;
        std::cout<<"- REQ_DISCONNECT: 1"<<std::endl;
        std::cout<<"- REQ_ALIVE:      2"<<std::endl;
        std::cout<<"- CUSTOM:         cmd param1 param2 ..."<<std::endl;
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
        parseCommand(client, command);
    }

    // Final log.
    std::cout << "Client stoped. All ok!!" << std::endl;

    // Restore the console.
    console_cfg.restoreConsole();

	return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
