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
 * @file clbk_command_server_base.cpp
 * @brief This file contains the implementation of the DebugClbkCommandServerBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <string>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/CommandServerClient/command_server/debug_clbk_command_server_base.h"
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

DebugClbkCommandServerBase::DebugClbkCommandServerBase(unsigned int port,
                                                       const std::string &server_iface,
                                                       bool log_internal_callbacks) :
    ClbkCommandServerBase(port, server_iface),
    log_internal_callbacks_(log_internal_callbacks)

{}

DebugClbkCommandServerBase::DebugClbkCommandServerBase(unsigned int server_port,
                                                       const std::string &server_iface,
                                                       const std::string &server_name,
                                                       const std::string &server_version,
                                                       const std::string &server_info,
                                                       bool log_internal_callbacks) :
    ClbkCommandServerBase(server_port, server_iface, server_name, server_version, server_info),
    log_internal_callbacks_(log_internal_callbacks)
{}

std::string DebugClbkCommandServerBase::generateStringHeader(const std::string &clbk_name,
                                                             const std::vector<std::string>& data)
{
    if(!this->log_internal_callbacks_)
        return std::string();

    std::stringstream ss;
    ss << std::string(100, '-') << std::endl;
    ss << "<" << this->getServerInfo().name << ">" << std::endl;
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

void DebugClbkCommandServerBase::onCustomCommandReceived(CommandRequest& request)
{
    // Log.
    std::string command = "Command: " + std::to_string(static_cast<CommandType>(request.command)) +
                          " (" + this->serverCommandToString(request.command) + ")";
    std::string ts_request = "Request timestamp: " + request.timestamp;
    std::cout << this->generateStringHeader("ON CUSTOM COMMAND RECEIVED", {command, ts_request});
}

void DebugClbkCommandServerBase::onServerStart()
{
    // Log.
    std::cout << this->generateStringHeader("ON SERVER START", {this->getServerInfo().toString()});
}

void DebugClbkCommandServerBase::onServerStop()
{
    // Log.
    std::cout << this->generateStringHeader("ON SERVER STOP", {});
}

void DebugClbkCommandServerBase::onWaitingCommand()
{
    // Log.
    std::cout << this->generateStringHeader("ON WAITING COMMAND", {});
}

void DebugClbkCommandServerBase::onDeadClient(const CommandClientInfo& client)
{
    // Log.
    std::string nclients = "Current Clients: " + std::to_string(this->getConnectedClients().size());
    std::cout << this->generateStringHeader("ON DEAD CLIENT", {client.toString(), nclients});
}

void DebugClbkCommandServerBase::onConnected(const CommandClientInfo& client)
{
    // Log.
    std::string nclients = "Current Clients: " + std::to_string(this->getConnectedClients().size());
    std::cout << this->generateStringHeader("ON CONNECTED", {client.toString(), nclients});
}

void DebugClbkCommandServerBase::onDisconnected(const CommandClientInfo& client)
{
    // Log.
    std::string nclients = "Current Clients: " + std::to_string(this->getConnectedClients().size());
    std::cout << this->generateStringHeader("ON DISCONNECTED", {client.toString(), nclients});
}

void DebugClbkCommandServerBase::onServerError(const zmq::error_t& error, const std::string &ext_info)
{
    // Log.
    std::stringstream data;
    data << "Code:  " << std::to_string(error.num())                                        << std::endl;
    data << "Error: " << error.what()                                                       << std::endl;
    data << "Info:  " << ext_info;
    std::cout << this->generateStringHeader("ON SERVER ERROR", {data.str()});
}


void DebugClbkCommandServerBase::onCommandReceived(const CommandRequest &request)
{
    // Log.
    BinarySerializer serializer(request.data.bytes.get(), request.data.size);
    std::stringstream data;
    data << "Client UUID:        " << request.client_uuid.toRFC4122String()                 << std::endl;
    data << "Request Timestamp:  " << request.timestamp                                     << std::endl;
    data << "Server Command:     "
         << std::to_string(static_cast<CommandType>(request.command))
         << " (" << this->serverCommandToString(request.command) << ")"                     << std::endl;
    data << "Params Size:        " << request.data.size                                     << std::endl;
    data << "Params Hex:         " << serializer.getDataHexString();
    std::cout << this->generateStringHeader("ON COMMAND RECEIVED", {data.str()});
}

void DebugClbkCommandServerBase::onInvalidMsgReceived(const CommandRequest &request)
{
    // Log.
    BinarySerializer serializer(request.data.bytes.get(), request.data.size);
    std::stringstream data;
    data << "Client UUID:        " << request.client_uuid.toRFC4122String()                 << std::endl;
    data << "Request Timestamp:  " << request.timestamp                                     << std::endl;
    data << "Server Command:     "
         << std::to_string(static_cast<CommandType>(request.command))
         << " (" << this->serverCommandToString(request.command) << ")"                     << std::endl;
    data << "Params Size:        " << request.data.size                                     << std::endl;
    data << "Params Hex:         " << serializer.getDataHexString();
    std::cout << this->generateStringHeader("ON BAD COMMAND RECEIVED", {data.str()});
}

void DebugClbkCommandServerBase::onSendingResponse(const CommandReply &reply)
{
    // Log.
    BinarySerializer serializer(reply.data.bytes.get(), reply.data.size);
    std::stringstream data;    
    data << "Server Command:     "
         << std::to_string(static_cast<CommandType>(reply.command))
         << " (" << this->serverCommandToString(reply.command) << ")"                       << std::endl;
    data << "Reply timestamp:    " << reply.timestamp                                       << std::endl;
    data << "Result:             "
         << static_cast<ResultType>(reply.result)
         << " (" << operationResultToString(reply.result) << ")"                            << std::endl;
    data << "Params Size:        " << reply.data.size                                       << std::endl;
    data << "Params Hex:         " << serializer.getDataHexString();
    std::cout << this->generateStringHeader("ON SENDING RESPONSE", {data.str()});
}

}} // END NAMESPACES.
// =====================================================================================================================
