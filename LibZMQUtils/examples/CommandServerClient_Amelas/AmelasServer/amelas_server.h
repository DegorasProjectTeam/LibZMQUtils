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
 * @version 2307.1
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
#include <unordered_map>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/CommandServer>
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
namespace cltsrv{
// =====================================================================================================================

// =====================================================================================================================
using namespace amelas::cltsrv::common;
using zmqutils::CommandServerBase;
using zmqutils::common::CommandReply;
using zmqutils::common::CommandRequest;
using zmqutils::common::ServerResult;
using zmqutils::common::HostClientInfo;
using zmqutils::utils::CallbackHandler;

// =====================================================================================================================


// Example of creating a command server from the base.
class AmelasServer : public CommandServerBase, public CallbackHandler
{
public:

    using ProcessFunctionsMap = std::unordered_map<AmelasServerCommand, std::function<void(const CommandRequest&, CommandReply&)>>;

    AmelasServer(unsigned port, const std::string& local_addr = "*");

    template<typename RetT = void, typename... Args>
    void registerCallback(AmelasServerCommand command,
                          controller::AmelasController* controller,
                          controller::ControllerError(controller::AmelasController::*callback)(Args...))
    {
        CallbackHandler::registerCallback(static_cast<CallbackHandler::CallbackId>(command), controller, callback);
    }

    bool hasCallback(AmelasServerCommand command)
    {
        return CallbackHandler::hasCallback(static_cast<CallbackHandler::CallbackId>(command));
    }

private:

    // Hide the invoke of the parent.
    using CallbackHandler::invokeCallback;

    // Helper to check if the custom command is valid.
    static bool validateAmelasCommand(AmelasServerCommand command);

    template <typename CallbackType, typename... Args>
    controller::ControllerError invokeCallback(const CommandRequest& request, CommandReply& reply, Args&&... args)
    {
        // Get the command.
        AmelasServerCommand cmd = static_cast<AmelasServerCommand>(request.command);

        // Check the callback.
        if(!this->hasCallback(cmd))
        {
            reply.result = static_cast<ServerResult>(AmelasServerResult::EMPTY_CALLBACK);
            return controller::ControllerError::INVALID_ERROR;
        }

        //Invoke the callback.
        try
        {
            return CallbackHandler::invokeCallback<CallbackType, controller::ControllerError>(
                static_cast<CallbackHandler::CallbackId>(cmd), std::forward<Args>(args)...);
        }
        catch(...)
        {
            reply.result = static_cast<ServerResult>(AmelasServerResult::INVALID_CALLBACK);
            return controller::ControllerError::INVALID_ERROR;
        }
    }

    template <typename CommandType, typename... Args>
    void registerProcessFunction(CommandType command,
                                 void(AmelasServer::*f)(const CommandRequest&, CommandReply&, Args...))
    {
        this->process_fnc_map_[command] = [this, f](const CommandRequest& request, CommandReply& reply)
        {
            (this->*f)(request, reply);
        };
    }

    // Process the specific commands.
    void processAmelasCommand(const CommandRequest&, CommandReply&);
    void processSetHomePosition(const CommandRequest&, CommandReply&);
    void processGetHomePosition(const CommandRequest&, CommandReply&);

    // Internal overrided custom command received callback.
    // WARNING The most important part.
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

    ProcessFunctionsMap process_fnc_map_;
};

}} // END NAMESPACES.
// =====================================================================================================================
