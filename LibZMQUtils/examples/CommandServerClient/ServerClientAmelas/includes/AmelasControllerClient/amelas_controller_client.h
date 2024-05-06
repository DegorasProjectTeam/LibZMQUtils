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
 * @file amelas_controller_client.h
 * @brief EXAMPLE FILE - This file contains the declaration of the AmelasControllerClient example class.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/CommandClient>
#include <LibZMQUtils/Modules/Utilities>
// =====================================================================================================================

// INTERFACE INCLUDES
// =====================================================================================================================
#include <AmelasController/amelas_controller_data.h>
#include <AmelasControllerServer/amelas_controller_server_data.h>
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace communication{
// =====================================================================================================================

class AmelasControllerClient : public zmqutils::serverclient::CommandClientBase
{
public:

    AmelasControllerClient(const std::string& server_endpoint,
                           const std::string& client_name = "",
                           const std::string interf_name = "");

    template<typename CmdId>
    bool validateCommand(CmdId command) const
    {
        return (CommandClientBase::validateCommand(command) ||
                (static_cast<std::int32_t>(command) >= kMinCmdId && static_cast<std::int32_t>(command) <= kMaxCmdId));
    }

    zmqutils::serverclient::OperationResult getHomePosition(controller::AltAzPos &pos,
                                                              controller::AmelasError &res);

    zmqutils::serverclient::OperationResult setHomePosition(const controller::AltAzPos &pos,
                                                              controller::AmelasError &res);

    zmqutils::serverclient::OperationResult doOpenSearchTelescope(controller::AmelasError &res);

private:

    virtual void onClientStart() override final;

    virtual void onClientStop() override final;

    virtual void onWaitingReply() override final;

    virtual void onDeadServer() override final;

    virtual void onConnected() override final;

    virtual void onDisconnected() override final;

    virtual void onInvalidMsgReceived(const zmqutils::serverclient::CommandReply&) override final;

    virtual void onReplyReceived(const zmqutils::serverclient::CommandReply& reply) override final;

    virtual void onSendingCommand(const zmqutils::serverclient::RequestData&) override final;

    virtual void onClientError(const zmq::error_t&, const std::string& ext_info) override final;
};

}} // END NAMESPACES.
// =====================================================================================================================
