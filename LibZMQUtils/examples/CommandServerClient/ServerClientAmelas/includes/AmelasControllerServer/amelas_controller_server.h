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
 * @file amelas_controller_server.h
 * @brief EXAMPLE FILE - This file contains the declaration of the AmelasServer example class.
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

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/CallbackCommandServer>
#include <LibZMQUtils/Modules/Utilities>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "includes/AmelasController/amelas_controller.h"
#include "includes/AmelasController/common.h"
#include "includes/AmelasControllerServer/common.h"
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace communication{
// =====================================================================================================================

// Example of creating a command server from the base.
class AmelasControllerServer : public zmqutils::serverclient::ClbkCommandServerBase
{
public:

    AmelasControllerServer(unsigned port, const std::string& local_addr = "*");

    // Register callback function helper.
    template<typename... Args>
    void registerControllerCallback(AmelasServerCommand command,
                                    controller::AmelasController* object,
                                    controller::AmelasControllerCallback<Args...> callback)
    {
        ClbkCommandServerBase::registerCallback(command, object, callback);
    }

private:

    // -----------------------------------------------------------------------------------------------------------------
    using CommandServerBase::registerRequestProcFunc;
    using ClbkCommandServerBase::registerCallback;
    // -----------------------------------------------------------------------------------------------------------------

    // -----------------------------------------------------------------------------------------------------------------

    // Internal overrided command validation function.
    virtual bool validateCustomCommand(zmqutils::serverclient::ServerCommand command) final;

    // Internal overrided custom command received callback.
    virtual void onCustomCommandReceived(zmqutils::serverclient::CommandRequest&,
                                         zmqutils::serverclient::CommandReply&) final;

    // Internal overrided start callback.
    virtual void onServerStart() final;

    // Internal overrided close callback.
    virtual void onServerStop() final;

    // Internal waiting command callback.
    virtual void onWaitingCommand() final;

    // Internal dead client callback.
    virtual void onDeadClient(const zmqutils::serverclient::HostInfo&) final;

    // Internal overrided connect callback.
    virtual void onConnected(const zmqutils::serverclient::HostInfo&) final;

    // Internal overrided disconnect callback.
    virtual void onDisconnected(const zmqutils::serverclient::HostInfo&) final;

    // Internal overrided command received callback.
    virtual void onCommandReceived(const zmqutils::serverclient::CommandRequest&) final;

    // Internal overrided bad command received callback.
    virtual void onInvalidMsgReceived(const zmqutils::serverclient::CommandRequest&) final;

    // Internal overrided sending response callback.
    virtual void onSendingResponse(const zmqutils::serverclient::CommandReply&) final;

    // Internal overrided server error callback.
    virtual void onServerError(const zmq::error_t&, const std::string& ext_info) final;
};

}} // END NAMESPACES.
// =====================================================================================================================
