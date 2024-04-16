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
 * @example ExampleLoggerSubscriberAmelas.cpp
 *
 * @brief This file serves as a program example of how to use the `AmelasLoggerSubscriber` class.
 *
 * This program initializes an instance of the `AmelasLoggerSubscriber` class to interact with an instance of
 * `AmelasLoggerPublisher` class.
 *
 * @author Degoras Project Team
 * @copyright EUPL License
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

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/Utils>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasLoggerSubscriber/amelas_logger_subscriber.h"
#include "AmelasLoggerPublisher/amelas_logger_publisher.h"
#include "AmelasController/amelas_log.h"
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
using zmqutils::pubsub::SubscriberResult;
using zmqutils::utils::BinarySerializer;
using amelas::communication::AmelasLoggerSubscriber;
using amelas::communication::AmelasLoggerTopic;
using amelas::controller::AmelasLog;
using amelas::controller::AmelasLogLevel;
// ---------------------------------------------------------------------------------------------------------------------

// Helper log processor class.
class LogProcessor
{

public:

    SubscriberResult processLogInfo(const AmelasLog &log)
    {
        std::cout << "[INFO] - " << log.str_info << ". Size of log: " << log.serializedSize() << std::endl;
        return SubscriberResult::MSG_OK;
    }

    SubscriberResult processLogWarning(const AmelasLog &log)
    {
        std::cout << "[WARNING] - " << log.str_info << std::endl;
        return zmqutils::pubsub::SubscriberResult::MSG_OK;
    }

    zmqutils::pubsub::SubscriberResult processLogError(const AmelasLog &log)
    {
        std::cout << "[ERROR] - " << log.str_info << std::endl;
        return zmqutils::pubsub::SubscriberResult::MSG_OK;
    }
};

/**
 * @brief Main entry point of the program `ExampleLoggerSubscriberAmelas`.
 */
int main(int, char**)
{
    // Configure the console.
    zmqutils::utils::ConsoleConfig& console_cfg = zmqutils::utils::ConsoleConfig::getInstance();
    console_cfg.configureConsole(true, true, true);

    // Configure the log processor.
    LogProcessor log_processor;

    // Instantiate and configure subscriber.
    AmelasLoggerSubscriber subscriber;
    subscriber.subscribe("tcp://127.0.0.1:9999");
    subscriber.addTopicFilter(AmelasLogLevel::AMELAS_INFO);
    subscriber.addTopicFilter(AmelasLogLevel::AMELAS_WARNING);
    subscriber.addTopicFilter(AmelasLogLevel::AMELAS_ERROR);

    // Set the callbacks in the subscriber.
    subscriber.registerCallback(AmelasLogLevel::AMELAS_INFO, &log_processor, &LogProcessor::processLogInfo);
    subscriber.registerCallback(AmelasLogLevel::AMELAS_WARNING, &log_processor, &LogProcessor::processLogWarning);
    subscriber.registerCallback(AmelasLogLevel::AMELAS_ERROR, &log_processor, &LogProcessor::processLogError);

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
