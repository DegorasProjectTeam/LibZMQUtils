/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
 *   ExamplesLibZMQUtils related project.                                                                              *
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
 * @example ExampleLoggerPublisherAmelas.cpp
 *
 * @brief This file serves as a program example of how to use the `AmelasLoggerPublisher` class.
 *
 * This program initializes an instance of the `AmelasLoggerPublisher` class to interact with an instance of
 * `AmelasLoggerSubscriber` class.
 *
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
#include <cstring>
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/Utilities>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasLoggerPublisher/amelas_logger_publisher.h"
#include "AmelasController/amelas_log.h"
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
// ZMQ Utils Namsespaces.
using zmqutils::pubsub::OperationResult;
using zmqutils::serializer::BinarySerializer;
// Amelas Nampesaces.
using amelas::communication::AmelasLoggerPublisher;
using amelas::controller::AmelasLog;
using amelas::controller::AmelasLogLevel;
// ---------------------------------------------------------------------------------------------------------------------

class AmelasLoggerPublisherCmdParser
{
public:

    AmelasLoggerPublisherCmdParser(AmelasLoggerPublisher& publisher) : publisher_(publisher)
    {}

    // Helper parse command function.
    void parseCommand(const std::string &command)
    {
        char *command_str = new char[command.size() + 1];
        std::copy(command.begin(), command.end(), command_str);
        command_str[command.size()] = '\0';
        char *token = std::strtok(command_str, " ");

        if (token)
        {
            std::string token_command(token);

            token = std::strtok(nullptr, "");

            if (!token)
            {
                std::cerr << "There is no message to send" << std::endl;
                delete[] command_str;
                return;
            }

            std::string token_msg(token);
            OperationResult res = OperationResult::INVALID_MSG;
            AmelasLog log;

            if (token_command == "info")
            {
                std::cout << "Sending info log with msg: " << token_msg << std::endl;
                log.level = AmelasLogLevel::AMELAS_INFO;
                log.str_info = token_msg;
                res = this->publisher_.sendLog(log);
            }
            else if (token_command == "debug")
            {
                std::cout << "Sending debug log with msg: " << token_msg << std::endl;
                log.level = AmelasLogLevel::AMELAS_DEBUG;
                log.str_info = token_msg;
                res = this->publisher_.sendLog(log);
            }
            else if (token_command == "warning")
            {
                std::cout << "Sending warning log with msg: " << token_msg << std::endl;
                log.level = AmelasLogLevel::AMELAS_WARNING;
                log.str_info = token_msg;
                res = this->publisher_.sendLog(log);
            }
            else if (token_command == "error")
            {
                std::cout << "Sending error log with msg: " << token_msg << std::endl;
                log.level = AmelasLogLevel::AMELAS_ERROR;
                log.str_info = token_msg;
                res = this->publisher_.sendLog(log);
            }
            else
            {
                std::cerr << "Failed at sending log message. Unknown type." << std::endl;
                delete[] command_str;
                return;
            }

            if (res != OperationResult::MSG_OK)
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

    AmelasLoggerPublisher& publisher_;
};


/**
 * @brief Main entry point of the program `ExampleLoggerPublisherAmelas`.
 */
int main(int, char**)
{
    // Configure the console.
    zmqutils::utils::ConsoleConfig& console_cfg = zmqutils::utils::ConsoleConfig::getInstance();
    console_cfg.configureConsole(true, false, true);

    // Publisher configuration variables.
    unsigned publisher_port = 9999;                                // Publisher connection port.
    std::string publisher_iface = "*";                             // Publisher network interface.
    std::string publisher_name = "AMELAS EXAMPLE PUBLISHER";       // Publisher name.
    std::string publisher_version = "1.7.6";                       // Publisher version.
    std::string publisher_info = "This is the AMELAS publisher.";  // Publisher information.

    // Instanciate the publisher.
    AmelasLoggerPublisher pub(publisher_port, publisher_iface, publisher_name, publisher_version, publisher_info);

    // Prepare the auxiliar testing parser.
    AmelasLoggerPublisherCmdParser publisher_parser(pub);

    // Set the exit callback to the console handler for safety.
    console_cfg.setExitCallback(
            [&pub]()
            {
                std::cout << std::endl;
                std::cout << "Stopping the publisher..." << std::endl;
                pub.stopPublisher();
            });

    // Start the publisher.
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
        std::cout<<"-- Topics --"<<std::endl;
        std::cout<<"- AMELAS_INFO:    info msg"<<std::endl;
        std::cout<<"- AMELAS_DEBUG:   debug msg <(not subscribed, for testing.>"<<std::endl;
        std::cout<<"- AMELAS_WARNING: warning msg"<<std::endl;
        std::cout<<"- AMELAS_ERROR:   error msg"<<std::endl;
        std::cout<<"-- Other --"<<std::endl;
        std::cout<<"- Publisher exit: exit"<<std::endl;
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

        // Break if we want to close the example program.
        if(console_cfg.closeStatus() || std::cin.eof())
        {
            console_cfg.waitForClose();
            break;
        }

        // Parse the command.
        publisher_parser.parseCommand(command);
    }

    // Final log.
    std::cout << "Publisher stoped. All ok!!" << std::endl;

    // Restore the console.
    console_cfg.restoreConsole();

    // Return.
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
