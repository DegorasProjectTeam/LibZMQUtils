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

struct EventMessage : public zmqutils::serializer::Serializable
{
    inline EventMessage() :
        channel(0), timetag(0)
    {}

    inline EventMessage(uint64_t ch, uint64_t ttag) :
        channel(ch), timetag(ttag)
    {}

    inline size_t serialize(zmqutils::serializer::BinarySerializer& serializer) const final
    {
        return serializer.write(this->channel, this->timetag);
    }

    inline void deserialize(zmqutils::serializer::BinarySerializer& serializer) final
    {
        serializer.read(this->channel, this->timetag);
    }

    inline size_t serializedSize() const final
    {
        return Serializable::calcSizeHelper(this->channel, this->timetag);
    }

    // Convert the object to a JSON string
    inline std::string toJsonString() const
    {
        std::ostringstream oss;
        oss << "{"
            << "\"channel\":" << channel << ","
            << "\"timetag\":" << timetag
            << "}";
        return oss.str();
    }

    uint64_t channel;
    uint64_t timetag;
};

class SubscriberCallbackHandler
{

public:

    inline SubscriberCallbackHandler()
    {
    }

    inline void handleMsg(const std::string &plc_time, const std::string &server_time)
    {
        std::cout<<"NEW PLC TIME: " + plc_time << std::endl;
        std::cout<<"NEW SERVER TIME: " + server_time << std::endl;
    }

    std::unique_ptr<zmqutils::utils::DebugConsole> console_;
};

class AmelasSubscriberCallbackHandler
{

public:

    inline AmelasSubscriberCallbackHandler()
    {
        this->console_ = std::make_unique<zmqutils::utils::DebugConsole>("AMELAS SUBSCRIBER CMD");
        this->console_->startProcess();
    }

    inline void handleMsg(const std::string &plc_time, const std::string &server_time)
    {
        this->console_->sendString("NEW PLC TIME: " + plc_time);
        this->console_->sendString("NEW SERVER TIME: " + server_time);
    }

    std::unique_ptr<zmqutils::utils::DebugConsole> console_;
};

class ESTTSubscriberCallbackHandler
{

public:

    inline ESTTSubscriberCallbackHandler()
    {
        this->console_ = std::make_unique<zmqutils::utils::DebugConsole>("ESTT SUBSCRIBER CMD");
        this->console_->startProcess();
    }

    inline void handleMsg(const std::vector<EventMessage>& events)
    {
        std::cout<<"HERE"<<std::endl;
        for(const auto& event : events)
            this->console_->sendString("NEW ESTT EVENT: " + event.toJsonString());
    }

    std::unique_ptr<zmqutils::utils::DebugConsole> console_;
};

/**
 * @brief Main entry point of the program `ExampleLoggerSubscriberAmelas`.
 */
int main(int, char**)
{
    class TestSubscriber : public zmqutils::pubsub::ClbkSubscriberBase
    {
    private:

        using zmqutils::pubsub::ClbkSubscriberBase::ClbkSubscriberBase;

        inline void onSubscriberStart() override {}

        inline void onSubscriberStop() override {}

        inline void onSubscriberError(const zmq::error_t &, const std::string &) override {}
    };

    // Configure the console.
    zmqutils::utils::ConsoleConfig& console_cfg = zmqutils::utils::ConsoleConfig::getInstance();
    console_cfg.configureConsole(true, true, false);

    // Subscriber configuration variables.
    std::string subscriber_name = "AMELAS EXAMPLE SUBSCRIBER";       // Subscriber name.
    std::string subscriber_version = "1.7.6";                        // Subscriber version.
    std::string subscriber_info = "This is the AMELAS subscriber.";  // Subscriber information.

    // Publisher endpoint.
    //std::string publisher_endpoint = "tcp://127.0.0.1:9999";
    std::string amelas_pub_endpoint = "tcp://192.168.3.244:9998";
    std::string estt_pub_endpoint = "tcp://192.168.1.201:9999";

    // Configure the log processor.
    AmelasLogProcessor log_processor;

    // Instantiate the amelas_sub.
    //AmelasLoggerSubscriber amelas_sub(subscriber_name, subscriber_version, subscriber_info);
    TestSubscriber amelas_sub(subscriber_name, subscriber_version, subscriber_info);
    TestSubscriber estt_sub("", "", "");
    AmelasSubscriberCallbackHandler amelas_handler;
    ESTTSubscriberCallbackHandler estt_handler;

    // Configure the subscriber.
    amelas_sub.subscribe(amelas_pub_endpoint);
    amelas_sub.addTopicFilter("NEW_TIME");

    estt_sub.subscribe(estt_pub_endpoint);
    estt_sub.addTopicFilter("ESTT_EVENTS");

    //subscriber.addTopicFilter(AmelasLogLevel::AMELAS_INFO);
    //subscriber.addTopicFilter(AmelasLogLevel::AMELAS_WARNING);
    //subscriber.addTopicFilter(AmelasLogLevel::AMELAS_ERROR);
    // This topic will be allowed in to test topics with no callbacks.
    //subscriber.addTopicFilter(AmelasLogLevel::AMELAS_DEBUG);

    // Set the callbacks in the subscriber.
    /*
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
    */

    amelas_sub.registerCbAndReqProcFunc<std::function<void(const std::string&, const std::string&)>>(
        "NEW_TIME", &amelas_handler, &AmelasSubscriberCallbackHandler::handleMsg);

    estt_sub.registerCbAndReqProcFunc<std::function<void(const std::vector<EventMessage>& events)>>(
        "ESTT_EVENTS", &estt_handler, &ESTTSubscriberCallbackHandler::handleMsg);

    // Start the amelas_sub.
    bool started = amelas_sub.startSubscriber();

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
    amelas_sub.stopSubscriber();

    // Final log.
    std::cout << "Subscriber stoped. All ok!!" << std::endl;

    // Restore the console.
    console_cfg.restoreConsole();

    // Return.
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------------
