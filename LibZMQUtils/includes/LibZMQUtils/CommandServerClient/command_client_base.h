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
 * @file command_client_base.h
 * @brief This file contains the declaration of the CommandClientBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <future>
#include <string>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Global/zmq_context_handler.h"
#include "LibZMQUtils/CommandServerClient/common.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

// =====================================================================================================================
using common::ServerCommand;
using common::ServerResult;
using common::ClientResult;
using common::CommandReply;
using common::CommandType;
using common::RequestData;
// =====================================================================================================================

class CommandClientBase : public ZMQContextHandler
{

public:
    
    LIBZMQUTILS_EXPORT CommandClientBase(const std::string& server_endpoint,
                                         const std::string& client_name = "",
                                         const std::string& interf_name = "");
    
    LIBZMQUTILS_EXPORT bool startClient();

    LIBZMQUTILS_EXPORT void stopClient();

    LIBZMQUTILS_EXPORT bool resetClient();

    LIBZMQUTILS_EXPORT void setAliveCallbacksEnabled(bool);

    LIBZMQUTILS_EXPORT void disableAutoAlive();

    LIBZMQUTILS_EXPORT const common::HostInfo& getClientInfo() const;

    LIBZMQUTILS_EXPORT const std::string& getServerEndpoint() const;

    LIBZMQUTILS_EXPORT const std::string& getClientName() const;

    LIBZMQUTILS_EXPORT bool isWorking() const;

    LIBZMQUTILS_EXPORT ClientResult doConnect(bool auto_alive = false);

    LIBZMQUTILS_EXPORT ClientResult doDisconnect();

    LIBZMQUTILS_EXPORT ClientResult doAlive();

    LIBZMQUTILS_EXPORT ClientResult doGetServerTime(std::string& datetime);

    LIBZMQUTILS_EXPORT ClientResult sendCommand(const RequestData&, CommandReply&);

    /**
     * @brief Virtual destructor to ensure proper cleanup when the derived class is destroyed.
     * @warning The client will stop if is running but in this case the `onClientStop` callback can't be executed.
     */
    LIBZMQUTILS_EXPORT virtual ~CommandClientBase() override;

protected:

    LIBZMQUTILS_EXPORT virtual void onClientStart() = 0;

    LIBZMQUTILS_EXPORT virtual void onClientStop() = 0;

    LIBZMQUTILS_EXPORT virtual void onWaitingReply() = 0;

    LIBZMQUTILS_EXPORT virtual void onDeadServer() = 0;

    LIBZMQUTILS_EXPORT virtual void onConnected() = 0;

    LIBZMQUTILS_EXPORT virtual void onDisconnected() = 0;

    LIBZMQUTILS_EXPORT virtual void onInvalidMsgReceived(const CommandReply&) = 0;

    LIBZMQUTILS_EXPORT virtual void onReplyReceived(const CommandReply&) = 0;

    LIBZMQUTILS_EXPORT virtual void onSendingCommand(const RequestData&) = 0;

    LIBZMQUTILS_EXPORT virtual void onClientError(const zmq::error_t&, const std::string& ext_info) = 0;

private:

    ClientResult recvFromSocket(CommandReply&repl, zmq::socket_t *recv_socket, zmq::socket_t *close_socket);

    void deleteSockets();

    void internalStopClient();

    bool internalResetClient();

    void startAutoAlive();

    void stopAutoAlive();

    void aliveWorker();

    zmq::multipart_t prepareMessage(const RequestData &msg);

    // Internal client identification.
    common::HostInfo client_info_;       ///< External client information for identification.
    std::string client_name_;            ///< Internal client name. Will not be use as id.

    // ZMQ sockets and endpoint.
    std::string server_endpoint_;        ///< Server endpoint.
    zmq::socket_t *client_socket_;       ///< ZMQ client socket.
    zmq::socket_t *recv_close_socket_;   ///< ZMQ auxiliar socket for requesting to close.
    zmq::socket_t *req_close_socket_;    ///< ZMQ auxiliar socket for receiving the close request.

    // Condition variables with associated flags.
    std::condition_variable stopped_done_cv_;
    std::atomic_bool flag_client_closed_;

    // Mutex.
    mutable std::mutex mtx_;                    ///< Safety mutex.
    mutable std::mutex client_close_mtx_;       ///< Safety mutex for closing client.

    // Futures for receiving response from send command and auto alive
    std::future<common::ClientResult> fut_recv_send_;   ///< Future that stores the client recv status for send command.
    std::future<common::ClientResult> fut_recv_alive_;  ///< Future that stores the client recv status for auto alive.

    // Auto alive functionality.
    std::future<void> auto_alive_future_;
    std::condition_variable auto_alive_cv_;

    // Usefull flags.
    std::atomic_bool flag_client_working_;    ///< Flag for check the client working status.
    std::atomic_bool flag_autoalive_enabled_; ///< Flag for enables or disables the automatic sending of alive messages.
    std::atomic_bool flag_alive_callbacks_;   ///< Flag for enables or disables the callbacks for alive messages.
};

} // END NAMESPACES.
// =====================================================================================================================
