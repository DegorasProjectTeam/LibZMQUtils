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
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <future>
#include <string>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Global/zmq_context_handler.h"
#include "LibZMQUtils/CommandServerClient/common.h"
// =====================================================================================================================

namespace zmq
{
    class socket_t;
}

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace serverclient{
// =====================================================================================================================

/**
 * @brief The CommandClientBase class implements a base class for a Command Client.
 */
class CommandClientBase : public ZMQContextHandler
{

public:
    
    /**
     * @brief CommandClientBase constructor.
     * @param server_endpoint, the URL endpoint of the server
     * @param client_name, this client name.
     * @param interf_name, this name of the interface to output commands. If empty, this class will look for the best one.
     */
    LIBZMQUTILS_EXPORT CommandClientBase(const std::string& server_endpoint,
                                         const std::string& client_name = "",
                                         const std::string& interf_name = "");
    
    /**
     * @brief Get the client info.
     * @return the client info.
     */
    LIBZMQUTILS_EXPORT const HostInfo& getClientInfo() const;

    /**
     * @brief Get the server endpoint.
     * @return the server endpoint.
     */
    LIBZMQUTILS_EXPORT const std::string& getServerEndpoint() const;

    /**
     * @brief Get the client name.
     * @return the client name.
     */
    LIBZMQUTILS_EXPORT const std::string& getClientName() const;

    /**
     * @brief Check if client is working, i.e., it was started successfully.
     * @return true if it is currently working, otherwise false.
     */
    LIBZMQUTILS_EXPORT bool isWorking() const;

    /**
     * @brief Start the client. It must be called before sending commands.
     * @return true if client was started successfully. False otherwise.
     */
    LIBZMQUTILS_EXPORT bool startClient();

    /**
     * @brief Stop the client.
     */
    LIBZMQUTILS_EXPORT void stopClient();

    /**
     * @brief Reset the client.
     * @return true if client was reset successfully. Otherwise false.
     */
    LIBZMQUTILS_EXPORT bool resetClient();

    /**
     * @brief Enable or disable calling alive callbacks. These callbacks include onSendingCommand, onWaitingReply and
     * onReplyReceived.
     * @param enabled, true to enable the calling, false to disable.
     */
    LIBZMQUTILS_EXPORT void setAliveCallbacksEnabled(bool enabled);

    /**
     * @brief If auto alive sending was enabled when connecting, stop the process.
     * @warning For enabling the process again, it is necessary to disconnect and connect again.
     */
    LIBZMQUTILS_EXPORT void disableAutoAlive();

    /**
     * @brief Try to connect to the Command Server.
     * @param auto_alive, true to enable the auto alive sending, false to disable.
     * @return the ClientResult of the operation.
     */
    LIBZMQUTILS_EXPORT ClientResult doConnect(bool auto_alive = false);

    /**
     * @brief Try to Disconnect from CommandServer.
     * @return the ClientResult of the operation.
     */
    LIBZMQUTILS_EXPORT ClientResult doDisconnect();

    /**
     * @brief Try to send a keep alive message to the Command Server
     * @return the ClientResult of the operation.
     */
    LIBZMQUTILS_EXPORT ClientResult doAlive();

    /**
     * @brief Request the time from a Command Server.
     * @param datetime, the resulting datetime obtained from the Command Server.
     * @return the ClientResult of the operation.
     */
    LIBZMQUTILS_EXPORT ClientResult doGetServerTime(std::string& datetime);

    /**
     * @brief Send a command to the Command Server.
     * @return the ClientResult of the operation.
     */
    LIBZMQUTILS_EXPORT ClientResult sendCommand(const RequestData&, CommandReply&);

    /**
     * @brief Virtual destructor to ensure proper cleanup when the derived class is destroyed.
     * @warning The client will stop if is running but in this case the `onClientStop` callback can't be executed.
     */
    LIBZMQUTILS_EXPORT virtual ~CommandClientBase() override;

protected:

    /**
     * @brief onClientStart will be called when the client starts. Must be overriden.
     */
    LIBZMQUTILS_EXPORT virtual void onClientStart() = 0;

    /**
     * @brief onClientStop will be called when the client stops. Must be overriden.
     */
    LIBZMQUTILS_EXPORT virtual void onClientStop() = 0;

    /**
     * @brief onWaitingReply will be called when the client sends a command and it is waiting for reply. Must be overriden.
     */
    LIBZMQUTILS_EXPORT virtual void onWaitingReply() = 0;

    /**
     * @brief onDeadServer will be called when the server response timeout is exceeded. Must be overriden.
     */
    LIBZMQUTILS_EXPORT virtual void onDeadServer() = 0;

    /**
     * @brief onConnected will be called when the client is connected to a server. Must be overriden.
     */
    LIBZMQUTILS_EXPORT virtual void onConnected() = 0;

    /**
     * @brief onDisconnected will be called when the client is disconnected from a server. Must be overriden.
     */
    LIBZMQUTILS_EXPORT virtual void onDisconnected() = 0;

    /**
     * @brief onInvalidMsgReceived will be called when an invalid message is received as reply. Must be overriden.
     * @param rep, the faulty reply received.
     */
    LIBZMQUTILS_EXPORT virtual void onInvalidMsgReceived(const CommandReply& rep) = 0;

    /**
     * @brief onReplyReceived will be called when a reply is received. Must be overriden.
     * @param rep, the reply received.
     */
    LIBZMQUTILS_EXPORT virtual void onReplyReceived(const CommandReply& rep) = 0;

    /**
     * @brief onSendingCommand will be called when the client is sending a command. Must be overriden.
     * @param data, the request data sent.
     */
    LIBZMQUTILS_EXPORT virtual void onSendingCommand(const RequestData& data) = 0;

    /**
     * @brief onClientError will be called whenever there is an error on the client. Must be overriden.
     * @param error, the generated error.
     * @param ext_info, a string with a description of the error.
     */
    LIBZMQUTILS_EXPORT virtual void onClientError(const zmq::error_t& error, const std::string& ext_info) = 0;

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
    HostInfo client_info_;       ///< External client information for identification.
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
    std::future<ClientResult> fut_recv_send_;   ///< Future that stores the client recv status for send command.
    std::future<ClientResult> fut_recv_alive_;  ///< Future that stores the client recv status for auto alive.

    // Auto alive functionality.
    std::future<void> auto_alive_future_;
    std::condition_variable auto_alive_cv_;

    // Usefull flags.
    std::atomic_bool flag_client_working_;    ///< Flag for check the client working status.
    std::atomic_bool flag_autoalive_enabled_; ///< Flag for enables or disables the automatic sending of alive messages.
    std::atomic_bool flag_alive_callbacks_;   ///< Flag for enables or disables the callbacks for alive messages.
};

}} // END NAMESPACES.
// =====================================================================================================================
