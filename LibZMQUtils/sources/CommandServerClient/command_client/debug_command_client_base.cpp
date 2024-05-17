/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
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
 * @file debug_command_server_base.cpp
 * @brief This file contains the implementation of the DebugCommandServerBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <string>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/CommandServerClient/command_client/debug_command_client_base.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace reqrep{
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
using zmqutils::reqrep::ServerCommand;
using zmqutils::reqrep::OperationResult;
using zmqutils::reqrep::ResultType;
using zmqutils::reqrep::CommandType;
using zmqutils::reqrep::CommandRequest;
using zmqutils::reqrep::CommandReply;
using zmqutils::reqrep::CommandClientInfo;
using zmqutils::serializer::BinarySerializer;
// ---------------------------------------------------------------------------------------------------------------------

std::string DebugCommandClientBase::generateStringHeader(const std::string &clbk_name,
                                                         const std::vector<std::string>& data)
{
    std::stringstream ss;
    ss << std::string(100, '-') << std::endl;
    ss << "<" << this->getClientInfo().name << ">" << std::endl;
    ss << "-> TIME: " << zmqutils::utils::currentISO8601Date() << std::endl;
    ss << "-> " << clbk_name << std::endl;
    for(const auto& str : data)
    {
        ss << std::string(20, '-') << std::endl;
        ss << str << std::endl;
    }
    ss << std::string(100, '-') << std::endl;
    return ss.str();
}

void DebugCommandClientBase::onClientStart()
{
    // Log.
    std::cout << this->generateStringHeader("ON CLIENT START", {this->getClientInfo().toString()});
}

void DebugCommandClientBase::onClientStop()
{
    // Log.
    std::cout << this->generateStringHeader("ON CLIENT STOP", {});
}

void DebugCommandClientBase::onWaitingReply()
{
    // Log.
    std::cout << this->generateStringHeader("ON WAITING REPLY", {});
}

void DebugCommandClientBase::onDeadServer(const CommandServerInfo& server)
{
    std::cout << this->generateStringHeader("ON DEAD SERVER", {server.toString()});
}

void DebugCommandClientBase::onConnected(const CommandServerInfo& server)
{
    // Log.
    std::cout << this->generateStringHeader("ON CONNECTED", {server.toString()});
}

void DebugCommandClientBase::onDisconnected(const CommandServerInfo& server)
{
    // Log.
    std::cout << this->generateStringHeader("ON DISCONNECTED", {server.toJsonString()});
}

void DebugCommandClientBase::onBadOperation(const CommandReply &rep)
{
    // Log.
    std::stringstream data;
    data << "Server Command: " << std::to_string(static_cast<CommandType>(rep.command))
         << " (" << this->serverCommandToString(rep.command) << ")" << std::endl;
    data << "Result: " << static_cast<ResultType>(rep.result)
         << " (" << this->operationResultToString(rep.result) << ")";
    std::cout << this->generateStringHeader("ON BAD OPERATION", {data.str()});
}

void DebugCommandClientBase::onReplyReceived(const CommandReply &rep)
{
    // Log.
    BinarySerializer serializer(rep.data.bytes.get(), rep.data.size);
    std::stringstream data;
    data << "Server Command: " << std::to_string(static_cast<CommandType>(rep.command))
         << " (" << this->serverCommandToString(rep.command) << ")" << std::endl;
    data << "Result: " << static_cast<ResultType>(rep.result)
         << " (" << operationResultToString(rep.result) << ")" << std::endl;
    data << "Params Size: " << rep.data.size << std::endl;
    data << "Params Hex:  " << serializer.getDataHexString();
    std::cout << this->generateStringHeader("ON REPLY RECEIVED", {data.str()});
}

void DebugCommandClientBase::onSendingCommand(const CommandRequest &req)
{
    // Log.
    BinarySerializer serializer(req.data.bytes.get(), req.data.size);
    std::stringstream data;
    data << "Command:     " << static_cast<CommandType>(req.command)
         << " (" <<  this->serverCommandToString(req.command) << ")"     << std::endl;
    data << "Params size: " << req.data.size                             << std::endl;
    data << "Params Hex:  " << serializer.getDataHexString();
    std::cout << this->generateStringHeader("ON SENDING COMMAND", {data.str()});
}

void DebugCommandClientBase::onClientError(const zmq::error_t &error, const std::string &ext_info)
{
    // Log.
    std::stringstream data;
    data << "Code:  " << std::to_string(error.num()) << std::endl;
    data << "Error: " << error.what() << std::endl;
    data << "Info:  " << ext_info;
    std::cout << this->generateStringHeader("ON CLIENT ERROR", {data.str()});
}

}} // END NAMESPACES.
// =====================================================================================================================


