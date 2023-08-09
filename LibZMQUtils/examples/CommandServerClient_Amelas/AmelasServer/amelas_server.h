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
 * @file amelas_server.h
 * @brief EXAMPLE FILE - This file contains the declaration of the AmelasServer example class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2308.2
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <unordered_map>
#include <string>
#include <any>
#include <variant>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/CallbackCommandServer>
#include <LibZMQUtils/Utils>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasController/amelas_controller.h"
#include "AmelasController/common.h"
#include "AmelasServer/common.h"
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace communication{
// =====================================================================================================================

// ---------------------------------------------------------------------------------------------------------------------
using namespace amelas::communication::common;
using zmqutils::common::CommandReply;
using zmqutils::common::CommandRequest;
using zmqutils::common::ServerResult;
using zmqutils::common::ServerCommand;
using zmqutils::common::HostClientInfo;
using zmqutils::utils::CallbackHandler;
// ---------------------------------------------------------------------------------------------------------------------

// Example of creating a command server from the base.
class AmelasServer : public zmqutils::ClbkCommandServerBase
{
public:

    AmelasServer(unsigned port, const std::string& local_addr = "*");

    // Register callback function helper.
    template<typename... Args>
    void registerControllerCallback(AmelasServerCommand command,
                                    controller::AmelasController* object,
                                    controller::AmelasControllerCallback<Args...> callback)
    {
        CallbackHandler::registerCallback(static_cast<CallbackHandler::CallbackId>(command), object, callback);
    }

private:

    // -----------------------------------------------------------------------------------------------------------------
    using CommandServerBase::registerRequestProcFunc;
    using CallbackHandler::registerCallback;
    using AmelasRequestProcFunc = void(AmelasServer::*)(const CommandRequest&, CommandReply&);
    // -----------------------------------------------------------------------------------------------------------------

    // Process functions for all the specific commands.
    void processSetHomePosition(const CommandRequest&, CommandReply&);
    void processGetHomePosition(const CommandRequest&, CommandReply&);

    // Subclass register process function helper.
    void registerRequestProcFunc(AmelasServerCommand command, AmelasRequestProcFunc func);

    // Subclass invoke callback helper.
    template <typename ClbkT, typename... Args>
    controller::ControllerError invokeCallback(const CommandRequest& request, CommandReply& reply, Args&&... args)
    {
        return ClbkCommandServerBase::invokeCallback<ClbkT>(request, reply,
                                                            controller::ControllerError::INVALID_ERROR,
                                                            std::forward<Args>(args)...);
    }

    // Internal overrided command validation function.
    virtual bool validateCustomCommand(ServerCommand command) final;

    // Internal overrided custom command received callback.
    virtual void onCustomCommandReceived(const CommandRequest&, CommandReply&) final;

    // Internal overrided start callback.
    virtual void onServerStart() final;

    // Internal overrided close callback.
    virtual void onServerStop() final;

    // Internal waiting command callback.
    virtual void onWaitingCommand() final;

    // Internal dead client callback.
    virtual void onDeadClient(const HostClientInfo&) final;

    // Internal overrided connect callback.
    virtual void onConnected(const HostClientInfo&) final;

    // Internal overrided disconnect callback.
    virtual void onDisconnected(const HostClientInfo&) final;

    // Internal overrided command received callback.
    virtual void onCommandReceived(const CommandRequest&) final;

    // Internal overrided bad command received callback.
    virtual void onInvalidMsgReceived(const CommandRequest&) final;

    // Internal overrided sending response callback.
    virtual void onSendingResponse(const CommandReply&) final;

    // Internal overrided server error callback.
    virtual void onServerError(const zmq::error_t&, const std::string& ext_info) final;
};

}} // END NAMESPACES.
// =====================================================================================================================
