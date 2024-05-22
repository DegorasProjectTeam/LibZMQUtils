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
#include <iostream>
#include <limits>
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/Utilities>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasLoggerSubscriber/amelas_logger_subscriber.h"
#include "AmelasLoggerPublisher/amelas_logger_publisher.h"
#include "AmelasController/amelas_log.h"
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
// ZMQ Utils Namsespaces.
using zmqutils::pubsub::OperationResult;
using zmqutils::serializer::BinarySerializer;
using zmqutils::pubsub::ResultType;
using zmqutils::pubsub::PublishedMessage;
// Amelas Nampesaces.
using amelas::communication::AmelasLoggerSubscriber;
using amelas::communication::AmelasLoggerTopic;
using amelas::controller::AmelasLog;
using amelas::controller::AmelasLogLevel;
// ---------------------------------------------------------------------------------------------------------------------

// Helper log processor class.
class AmelasLogProcessor
{

public:

    void processLogInfo(const AmelasLog &log)
    {
        std::cout << std::string(100, '-') << std::endl;
        std::cout << "<AMELAS LOG PROCESSOR>"<<std::endl;
        std::cout << "Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout << "New log: [INFO] - " << log.str_info << ". Size: " << log.serializedSize() << std::endl;
        std::cout << std::string(100, '-') << std::endl;
    }

    void processLogWarning(const AmelasLog &log)
    {
        std::cout << std::string(100, '-') << std::endl;
        std::cout << "<AMELAS LOG PROCESSOR>"<<std::endl;
        std::cout << "Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout << "New log: [WARNING] - " << log.str_info << ". Size: " << log.serializedSize() << std::endl;
        std::cout << std::string(100, '-') << std::endl;
    }

    void processLogError(const AmelasLog &log)
    {
        std::cout << std::string(100, '-') << std::endl;
        std::cout << "<AMELAS LOG PROCESSOR>"<<std::endl;
        std::cout << "Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout << "New log: [ERROR] - " << log.str_info << ". Size: " << log.serializedSize() << std::endl;
        std::cout << std::string(100, '-') << std::endl;
    }

    void processErrorCallback(const PublishedMessage&, OperationResult res)
    {
        std::cout << std::string(100, '-') << std::endl;
        std::cout << "<AMELAS LOG PROCESSOR>"<<std::endl;
        std::cout << "Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout << "Error callback with code: " << static_cast<ResultType>(res)
                  << " (" << AmelasLoggerSubscriber::operationResultToString(res) << ")" << std::endl;
        std::cout << std::string(100, '-') << std::endl;
    }
};

/**
 * @brief Main entry point of the program `ExampleLoggerSubscriberAmelas`.
 */
int main(int, char**)
{
    // Configure the console.
    zmqutils::utils::ConsoleConfig& console_cfg = zmqutils::utils::ConsoleConfig::getInstance();
    console_cfg.configureConsole(true, true, false);

    // Subscriber configuration variables.
    std::string subscriber_name = "AMELAS EXAMPLE SUBSCRIBER";       // Subscriber name.
    std::string subscriber_version = "1.7.6";                        // Subscriber version.
    std::string subscriber_info = "This is the AMELAS subscriber.";  // Subscriber information.

    // Publisher endpoint.
    std::string publisher_endpoint = "tcp://127.0.0.1:9999";

    // Configure the log processor.
    AmelasLogProcessor log_processor;

    // Instantiate the subscriber.
    AmelasLoggerSubscriber subscriber(subscriber_name, subscriber_version, subscriber_info);

    // Configure the subscriber.
    subscriber.subscribe(publisher_endpoint);
    subscriber.addTopicFilter(AmelasLogLevel::AMELAS_INFO);
    subscriber.addTopicFilter(AmelasLogLevel::AMELAS_WARNING);
    subscriber.addTopicFilter(AmelasLogLevel::AMELAS_ERROR);
    // This topic will be allowed in to test topics with no callbacks.
    subscriber.addTopicFilter(AmelasLogLevel::AMELAS_DEBUG);

    // Set the callbacks in the subscriber.

    subscriber.registerCallbackAndRequestProcFunc(AmelasLogLevel::AMELAS_INFO,
                                                  &log_processor,
                                                  &AmelasLogProcessor::processLogInfo);

    subscriber.registerCallbackAndRequestProcFunc(AmelasLogLevel::AMELAS_WARNING,
                                                  &log_processor,
                                                  &AmelasLogProcessor::processLogWarning);

    subscriber.registerCallbackAndRequestProcFunc(AmelasLogLevel::AMELAS_ERROR,
                                                  &log_processor,
                                                  &AmelasLogProcessor::processLogError);

    subscriber.setErrorCallback(&log_processor,
                                &AmelasLogProcessor::processErrorCallback);

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
