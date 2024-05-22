/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
 *   ExamplesLibZMQUtils related project.                                                                              *
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

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasControllerClient/amelas_controller_client.h"
#include "AmelasControllerServer/amelas_controller_server_data.h"
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace communication{
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
using zmqutils::reqrep::ServerCommand;
using zmqutils::reqrep::OperationResult;
using zmqutils::reqrep::ResultType;
using zmqutils::reqrep::CommandType;
using zmqutils::reqrep::CommandReply;
using zmqutils::reqrep::RequestData;
using zmqutils::serializer::BinarySerializer;
// ---------------------------------------------------------------------------------------------------------------------

AmelasControllerClient::AmelasControllerClient(const std::string& server_endpoint, const std::string& client_name,
                                               const std::string& client_version, const std::string& client_info,
                                               const std::string& net_interface) :
    zmqutils::reqrep::DebugCommandClientBase(server_endpoint, client_name, client_version, client_info, net_interface)
{
    // Register the enum to string lookup arrays.
    this->registerCommandToStrLookup(AmelasServerCommandStr);
}

bool AmelasControllerClient::validateCommand(zmqutils::reqrep::CommandType command) const
{
    return (CommandClientBase::isBaseCommand(command) || (command >= kMinCmdId && command <= kMaxCmdId));
}

OperationResult AmelasControllerClient::getHomePosition(controller::AltAzPos &pos, controller::AmelasError &res)
{    
    return this->executeCommand(AmelasServerCommand::REQ_GET_HOME_POSITION, res, pos);
}

OperationResult AmelasControllerClient::setHomePosition(const controller::AltAzPos &pos,
                                                          controller::AmelasError &res)
{
    RequestData request = this->prepareRequestData(pos);
    return this->executeCommand(AmelasServerCommand::REQ_SET_HOME_POSITION, request, res);
}

OperationResult AmelasControllerClient::doOpenSearchTelescope(controller::AmelasError &res)
{
    return this->executeCommand(AmelasServerCommand::REQ_DO_OPEN_SEARCH_TELESCOPE, res);
}

OperationResult AmelasControllerClient::doExampleNotImp(controller::AmelasError &res)
{
    return this->executeCommand(AmelasServerCommand::REQ_DO_EXAMPLE_NOT_IMP, res);
}

}} // END NAMESPACES.
// =====================================================================================================================

