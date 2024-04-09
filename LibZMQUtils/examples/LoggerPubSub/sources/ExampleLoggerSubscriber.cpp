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
 * @example ExampleLoggerSubscriber.cpp
 *
 * @brief This file serves as a program example of how to use the subscriber.
 *
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif
#include <iostream>
#include <limits>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Utils>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "includes/LoggerSubscriber/logger_subscriber.h"
// =====================================================================================================================

class Logger
{
public:
    zmqutils::pubsub::SubscriberResult processLogInfo(const std::string &msg)
    {
        std::cout << "[INFO] - " << msg << std::endl;
        return zmqutils::pubsub::SubscriberResult::MSG_OK;
    }
    zmqutils::pubsub::SubscriberResult processLogWarning(const std::string &msg)
    {
        std::cout << "[WARNING] - " << msg << std::endl;
        return zmqutils::pubsub::SubscriberResult::MSG_OK;
    }
    zmqutils::pubsub::SubscriberResult processLogError(const std::string &msg)
    {
        std::cout << "[ERROR] - " << msg << std::endl;
        return zmqutils::pubsub::SubscriberResult::MSG_OK;
    }
};


int main(int, char**)
{

    // Configure the console.
    zmqutils::utils::ConsoleConfig& console_cfg = zmqutils::utils::ConsoleConfig::getInstance();
    console_cfg.configureConsole(true, true, true);

    Logger log;


    // Instantiate and configure subscriber.
    logger::LoggerSubscriber subscriber;
    subscriber.subscribe("tcp://127.0.0.1:9999");
    subscriber.addTopicFilter("LOG_INFO");
    subscriber.addTopicFilter("LOG_WARNING");
    subscriber.addTopicFilter("LOG_ERROR");


    // ---------------------------------------
    // Set the callbacks in the subscriber.
    // ---------------------------------------
    subscriber.registerCallback("LOG_INFO", &log, &Logger::processLogInfo);
    subscriber.registerCallback("LOG_WARNING", &log, &Logger::processLogWarning);
    subscriber.registerCallback("LOG_ERROR", &log, &Logger::processLogError);


    // Start the subscriber.
    bool started = subscriber.startSubscriber();

    // Check if the subscriber starts ok.
    if(!started)
    {
        // Log.
        std::cout << "Subscriber start failed!! Press Enter to exit!" << std::endl;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.clear();
        return 1;
    }

    // Wait for closing as an infinite loop until ctrl-c.
    console_cfg.waitForClose();

    // Log.
    std::cout << "Stopping the subscriber..." << std::endl;

    // Stop the subscriber.
    subscriber.stopSubscriber();

    // Final log.
    std::cout << "Subscriber stoped. All ok!!" << std::endl;

    // Restore the console.
    console_cfg.restoreConsole();

    // Return.
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
