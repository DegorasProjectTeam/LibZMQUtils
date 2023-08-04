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
 * @file amelas_server.cpp
 * @brief EXAMPLE FILE - This file contains the implementation of the AmelasServer example class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2307.1
***********************************************************************************************************************/

// PROJECT INCLUDES
// =====================================================================================================================
#include "amelas_server.h"
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace communication{
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
using zmqutils::common::ServerCommand;
using zmqutils::common::ServerResult;
using zmqutils::common::ResultType;
using zmqutils::utils::BinarySerializer;
// ---------------------------------------------------------------------------------------------------------------------

AmelasServer::AmelasServer(unsigned int port, const std::string &local_addr) :
    ClbkCommandServerBase(port, local_addr)
{
    // Register each internal specific process function in the base server.

    // REQ_SET_HOME_POSITION
    this->registerRequestProcFunc(AmelasServerCommand::REQ_SET_HOME_POSITION,
                                  &AmelasServer::processSetHomePosition);

    // REQ_GET_HOME_POSITION.
    this->registerRequestProcFunc(AmelasServerCommand::REQ_GET_HOME_POSITION,
                                  &AmelasServer::processGetHomePosition);
}

void AmelasServer::processSetHomePosition(const CommandRequest& request, CommandReply& reply)
{
    // Auxiliar variables and containers.
    controller::ControllerError ctrl_err;
    double az, el;

    // Check the request parameters size.
    if (request.params_size == 0 || !request.params)
    {
        reply.result = ServerResult::EMPTY_PARAMS;
        return;
    }

    // Try to read the parameters data.
    try
    {
        BinarySerializer::fastDeserialization(request.params.get(), request.params_size, az, el);
    }
    catch(...)
    {
        reply.result = ServerResult::BAD_PARAMETERS;
        return;
    }

    // Position struct.
    controller::AltAzPos pos = {az, el};

    // Now we will process the command in the controller.
    ctrl_err = this->invokeCallback<controller::SetHomePositionCallback>(request, reply, pos);

    // Serialize parameters if all ok.
    if(reply.result == ServerResult::COMMAND_OK)
        reply.params_size = BinarySerializer::fastSerialization(reply.params, ctrl_err);
}

void AmelasServer::processGetHomePosition(const CommandRequest& request, CommandReply &reply)
{
    // Auxiliar variables and containers.
    controller::ControllerError ctrl_err;
    controller::AltAzPos pos;

    // Now we will process the command in the controller.
    ctrl_err = this->invokeCallback<controller::GetHomePositionCallback>(request, reply, pos);

    // Serialize parameters if all ok.
    if(reply.result == ServerResult::COMMAND_OK)
        reply.params_size = BinarySerializer::fastSerialization(reply.params, ctrl_err, pos.az, pos.el);
}

void AmelasServer::registerRequestProcFunc(AmelasServerCommand command, AmelasRequestProcFunc func)
{
    CommandServerBase::registerRequestProcFunc(static_cast<ServerCommand>(command), this, func);
}

bool AmelasServer::validateCustomCommand(ServerCommand command)
{
    // Auxiliar variables.
    bool result = false;
    zmqutils::common::CommandType cmd = static_cast<zmqutils::common::CommandType>(command);
    // Check if the command is within the range of implemented custom commands.
    if (cmd >= common::kMinCmdId && cmd <= common::kMaxCmdId)
        result = true;
    return result;
}

void AmelasServer::onCustomCommandReceived(const CommandRequest& request, CommandReply& reply)
{
    // Get the command string.
    std::string cmd_str;
    std::uint32_t cmd_uint = static_cast<std::uint32_t>(request.command);
    cmd_str = (cmd_uint < AmelasServerCommandStr.size()) ? AmelasServerCommandStr[cmd_uint] : "Unknown command";

    // Log the command.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"ON CUSTOM COMMAND RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client UUID: "<<request.client_uuid.toRFC4122String()<<std::endl;
    std::cout<<"Command: "<<cmd_uint<<" ("<<cmd_str<<")"<<std::endl;
    std::cout << std::string(100, '-') << std::endl;

    // Call to the base function for process the custom command with the registered process functions.
    CommandServerBase::onCustomCommandReceived(request, reply);
}

void AmelasServer::onServerStart()
{
    // Ips.
    std::string ips;

    // Get listen interfaces ips.
    for(const auto& intrfc : this->getServerAddresses())
    {
        ips.append(intrfc.ip);
        ips.append(" - ");
    }
    ips.pop_back();
    ips.pop_back();

    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SERVER START: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Addresses: "<<ips<<std::endl;
    std::cout<<"Port: "<<this->getServerPort()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onServerStop()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SERVER CLOSE: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onWaitingCommand()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON WAITING COMMAND: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onDeadClient(const HostClientInfo& client)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON DEAD CLIENT: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Current Clients: "<<this->getConnectedClients().size()<<std::endl;
    std::cout<<"Client UUID: "<<client.uuid.toRFC4122String()<<std::endl;
    std::cout<<"Client Ip: "<<client.ip<<std::endl;
    std::cout<<"Client Host: "<<client.hostname<<std::endl;
    std::cout<<"Client Process: "<<client.pid<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onConnected(const HostClientInfo& client)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON CONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Current Clients: "<<this->getConnectedClients().size()<<std::endl;
    std::cout<<"Client UUID: "<<client.uuid.toRFC4122String()<<std::endl;
    std::cout<<"Client Name: "<<client.name<<std::endl;
    std::cout<<"Client Ip: "<<client.ip<<std::endl;
    std::cout<<"Client Host: "<<client.hostname<<std::endl;
    std::cout<<"Client Process: "<<client.pid<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onDisconnected(const HostClientInfo& client)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON DISCONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Current Clients: "<<this->getConnectedClients().size()<<std::endl;
    std::cout<<"Client UUID: "<<client.uuid.toRFC4122String()<<std::endl;
    std::cout<<"Client Name: "<<client.name<<std::endl;
    std::cout<<"Client Ip: "<<client.ip<<std::endl;
    std::cout<<"Client Host: "<<client.hostname<<std::endl;
    std::cout<<"Client Process: "<<client.pid<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onServerError(const zmq::error_t &error, const std::string &ext_info)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SERVER ERROR: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Code: "<<error.num()<<std::endl;
    std::cout<<"Error: "<<error.what()<<std::endl;
    std::cout<<"Info: "<<ext_info<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onCommandReceived(const CommandRequest &request)
{
    // Get the command string.
    std::string cmd_str;
    std::uint32_t command = static_cast<std::uint32_t>(request.command);
    cmd_str = (command < AmelasServerCommandStr.size()) ? AmelasServerCommandStr[command] : "Unknown command";
    // Log.
    BinarySerializer serializer(request.params.get(), request.params_size);
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON COMMAND RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client UUID: "<<request.client_uuid.toRFC4122String()<<std::endl;
    std::cout<<"Command: "<<command<<" ("<<cmd_str<<")"<<std::endl;
    std::cout<<"Params Size: "<<request.params_size<<std::endl;
    std::cout<<"Params Hex: "<<serializer.getDataHexString()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onInvalidMsgReceived(const CommandRequest &request)
{
    // Log.
    BinarySerializer serializer(request.params.get(), request.params_size);
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON BAD COMMAND RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client UUID: "<<request.client_uuid.toRFC4122String()<<std::endl;
    std::cout<<"Command: "<<static_cast<int>(request.command)<<std::endl;
    std::cout<<"Params Size: "<<request.params_size<<std::endl;
    std::cout<<"Params Hex: "<<serializer.getDataHexString()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onSendingResponse(const CommandReply &reply)
{
    // Log.
    BinarySerializer serializer(reply.params.get(), reply.params_size);
    int result = static_cast<int>(reply.result);
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SENDING RESPONSE: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Result: "<<result<<" ("<<AmelasServerResultStr[result]<<")"<<std::endl;
    std::cout<<"Params Size: "<<reply.params_size<<std::endl;
    std::cout<<"Params Hex: "<<serializer.getDataHexString()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

}} // END NAMESPACES.
// =====================================================================================================================

