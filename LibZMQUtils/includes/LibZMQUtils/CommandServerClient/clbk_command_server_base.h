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
 * @file clbk_command_server_base.h
 * @brief This file contains the declaration of the ClbkCommandServerBase class and related.
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
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/libzmqutils_global.h"
#include "LibZMQUtils/CommandServerClient/common.h"
#include "LibZMQUtils/CommandServerClient/command_server_base.h"
#include "LibZMQUtils/Utilities/callback_handler.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

// =====================================================================================================================
using common::ServerCommand;
using utils::CallbackHandler;
// =====================================================================================================================

class LIBZMQUTILS_EXPORT ClbkCommandServerBase : public CommandServerBase,
                                                 public CallbackHandler
{
public:

    ClbkCommandServerBase(unsigned port, const std::string& local_addr = "*");

    template<typename ClassT, typename RetT = void, typename... Args>
    void registerCallback(ServerCommand command, ClassT* object, RetT(ClassT::*callback)(Args...))
    {
        CallbackHandler::registerCallback(static_cast<CallbackHandler::CallbackId>(command), object, callback);
    }

    void removeCallback(ServerCommand command);

    bool hasCallback(ServerCommand command);

    virtual ~ClbkCommandServerBase() override;

protected:

    template <typename CallbackType, typename RetT = void,  typename... Args>
    RetT invokeCallback(const CommandRequest& request, CommandReply& reply, const RetT& err_ret, Args&&... args)
    {
        // Get the command.
        ServerCommand cmd = static_cast<ServerCommand>(request.command);

        // Check the callback.
        if(!this->hasCallback(cmd))
        {
            reply.result = ServerResult::EMPTY_EXT_CALLBACK;
            return err_ret;
        }

        //Invoke the callback.
        try
        {
            return CallbackHandler::invokeCallback<CallbackType, RetT>(
                static_cast<CallbackHandler::CallbackId>(cmd), std::forward<Args>(args)...);
        }
        catch(...)
        {
            reply.result = ServerResult::INVALID_EXT_CALLBACK;
            return err_ret;
        }
    }

private:

    // Hide the base functions.
    using CallbackHandler::invokeCallback;
    using CallbackHandler::removeCallback;
    using CallbackHandler::hasCallback;
};

} // END NAMESPACES.
// =====================================================================================================================
