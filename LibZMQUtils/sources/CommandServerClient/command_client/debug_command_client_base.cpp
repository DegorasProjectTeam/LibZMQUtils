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

DebugCommandClientBase::DebugCommandClientBase(const std::string &server_endpoint,
                                               const std::string &client_iface,
                                               const std::string &client_name,
                                               const std::string &client_version,
                                               const std::string &client_info,
                                               bool log_internal_callbacks) :
    CommandClientBase(server_endpoint, client_iface, client_name, client_version, client_info),
    log_internal_callbacks_(log_internal_callbacks)
{}

DebugCommandClientBase::DebugCommandClientBase(const std::string &server_endpoint,
                                               const std::string &client_iface,
                                               bool log_internal_callbacks) :
    CommandClientBase(server_endpoint, client_iface),
    log_internal_callbacks_(log_internal_callbacks)
{}

std::string DebugCommandClientBase::generateStringHeader(const std::string &clbk_name,
                                                         const std::vector<std::string>& data)
{
    if(!this->log_internal_callbacks_)
        return std::string();

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
    // Log.
    std::string data = this->serverWasSeen() ? server.toString() : server.toString(true);
    std::cout << this->generateStringHeader("ON DEAD SERVER", {data});
}

void DebugCommandClientBase::onConnected(const CommandServerInfo& server)
{
    // Log.
    std::cout << this->generateStringHeader("ON CONNECTED", {server.toString()});
}

void DebugCommandClientBase::onDisconnected(const CommandServerInfo& server)
{
    // Log.
    std::cout << this->generateStringHeader("ON DISCONNECTED", {server.toString()});
}

void DebugCommandClientBase::onBadOperation(const CommandReply &rep)
{
    // Log.
    std::stringstream data;
    data << "Reply Timestamp:  " << rep.timestamp                          << std::endl;
    data << "Elapsed ms:       " << rep.elapsed.count()                   << std::endl;
    data << "Server UUID:      " << rep.server_uuid.toRFC4122String()      << std::endl;
    data << "Server Command:   " << std::to_string(static_cast<CommandType>(rep.command))
         << " (" << this->serverCommandToString(rep.command) << ")"        << std::endl;
    data << "Result:           " << static_cast<ResultType>(rep.result)
         << " (" << this->operationResultToString(rep.result) << ")";
    std::cout << this->generateStringHeader("ON BAD OPERATION", {data.str()});
}

void DebugCommandClientBase::onReplyReceived(const CommandReply &rep)
{
    // Log.
    BinarySerializer serializer(rep.data.bytes.get(), rep.data.size);
    std::stringstream data;
    data << "Reply Timestamp:   " << rep.timestamp                         << std::endl;
    data << "Elapsed ms:        " << rep.elapsed.count()                   << std::endl;
    data << "Server UUID:       " << rep.server_uuid.toRFC4122String()     << std::endl;
    data << "Server Command:    " << std::to_string(static_cast<CommandType>(rep.command))
         << " (" << this->serverCommandToString(rep.command) << ")"        << std::endl;
    data << "Result:            " << static_cast<ResultType>(rep.result)
         << " (" << operationResultToString(rep.result) << ")"             << std::endl;
    data << "Params Size:       " << rep.data.size << std::endl;
    data << "Params Hex:        " << serializer.getDataHexString();
    std::cout << this->generateStringHeader("ON REPLY RECEIVED", {data.str()});
}

void DebugCommandClientBase::onSendingCommand(const CommandRequest &req)
{
    // Log.
    BinarySerializer serializer(req.data.bytes.get(), req.data.size);
    std::stringstream data;
    data << "Req. Timestamp:  " << req.timestamp                           << std::endl;
    data << "Server Command:  " << static_cast<CommandType>(req.command)
         << " (" <<  this->serverCommandToString(req.command) << ")"       << std::endl;
    data << "Params size:     " << req.data.size                           << std::endl;
    data << "Params Hex:      " << serializer.getDataHexString();
    std::cout << this->generateStringHeader("ON SENDING COMMAND", {data.str()});
}

void DebugCommandClientBase::onClientError(const zmq::error_t &error, const std::string &ext_info)
{
    // Log.
    std::stringstream data;
    data << "Code:  " << std::to_string(error.num())                     << std::endl;
    data << "Error: " << error.what()                                    << std::endl;
    data << "Info:  " << ext_info;
    std::cout << this->generateStringHeader("ON CLIENT ERROR", {data.str()});
}

}} // END NAMESPACES.
// =====================================================================================================================
