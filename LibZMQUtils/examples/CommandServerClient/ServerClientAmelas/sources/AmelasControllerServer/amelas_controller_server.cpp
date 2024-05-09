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
 * @file amelas_server.cpp
 * @brief EXAMPLE FILE - This file contains the implementation of the AmelasServer example class.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// PROJECT INCLUDES
// =====================================================================================================================
#include "includes/AmelasControllerServer/amelas_controller_server.h"
#include "includes/AmelasControllerServer/amelas_controller_server_data.h"
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace communication{
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
using zmqutils::serverclient::ServerCommand;
using zmqutils::serverclient::OperationResult;
using zmqutils::serverclient::ResultType;
using zmqutils::serverclient::CommandType;
using zmqutils::serverclient::CommandRequest;
using zmqutils::serverclient::CommandReply;
using zmqutils::serverclient::ClientInfo;
using zmqutils::serializer::BinarySerializer;
// ---------------------------------------------------------------------------------------------------------------------

bool AmelasControllerServer::validateCustomCommand(ServerCommand command)
{
    zmqutils::serverclient::CommandType cmd = static_cast<zmqutils::serverclient::CommandType>(command);
    return (cmd >= kMinCmdId && cmd <= kMaxCmdId) ? true : false;
}

void AmelasControllerServer::onCustomCommandReceived(CommandRequest& request, CommandReply& reply)
{
    // Log the command.
    std::cout<< std::string(100, '-')                                    << std::endl;
    std::cout<< "<" << this->getServerInfo().name << ">"                 << std::endl;
    std::cout<< "-> ON CUSTOM COMMAND RECEIVED: "                        << std::endl;
    std::cout<< "Time:        " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< "Client UUID: " << request.client_uuid.toRFC4122String() << std::endl;
    std::cout<< "Command:     " << static_cast<CommandType>(request.command) << " (" <<
        serverCommandStr(AmelasServerCommandStr, request.command) << ")" << std::endl;
    std::cout<< std::string(100, '-')                                    << std::endl;

    // Call to the base function for process the custom command with the registered process functions.
    CommandServerBase::onCustomCommandReceived(request, reply);
}

void AmelasControllerServer::onServerStart()
{
    // Log.
    std::cout<< std::string(100, '-')                                  << std::endl;
    std::cout<< "<" << this->getServerInfo().name << ">"               << std::endl;
    std::cout<< "-> ON SERVER START: "                                 << std::endl;
    std::cout<< "Time:      " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< "Addresses: " << this->getServerIpsStr(" - ")          << std::endl;
    std::cout<< "Port:      " << this->getServerInfo().port            << std::endl;
    std::cout<< "Name:      " << this->getServerInfo().name            << std::endl;
    std::cout<< "Hostname:  " << this->getServerInfo().hostname        << std::endl;
    std::cout<< "Endpoint:  " << this->getServerInfo().endpoint        << std::endl;
    std::cout<< "Info:      " << this->getServerInfo().info            << std::endl;
    std::cout<< "Version:   " << this->getServerInfo().version         << std::endl;
    std::cout<< std::string(100, '-')                                  << std::endl;
}

void AmelasControllerServer::onServerStop()
{
    // Log.
    std::cout<< std::string(100, '-')                             << std::endl;
    std::cout<< "<" << this->getServerInfo().name << ">"          << std::endl;
    std::cout<< "-> ON SERVER CLOSE: "                            << std::endl;
    std::cout<< "Time: " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< std::string(100, '-')                             << std::endl;
}

void AmelasControllerServer::onWaitingCommand()
{
    // Log.
    std::cout << std::string(100, '-')                           << std::endl;
    std::cout << "<" << this->getServerInfo().name << ">"        << std::endl;
    std::cout << "-> ON WAITING COMMAND: "                       << std::endl;
    std::cout << "Time: "<<zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout << std::string(100, '-')                           << std::endl;
}

void AmelasControllerServer::onDeadClient(const ClientInfo& client)
{
    // Log.
    std::cout << std::string(100, '-')                                       << std::endl;
    std::cout<< "<" << this->getServerInfo().name << ">"                     << std::endl;
    std::cout<<"-> ON DEAD CLIENT: "                                         << std::endl;
    std::cout<< "Time:            " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< "Current Clients: " << this->getConnectedClients().size()    << std::endl;
    std::cout<< "Client UUID:     " << client.uuid.toRFC4122String()         << std::endl;
    std::cout<< "Client Ip:       " << client.ip                             << std::endl;
    std::cout<< "Client PID:      " << client.pid                            << std::endl;
    std::cout<< "Client Hostname: " << client.hostname                       << std::endl;
    std::cout<< "Client Name:     " << client.name                           << std::endl;
    std::cout<< "Client Info:     " << client.info                           << std::endl;
    std::cout<< "Client Version:  " << client.version                        << std::endl;
    std::cout<< std::string(100, '-')                                        << std::endl;
}

void AmelasControllerServer::onConnected(const ClientInfo& client)
{
    // Log.
    std::cout << std::string(100, '-')                                       << std::endl;
    std::cout<< "<" << this->getServerInfo().name << ">"                     << std::endl;
    std::cout<< "-> ON CONNECTED: "                                          << std::endl;
    std::cout<< "Time:            " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< "Current Clients: " << this->getConnectedClients().size()    << std::endl;
    std::cout<< "Client UUID:     " << client.uuid.toRFC4122String()         << std::endl;
    std::cout<< "Client Ip:       " << client.ip                             << std::endl;
    std::cout<< "Client PID:      " << client.pid                            << std::endl;
    std::cout<< "Client Hostname: " << client.hostname                       << std::endl;
    std::cout<< "Client Name:     " << client.name                           << std::endl;
    std::cout<< "Client Info:     " << client.info                           << std::endl;
    std::cout<< "Client Version:  " << client.version                        << std::endl;
    std::cout<< std::string(100, '-')                                        << std::endl;
}

void AmelasControllerServer::onDisconnected(const ClientInfo& client)
{
    // Log.
    std::cout << std::string(100, '-')                                       << std::endl;
    std::cout<< "<" << this->getServerInfo().name << ">"                     << std::endl;
    std::cout<< "-> ON DISCONNECTED: "                                       << std::endl;
    std::cout<< "Time:            " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< "Current Clients: " << this->getConnectedClients().size()    << std::endl;
    std::cout<< "Client UUID:     " << client.uuid.toRFC4122String()         << std::endl;
    std::cout<< "Client Ip:       " << client.ip                             << std::endl;
    std::cout<< "Client PID:      " << client.pid                            << std::endl;
    std::cout<< "Client Hostname: " << client.hostname                       << std::endl;
    std::cout<< "Client Name:     " << client.name                           << std::endl;
    std::cout<< "Client Info:     " << client.info                           << std::endl;
    std::cout<< "Client Version:  " << client.version                        << std::endl;
    std::cout << std::string(100, '-')                                       << std::endl;
}

void AmelasControllerServer::onServerError(const zmq::error_t &error, const std::string &ext_info)
{
    // Log.
    std::cout<< std::string(100, '-')                              << std::endl;
    std::cout<< "<" << this->getServerInfo().name << ">"           << std::endl;
    std::cout<< "-> ON SERVER ERROR: "                             << std::endl;
    std::cout<< "Time:  " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< "Code:  "<<error.num()                             << std::endl;
    std::cout<< "Error: "<<error.what()                            << std::endl;
    std::cout<< "Info:  "<<ext_info                                << std::endl;
    std::cout<< std::string(100, '-')                              << std::endl;
}

void AmelasControllerServer::onCommandReceived(const CommandRequest &request)
{
    // Log.
    BinarySerializer serializer(request.params.get(), request.params_size);
    std::cout<< std::string(100, '-')                                    << std::endl;
    std::cout<< "<" << this->getServerInfo().name << ">"                 << std::endl;
    std::cout<< "-> ON COMMAND RECEIVED: "                               << std::endl;
    std::cout<< "Time:        " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< "Client UUID: "<<request.client_uuid.toRFC4122String()   << std::endl;
    std::cout<< "Command:     " << static_cast<CommandType>(request.command) << " (" <<
        serverCommandStr(AmelasServerCommandStr, request.command) << ")" << std::endl;
    std::cout<< "Params Size: " << request.params_size                   << std::endl;
    std::cout<< "Params Hex:  " << serializer.getDataHexString()         << std::endl;
    std::cout<< std::string(100, '-')                                    << std::endl;
}

void AmelasControllerServer::onInvalidMsgReceived(const CommandRequest &request)
{
    // Log.
    BinarySerializer serializer(request.params.get(), request.params_size);
    std::cout<< std::string(100, '-')                                    << std::endl;
    std::cout<< "<" << this->getServerInfo().name << ">"                 << std::endl;
    std::cout<< "-> ON BAD COMMAND RECEIVED: "                           << std::endl;
    std::cout<< "Time:        " << zmqutils::utils::currentISO8601Date() << std::endl;
    std::cout<< "Client UUID: "<<request.client_uuid.toRFC4122String()   << std::endl;
    std::cout<< "Command:     " << static_cast<CommandType>(request.command) << " (" <<
        serverCommandStr(AmelasServerCommandStr, request.command) << ")" << std::endl;
    std::cout<< "Params Size: " << request.params_size                   << std::endl;
    std::cout<< "Params Hex:  " << serializer.getDataHexString()         << std::endl;
    std::cout<< std::string(100, '-')                                    << std::endl;
}

void AmelasControllerServer::onSendingResponse(const CommandReply &reply)
{
    // Log.
    BinarySerializer serializer(reply.params.get(), reply.params_size);
    std::cout<< std::string(100, '-')                                            << std::endl;
    std::cout<< "<" << this->getServerInfo().name << ">"                         << std::endl;
    std::cout<< "-> ON SENDING RESPONSE: "                                       << std::endl;
    std::cout<< "Time:        " << zmqutils::utils::currentISO8601Date()         << std::endl;
    std::cout<< "Result:      " << static_cast<ResultType>(reply.server_result) << " (" <<
        operationResultStr(AmelasOperationResultStr, reply.server_result) << ")" << std::endl;
    std::cout<< "Params Size: " << reply.params_size                             << std::endl;
    std::cout<< "Params Hex:  " << serializer.getDataHexString()                 << std::endl;
    std::cout<< std::string(100, '-')                                            << std::endl;
}

}} // END NAMESPACES.
// =====================================================================================================================

