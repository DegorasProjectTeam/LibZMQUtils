/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
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
 * @file clbk_command_server_base.h
 * @brief This file contains the declaration of the ClbkCommandServerBase class and related.
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
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/CommandServerClient/common.h"
#include "LibZMQUtils/CommandServerClient/command_server_base.h"
#include "LibZMQUtils/Utilities/callback_handler.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace serverclient{
// =====================================================================================================================

/**
 * @brief The ClbkCommandServerBase class implements a CommandServer that includes callback handling for each command.
 */
class ClbkCommandServerBase : public CommandServerBase,
                              public utils::CallbackHandler
{
public:

    /**
     * @brief ClbkCommandServerBase default constructor.
     */
    LIBZMQUTILS_EXPORT ClbkCommandServerBase(unsigned port, const std::string& local_addr = "*");

    /**
     * @brief Template function for registering a callback. This callback will be registered for a specific command.
     * @param command, the command the callback is applied to.
     * @param object, a parametric object whose method will be called.
     * @param callback, the callback method that will be called.
     */
    template<typename ClassT, typename RetT = void, typename... Args>
    void registerCallback(ServerCommand command, ClassT* object, RetT(ClassT::*callback)(Args...))
    {
        CallbackHandler::registerCallback(static_cast<CallbackHandler::CallbackId>(command), object, callback);
    }

    /**
     * @brief Remove the registered callback for a specific command.
     * @param command, the command whose callback will be erased.
     */
    LIBZMQUTILS_EXPORT void removeCallback(ServerCommand command);

    /**
     * @brief Check if there is a registered callback for a specific command.
     * @param command, the command whose callback existence will be checked.
     * @return
     */
    LIBZMQUTILS_EXPORT bool hasCallback(ServerCommand command);

    /**
     * @brief Virtual destructor.
     */
    LIBZMQUTILS_EXPORT virtual ~ClbkCommandServerBase() override;

protected:

    /**
     * @brief Parametric method for invoking a registed callback. If no callback is registered, an error is returned.
     * @param msg, the received message.
     * @param args, the args passed to the callback.
     * @return the result of the callback inovocation.
     */
    template <typename CallbackType, typename RetT = void,  typename... Args>
    RetT invokeCallback(const CommandRequest& request, CommandReply& reply, const RetT& err_ret, Args&&... args)
    {
        // Get the command.
        ServerCommand cmd = static_cast<ServerCommand>(request.command);

        // Check the callback.
        if(!this->hasCallback(cmd))
        {
            reply.server_result = OperationResult::EMPTY_EXT_CALLBACK;
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
            reply.server_result = OperationResult::INVALID_EXT_CALLBACK;
            return err_ret;
        }
    }

private:

    // Hide the base functions.
    using CallbackHandler::invokeCallback;
    using CallbackHandler::removeCallback;
    using CallbackHandler::hasCallback;
};

}} // END NAMESPACES.
// =====================================================================================================================
