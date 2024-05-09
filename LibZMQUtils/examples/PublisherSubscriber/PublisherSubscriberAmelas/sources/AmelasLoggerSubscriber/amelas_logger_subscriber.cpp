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
 * @file logger_subscriber.cpp
 * @brief EXAMPLE FILE - This file contains the implementation of the AmelasAmelasLoggerSubscriber example class.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasLoggerSubscriber/amelas_logger_subscriber.h"
// =====================================================================================================================

// NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace communication{
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
using namespace controller;
// ---------------------------------------------------------------------------------------------------------------------

AmelasLoggerSubscriber::AmelasLoggerSubscriber()
{}

void AmelasLoggerSubscriber::addTopicFilter(const controller::AmelasLogLevel &log_level)
{
    zmqutils::pubsub::SubscriberBase::addTopicFilter(AmelasLoggerTopic[static_cast<size_t>(log_level)]);
}

void AmelasLoggerSubscriber::removeTopicFilter(const controller::AmelasLogLevel &log_level)
{
    zmqutils::pubsub::SubscriberBase::removeTopicFilter(AmelasLoggerTopic[static_cast<size_t>(log_level)]);
}

void AmelasLoggerSubscriber::onSubscriberStart()
{

    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout << "<LOGGER SUBSCRIBER>" << std::endl;
    std::cout << "-> ON SUBSCRIBER START: " << std::endl;
    std::cout << "Time: " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasLoggerSubscriber::onSubscriberStop()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout << "<LOGGER SUBSCRIBER>" << std::endl;
    std::cout << "-> ON SUBSCRIBER STOP: " << std::endl;
    std::cout << "Time: " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasLoggerSubscriber::onSubscriberError(const zmq::error_t &error, const std::string &ext_info)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout << "<LOGGER SUBSCRIBER>" << std::endl;
    std::cout << "-> ON SUBSCRIBER ERROR: " << std::endl;
    std::cout << "Time: " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout << "Code: " << error.num() << std::endl;
    std::cout << "Error: " << error.what() << std::endl;
    std::cout << "Info: " << ext_info << std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasLoggerSubscriber::onMsgReceived(const zmqutils::pubsub::PubSubMsg& msg,
                                           zmqutils::pubsub::SubscriberResult &res)
{
    // Log.
    zmqutils::serializer::BinarySerializer serializer(msg.data.data.get(), msg.data.data_size);
    std::cout << std::string(100, '-') << std::endl;
    std::cout << "<LOGGER SUBSCRIBER>" << std::endl;
    std::cout << "-> ON MSG RECEIVED: " << std::endl;
    std::cout << "Time: " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout << "Publisher UUID: " << msg.pub_info.uuid.toRFC4122String() << std::endl;
    std::cout << "Params Size: " << msg.data.data_size << std::endl;
    std::cout << "Params Hex: " << serializer.getDataHexString() << std::endl;
    std::cout << std::string(100, '-') << std::endl;

    // Call father method to execute error callback
    ClbkSubscriberBase::onMsgReceived(msg, res);

    std::cout << "Result on message processing is: " << static_cast<int>(res) << std::endl;
}

void AmelasLoggerSubscriber::onInvalidMsgReceived(const zmqutils::pubsub::PubSubMsg& msg,
                                                  zmqutils::pubsub::SubscriberResult res)
{
    // Log.
    zmqutils::serializer::BinarySerializer serializer(msg.data.data.get(), msg.data.data_size);
    std::cout << std::string(100, '-') << std::endl;
    std::cout << "<LOGGER SUBSCRIBER>" << std::endl;
    std::cout << "-> ON BAD MSG RECEIVED: " << std::endl;
    std::cout << "Time: "<< zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout << "Publisher UUID: " << msg.pub_info.uuid.toRFC4122String() << std::endl;
    std::cout << "Params Size: " << msg.data.data_size << std::endl;
    std::cout << "Params Hex: " << serializer.getDataHexString() << std::endl;
    std::cout << std::string(100, '-') << std::endl;

    // Call father method to execute error callback
    ClbkSubscriberBase::onInvalidMsgReceived(msg, res);
}

}} // END NAMESPACES.
// =====================================================================================================================

