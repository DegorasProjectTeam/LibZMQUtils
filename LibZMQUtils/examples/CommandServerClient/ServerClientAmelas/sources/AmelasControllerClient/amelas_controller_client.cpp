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

#include "includes/AmelasControllerClient/amelas_controller_client.h"
#include "includes/AmelasControllerServer/common.h"

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace communication{
// =====================================================================================================================

using common::AmelasServerCommandStr;
using common::AmelasOperationResultStr;
using common::AmelasServerCommand;
using common::AmelasOperationResult;
using zmqutils::serverclient::ServerCommand;
using zmqutils::serverclient::OperationResult;
using zmqutils::serverclient::ResultType;
using zmqutils::serverclient::CommandType;
using zmqutils::serverclient::CommandReply;
using zmqutils::serverclient::RequestData;
using zmqutils::utils::BinarySerializer;

AmelasControllerClient::AmelasControllerClient(const std::string& server_endpoint,
                           const std::string& client_name,
                           const std::string interf_name) :
    zmqutils::serverclient::CommandClientBase(server_endpoint, client_name, interf_name)
{}

OperationResult AmelasControllerClient::doGetHomePosition(controller::AltAzPos &pos, controller::AmelasError &res)
{
    RequestData command_msg(static_cast<ServerCommand>(AmelasServerCommand::REQ_GET_HOME_POSITION));
    CommandReply reply;
    OperationResult op_res = this->sendCommand(command_msg, reply);

    if (OperationResult::COMMAND_OK == op_res)
    {
        try
        {
            BinarySerializer::fastDeserialization(reply.params.get(), reply.params_size, res, pos.az, pos.el);
        }
        catch(...)
        {
            op_res = OperationResult::BAD_PARAMETERS;
        }
    }

    return op_res;
}

OperationResult AmelasControllerClient::doSetHomePosition(const controller::AltAzPos &pos,
                                                          controller::AmelasError &res)
{
    RequestData command_msg(static_cast<ServerCommand>(AmelasServerCommand::REQ_SET_HOME_POSITION));
    CommandReply reply;
    BinarySerializer serializer;
    pos.serialize(serializer);
    command_msg.params_size = serializer.moveUnique(command_msg.params);
    OperationResult op_res = this->sendCommand(command_msg, reply);

    if (OperationResult::COMMAND_OK == op_res)
    {
        try
        {
            BinarySerializer::fastDeserialization(reply.params.get(), reply.params_size, res);
        }
        catch(...)
        {
            op_res = OperationResult::BAD_PARAMETERS;
        }
    }

    return op_res;
}

void AmelasControllerClient::onClientStart()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON CLIENT START: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Endpoint: "<<this->getServerEndpoint()<<std::endl;
    std::cout<<"Name: "<<this->getClientInfo().name<<std::endl;
    std::cout<<"UUID: "<<this->getClientInfo().uuid.toRFC4122String()<<std::endl;
    std::cout<<"Ip: "<<this->getClientInfo().ip<<std::endl;
    std::cout<<"Pid: "<<this->getClientInfo().pid<<std::endl;
    std::cout<<"Hostname: "<<this->getClientInfo().hostname<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasControllerClient::onClientStop()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON CLIENT STOP: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasControllerClient::onWaitingReply()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON WAITING REPLY: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasControllerClient::onDeadServer()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON DEAD SERVER: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasControllerClient::onConnected()
{
    // TODO In base get server info when connected.
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON CONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Endpoint: "<<this->getServerEndpoint()<<std::endl;
    std::cout<<"Server Name: "<<std::endl;
    std::cout<<"Server Version: "<<std::endl;
    std::cout<<"Server Info: "<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasControllerClient::onDisconnected()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON DISCONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasControllerClient::onReplyReceived(const CommandReply &reply)
{
    // Auxiliar.
    BinarySerializer serializer(reply.params.get(), reply.params_size);
    ResultType result = static_cast<ResultType>(reply.server_result);
    std::string res_str = zmqutils::utils::getEnumString(reply.server_result, AmelasOperationResultStr);
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON REPLY RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Result: "<<result<<" ("<<res_str<<")"<<std::endl;
    std::cout<<"Params Size: "<<reply.params_size<<std::endl;
    std::cout<<"Params Hex: "<<serializer.getDataHexString()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasControllerClient::onSendingCommand(const RequestData &req)
{
    BinarySerializer serializer(req.params.get(), req.params_size);
    CommandType command = static_cast<CommandType>(req.command);
    std::string cmd_str = zmqutils::utils::getEnumString(req.command, AmelasServerCommandStr);
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON SEND COMMAND: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Command: "<<command<<" ("<<cmd_str<<")"<<std::endl;
    std::cout<<"Params size: "<<req.params_size<<std::endl;
    std::cout<<"Params Hex: "<<serializer.getDataHexString()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasControllerClient::onClientError(const zmq::error_t& error, const std::string& ext_info)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON CLIENT ERROR: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Code: "<<error.num()<<std::endl;
    std::cout<<"Error: "<<error.what()<<std::endl;
    std::cout<<"Info: "<<ext_info<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void amelas::communication::AmelasControllerClient::onInvalidMsgReceived(const CommandReply &) {}



}} // END NAMESPACES.
// =====================================================================================================================

