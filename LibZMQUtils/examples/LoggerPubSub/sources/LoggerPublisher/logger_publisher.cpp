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
 * @file logger_publisher.cpp
 * @brief EXAMPLE FILE - This file contains the definition of the LoggerPublisher example class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

#include "includes/LoggerPublisher/logger_publisher.h"

// NAMESPACES
// =====================================================================================================================
namespace logger {
// =====================================================================================================================

LoggerPublisher::LoggerPublisher(std::string endpoint,
                                 std::string name) :
    PublisherBase(std::move(endpoint), std::move(name))
{}

zmqutils::PublisherResult LoggerPublisher::sendInfoLog(const std::string &msg)
{
    return this->sendMsg(this->prepareData("LOG_INFO", msg));
}

zmqutils::PublisherResult LoggerPublisher::sendWarningLog(const std::string &msg)
{
    return this->sendMsg(this->prepareData("LOG_WARNING", msg));
}

zmqutils::PublisherResult LoggerPublisher::sendErrorLog(const std::string &msg)
{
    return this->sendMsg(this->prepareData("LOG_ERROR", msg));
}

zmqutils::common::PubSubData LoggerPublisher::prepareData(const std::string &topic, const std::string &msg_string)
{
    zmqutils::common::PubSubData data;

    data.topic = topic;
    data.data_size = zmqutils::utils::BinarySerializer::fastSerialization(data.data, msg_string);

    return data;
}

void LoggerPublisher::onPublisherStart()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout << "<"<<this->getName() << ">" <<std::endl;
    std::cout << "-> ON PUBLISHER START: " << std::endl;
    std::cout << "Time: " << zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << "Endpoint: " << this->getEndpoint() << std::endl;
    std::cout << "Name: " << this->getName() << std::endl;
    std::cout << "UUID: " << this->getUUID().toRFC4122String() << std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void LoggerPublisher::onPublisherStop()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getName()<<">"<<std::endl;
    std::cout<<"-> ON PUBLISHER STOP: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void LoggerPublisher::onSendingMsg(const zmqutils::common::PubSubData &req)
{
    zmqutils::utils::BinarySerializer serializer(req.data.get(), req.data_size);
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout << "<"<<this->getName() << ">" << std::endl;
    std::cout << "-> ON PUBLISHER SEND COMMAND: " << std::endl;
    std::cout << "Time: " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout << "Topic: " << req.topic << std::endl;
    std::cout << "Params size: " << req.data_size <<std::endl;
    std::cout << "Params Hex: " << serializer.getDataHexString() << std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void LoggerPublisher::onPublisherError(const zmq::error_t& error, const std::string& ext_info)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getName()<<">"<<std::endl;
    std::cout<<"-> ON PUBLISHER ERROR: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Code: "<<error.num()<<std::endl;
    std::cout<<"Error: "<<error.what()<<std::endl;
    std::cout<<"Info: "<<ext_info<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

}  // END NAMESPACES.
// =====================================================================================================================

