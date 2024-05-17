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
 * @file amelas_logger_publisher.cpp
 * @brief EXAMPLE FILE - This file contains the definition of the AmelasLoggerPublisher example class.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasLoggerPublisher/amelas_logger_publisher.h"
// =====================================================================================================================

// NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace communication{
// =====================================================================================================================

zmqutils::pubsub::PublisherResult AmelasLoggerPublisher::sendLog(const controller::AmelasLog &log)
{
    std::string log_topic = AmelasLoggerTopic[static_cast<size_t>(log.level)];
    return this->sendMsg(log_topic, log);
}

void AmelasLoggerPublisher::onPublisherStart()
{
    // Log.
    std::cout<< std::string(100, '-') << std::endl;
    std::cout<< "<"<<this->getPublisherInfo().name << ">"                        << std::endl;
    std::cout<< "-> ON PUBLISHER START: "                                        << std::endl;
    std::cout<< "Time:      " << zmqutils::utils::currentISO8601Date()           << std::endl;
    std::cout<< "Addresses: " << this->getPublisherIpsStr(" - ")                 << std::endl;
    std::cout<< "Port:      " << this->getPublisherInfo().port                   << std::endl;
    std::cout<< "UUID:      " << this->getPublisherInfo().uuid.toRFC4122String() << std::endl;
    std::cout<< "Endpoint:  " << this->getPublisherInfo().endpoint               << std::endl;
    std::cout<< "Hostname:  " << this->getPublisherInfo().hostname               << std::endl;
    std::cout<< "Name:      " << this->getPublisherInfo().name                   << std::endl;
    std::cout<< "Info:      " << this->getPublisherInfo().info                   << std::endl;
    std::cout<< "Version:   " << this->getPublisherInfo().version                << std::endl;
    std::cout<< std::string(100, '-')                                            << std::endl;
}

void AmelasLoggerPublisher::onPublisherStop()
{
    // Log.
    std::cout<< std::string(100, '-')                                 << std::endl;
    std::cout<< "<"<<this->getPublisherInfo().name << ">"             << std::endl;
    std::cout<< "-> ON PUBLISHER STOP: "                              << std::endl;
    std::cout<< "Time:     " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< std::string(100, '-')                                 << std::endl;
}

void AmelasLoggerPublisher::onSendingMsg(const zmqutils::pubsub::PubSubData &req)
{
    zmqutils::serializer::BinarySerializer serializer(req.data.get(), req.data_size);
    // Log.
    std::cout<< std::string(100, '-') << std::endl;
    std::cout<< "<" << this->getPublisherInfo().name << ">"       << std::endl;
    std::cout<< "-> ON PUBLISHER SEND COMMAND: "                  << std::endl;
    std::cout<< "Time: " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< "Topic: " << req.topic                            << std::endl;
    std::cout<< "Params size: " << req.data_size                  << std::endl;
    std::cout<< "Params Hex: " << serializer.getDataHexString()   << std::endl;
    std::cout<< std::string(100, '-')                             << std::endl;
}

void AmelasLoggerPublisher::onPublisherError(const zmq::error_t& error, const std::string& ext_info)
{
    // Log.
    std::cout<< std::string(100, '-')                              << std::endl;
    std::cout<< "<" << this->getPublisherInfo().name << ">"        << std::endl;
    std::cout<< "-> ON PUBLISHER ERROR: "                          << std::endl;
    std::cout<< "Time:  " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< "Code:  " << error.num()                           << std::endl;
    std::cout<< "Error: " << error.what()                          << std::endl;
    std::cout<< "Info:  " << ext_info                              << std::endl;
    std::cout<< std::string(100, '-')                              << std::endl;
}

}}  // END NAMESPACES.
// =====================================================================================================================
