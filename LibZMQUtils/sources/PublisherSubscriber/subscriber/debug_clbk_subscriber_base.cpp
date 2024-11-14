/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
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
 * @file debug_clbk_subscriber_base.cpp
 * @brief This file contains the implementation of the DebugClbkSubscriberBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/PublisherSubscriber/subscriber/debug_clbk_subscriber_base.h"
#include "LibZMQUtils/Utilities/utils.h"
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

DebugClbkSubscriberBase::DebugClbkSubscriberBase(const std::string &subscriber_name,
                                                 const std::string &subscriber_version,
                                                 const std::string &subscriber_info,
                                                 bool log_internal_callbacks) :
    ClbkSubscriberBase(subscriber_name, subscriber_version, subscriber_info),
    log_internal_callbacks_(log_internal_callbacks)
{}

std::string DebugClbkSubscriberBase::generateStringHeader(const std::string &clbk_name,
                                                          const std::vector<std::string>& data)
{
    if(!this->log_internal_callbacks_)
        return std::string();

    // Log.
    std::stringstream ss;
    ss << std::string(100, '-')                                              << std::endl;
    ss << "<" << this->getSubscriberInfo().name << ">"                       << std::endl;
    ss << "-> TIME: " << zmqutils::utils::currentISO8601Date()               << std::endl;
    ss << "-> " << clbk_name                                                 << std::endl;

    for(const auto& str : data)
    {
        ss << std::string(20, '-')                                           << std::endl;
        ss << str                                                            << std::endl;
    }
    ss << std::string(100, '-')                                              << std::endl;
    return ss.str();
}

void DebugClbkSubscriberBase::onSubscriberStart()
{
    // Log.
    std::cout << this->generateStringHeader("ON SUBSCRIBER START", {this->getSubscriberInfo().toString()});
}

void DebugClbkSubscriberBase::onSubscriberStop()
{
    // Log.
    std::cout << this->generateStringHeader("ON SUBSCRIBER STOP", {});
}

void DebugClbkSubscriberBase::onSubscriberError(const zmq::error_t &error, const std::string &ext_info)
{
    // Log.
    std::stringstream data;
    data << "Code:  " << std::to_string(error.num())                         << std::endl;
    data << "Error: " << error.what()                                        << std::endl;
    data << "Info:  " << ext_info;
    std::cout << this->generateStringHeader("ON SUBSCRIBER ERROR", {data.str()});
}

void DebugClbkSubscriberBase::onInvalidMsgReceived(const PublishedMessage& msg, OperationResult res)
{
    // Log.
    serializer::BinarySerializer serializer(msg.data.bytes.get(), msg.data.size);
    std::stringstream data;
    data << msg.pub_info.toString()                           << std::endl;
    data << std::string(20, '-')                              << std::endl;
    data << "Topic:       " << msg.topic                      << std::endl;
    data << "Timestamp:   " << msg.timestamp                  << std::endl;
    data << "Result:      " << static_cast<ResultType>(res)
         << " (" << operationResultToString(res) << ")"       << std::endl;
    data << "Params size: " << msg.data.size                  << std::endl;
    data << "Params Hex:  " << serializer.getDataHexString();
    std::cout << this->generateStringHeader("ON INVALID MSG RECEIVED", {data.str()});

    // Call parent method.
    ClbkSubscriberBase::onInvalidMsgReceived(msg, res);
}

void DebugClbkSubscriberBase::onMsgReceived(const PublishedMessage &msg, OperationResult res)
{
    // Log.
    serializer::BinarySerializer serializer(msg.data.bytes.get(), msg.data.size);
    std::stringstream data;
    data << msg.pub_info.toString()                           << std::endl;
    data << std::string(20, '-')                              << std::endl;
    data << "Topic:       " << msg.topic                      << std::endl;
    data << "Timestamp:   " << msg.timestamp                  << std::endl;
    data << "Result:      " << static_cast<ResultType>(res)
         << " (" << operationResultToString(res) << ")"       << std::endl;
    data << "Params size: " << msg.data.size                  << std::endl;
    data << "Params Hex:  " << serializer.getDataHexString();
    std::cout << this->generateStringHeader("ON MSG RECEIVED", {data.str()});

    // Call parent method.
    ClbkSubscriberBase::onMsgReceived(msg, res);
}

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
