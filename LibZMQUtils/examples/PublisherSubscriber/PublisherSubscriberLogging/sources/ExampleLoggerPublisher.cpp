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
 * @example ExampleLoggerPublisher.cpp
 *
 * @brief This file serves as a program example of how to use the `LoggerPublisher` class.
 *
 * This program initializes an instance of the `LoggerPublisher` class to interact with an `LoggerSubscriber`.
 *
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
#include <cstring>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/Utils>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "includes/LoggerPublisher/logger_publisher.h"
// =====================================================================================================================

using zmqutils::pubsub::PublisherResult;
using zmqutils::utils::BinarySerializer;

void parseCommand(logger::LoggerPublisher &pub, const std::string &command)
{

    char *command_str = new char[command.size() + 1];
    std::copy(command.begin(), command.end(), command_str);
    command_str[command.size()] = '\0';

    char *token = std::strtok(command_str, " ");

    if (token)
    {
        std::string token_command(token);
        if (token_command != "info" && token_command != "warning" && token_command != "error")
        {
            std::cerr << "Failed at sending log message. Unknown type." << std::endl;
            delete[] command_str;
            return;
        }

        token = std::strtok(nullptr, "");

        if (!token)
        {
            std::cerr << "There is no message to send" << std::endl;
            delete[] command_str;
            return;
        }

        std::string token_msg(token);
        PublisherResult res = PublisherResult::INVALID_MSG;

        if (token_command == "info")
        {
            std::cout << "Sending info log with msg: " << token_msg << std::endl;
            res = pub.sendInfoLog(token_msg);
        }
        else if (token_command == "warning")
        {
            std::cout << "Sending warning log with msg: " << token_msg << std::endl;
            res = pub.sendWarningLog(token_msg);
        }
        else if (token_command == "error")
        {
            std::cout << "Sending error log with msg: " << token_msg << std::endl;
            res = pub.sendErrorLog(token_msg);
        }

        if (res != PublisherResult::MSG_OK)
        {
            std::cerr << "Error at sending log message. Error reason: " << static_cast<int>(res) << std::endl;
        }
    }
    else
    {
        std::cerr << "Not a valid command" << std::endl;
    }


    delete[] command_str;
}

/**
 * @brief Main entry point of the program `ExampleLoggerPublisher`.
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
    
    logger::LoggerPublisher pub(endpoint, "Log Publisher");

    // Set the exit callback to the console handler for safety.
    console_cfg.setExitCallback(
            [&pub]()
            {
                std::cout << std::endl;
                std::cout << "Stopping the publisher..." << std::endl;
                pub.stopPublisher();
            });

    bool started = pub.startPublisher();

    if(!started)
    {
        std::cout<<"Unable to start the publisher.";
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
        std::cout<<"- info:          Send info msg."<<std::endl;
        std::cout<<"- warning:       Send warning msg."<<std::endl;
        std::cout<<"- error:         Send error msg"<<std::endl;
        std::cout<<"-- Other --"<<std::endl;
        std::cout<<"- Exit:             exit"<<std::endl;
        std::cout<<"------------------------------------------------------"<<std::endl;
        std::cout<<"Write a command: ";
        std::getline(std::cin, command);

        // Check for other commands.
        if(command == "exit")
        {
            std::cout << "Stopping the publisher..." << std::endl;
            pub.stopPublisher();
            break;
        }
        else
            parseCommand(pub, command);


        // Break if we want to close the example program.
        if(console_cfg.closeStatus() || std::cin.eof())
        {
            console_cfg.waitForClose();
            break;
        }

    }

    // Final log.
    std::cout << "Publisher stoped. All ok!!" << std::endl;

    // Restore the console.
    console_cfg.restoreConsole();

	return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
