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
 * @file logger_subscriber.cpp
 * @brief EXAMPLE FILE - This file contains the implementation of the LoggerSubscriber example class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// PROJECT INCLUDES
// =====================================================================================================================
#include "includes/LoggerSubscriber/logger_subscriber.h"
// =====================================================================================================================

// NAMESPACES
// =====================================================================================================================
namespace logger{
// =====================================================================================================================

LoggerSubscriber::LoggerSubscriber()
{
    // Register each internal specific process function in the base subscriber.

    // Process log info
    this->registerRequestProcFunc("LOG_INFO", this,
                                  &LoggerSubscriber::processLogMsg);

    // Process log info
    this->registerRequestProcFunc("LOG_WARNING", this,
                                  &LoggerSubscriber::processLogMsg);

    // Process log info
    this->registerRequestProcFunc("LOG_ERROR", this,
                                  &LoggerSubscriber::processLogMsg);

}

zmqutils::SubscriberResult LoggerSubscriber::processLogMsg(const zmqutils::common::PubSubMsg& msg)
{
    std::string message_str;

    // Check the request parameters size.
    if (msg.data.data_size == 0 || !msg.data.data)
    {
        return zmqutils::SubscriberResult::EMPTY_PARAMS;
    }

    // Try to read the parameters data.
    try
    {
        zmqutils::utils::BinarySerializer::fastDeserialization(msg.data.data.get(), msg.data.data_size, message_str);
    }
    catch(...)
    {
        return zmqutils::SubscriberResult::INVALID_MSG;
    }

    // Now we will process the command in the controller.
    return this->invokeCallback<LogMsgCallback, zmqutils::common::SubscriberResult>(msg, message_str);
}



void LoggerSubscriber::onSubscriberStart()
{

    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout << "<LOGGER SUBSCRIBER>" << std::endl;
    std::cout << "-> ON SUBSCRIBER START: " << std::endl;
    std::cout << "Time: " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void LoggerSubscriber::onSubscriberStop()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout << "<LOGGER SUBSCRIBER>" << std::endl;
    std::cout << "-> ON SUBSCRIBER STOP: " << std::endl;
    std::cout << "Time: " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void LoggerSubscriber::onSubscriberError(const zmq::error_t &error, const std::string &ext_info)
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

zmqutils::SubscriberResult LoggerSubscriber::onMsgReceived(const zmqutils::common::PubSubMsg& msg)
{
    // Log.
    zmqutils::utils::BinarySerializer serializer(msg.data.data.get(), msg.data.data_size);
    std::cout << std::string(100, '-') << std::endl;
    std::cout << "<LOGGER SUBSCRIBER>" << std::endl;
    std::cout << "-> ON MSG RECEIVED: " << std::endl;
    std::cout << "Time: " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout << "Publisher UUID: " << msg.pub_info.uuid.toRFC4122String() << std::endl;
    std::cout << "Params Size: " << msg.data.data_size << std::endl;
    std::cout << "Params Hex: " << serializer.getDataHexString() << std::endl;
    std::cout << std::string(100, '-') << std::endl;

    auto result = SubscriberBase::onMsgReceived(msg);

    std::cout << "Result on message processing is: " << static_cast<int>(result) << std::endl;

    return result;
}

void LoggerSubscriber::onInvalidMsgReceived(const zmqutils::common::PubSubMsg& msg, zmqutils::SubscriberResult)
{
    // Log.
    zmqutils::utils::BinarySerializer serializer(msg.data.data.get(), msg.data.data_size);
    std::cout << std::string(100, '-') << std::endl;
    std::cout << "<LOGGER SUBSCRIBER>" << std::endl;
    std::cout << "-> ON BAD MSG RECEIVED: " << std::endl;
    std::cout << "Time: "<< zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout << "Publisher UUID: " << msg.pub_info.uuid.toRFC4122String() << std::endl;
    std::cout << "Params Size: " << msg.data.data_size << std::endl;
    std::cout << "Params Hex: " << serializer.getDataHexString() << std::endl;
    std::cout << std::string(100, '-') << std::endl;
}



} // END NAMESPACES.
// =====================================================================================================================

