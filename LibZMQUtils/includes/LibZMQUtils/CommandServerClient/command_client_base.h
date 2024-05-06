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
 * @file command_client_base.h
 * @author Degoras Project Team
 * @brief This file contains the declaration of the CommandClientBase class and related.
 * @copyright EUPL License
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
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
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

// CONSTANTS
// =====================================================================================================================
constexpr unsigned kDefaultServerAliveTimeoutMsec = 10000;     ///< Default timeout for consider a server dead (msec).
constexpr unsigned kDefaultClientSendAlivePeriodMsec = 5000;   ///< Default period for sending alive commands (msec).
// =====================================================================================================================

/**
 * @brief The CommandClientBase class implements a base class for a Command Client.
 */
class LIBZMQUTILS_EXPORT CommandClientBase : public ZMQContextHandler
{

public:
    
    /**
     * @brief CommandClientBase constructor.
     * @param server_endpoint, the URL endpoint of the server
     * @param client_name, this client name.
     * @param interf_name, this name of the interface to output commands. If empty, this class will look for the best one.
     */
    CommandClientBase(const std::string& server_endpoint,
                                         const std::string& client_name = "",
                                         const std::string& interf_name = "");
    
    /**
     * @brief Get the client info.
     * @return the client info.
     */
    const HostInfo& getClientInfo() const;

    /**
     * @brief Get the server endpoint.
     * @return the server endpoint.
     */
    const std::string& getServerEndpoint() const;

    /**
     * @brief Get the client name.
     * @return the client name.
     */
    const std::string& getClientName() const;

    /**
     * @brief Check if client is working, i.e., it was started successfully.
     * @return true if it is currently working, otherwise false.
     */
    bool isWorking() const;

    /**
     * @brief Start the client. It must be called before sending commands.
     * @return true if client was started successfully. False otherwise.
     */
    bool startClient();

    /**
     * @brief Stop the client.
     */
    void stopClient();

    /**
     * @brief Reset the client.
     * @return true if client was reset successfully. Otherwise false.
     */
    bool resetClient();

    /**
     * @brief Enable or disable calling alive callbacks. These callbacks include onSendingCommand, onWaitingReply and
     * onReplyReceived.
     * @param enabled, true to enable the calling, false to disable.
     */
    void setAliveCallbacksEnabled(bool enabled);

    /**
     * @brief Set the timeout to consider a connected server dead.
     * @param timeout_msec, the timeout in millliseconds.
     */
    void setServerAliveTimeout(unsigned timeout_msec);

    /**
     * @brief Set the period for automatically sending alive messages if active.
     * @param period_msec, the period in milliseconds.
     */
    void setSendAlivePeriod(unsigned period_msec);

    /**
     * @brief If auto alive sending was enabled when connecting, stop the process.
     * @warning For enabling the process again, it is necessary to disconnect and connect again.
     */
    void disableAutoAlive();

    /**
     * @brief Validates the given command against predefined command ranges.
     *
     * This function checks if the specified command is within the valid range
     * defined by `ServerCommand::INVALID_COMMAND` and `ServerCommand::END_BASE_COMMANDS`.
     * Commands within this range are considered valid.
     *
     * @param command The command to validate, typically received as input or parsed from a message.
     * @return true If the command is within the valid range.
     * @return false If the command is outside the valid range.
     */
    template<typename CmdId>
    bool validateCommand(CmdId command) const
    {
        return (static_cast<ServerCommand>(command) > ServerCommand::INVALID_COMMAND &&
                static_cast<ServerCommand>(command) < ServerCommand::END_BASE_COMMANDS);
    }

    /**
     * @brief Try to connect to the Command Server.
     * @param auto_alive, true to enable the auto alive sending, false to disable.
     * @return the OperationResult.
     */
    OperationResult doConnect(bool auto_alive = false);

    /**
     * @brief Try to Disconnect from CommandServer.
     * @return the OperationResult.
     */
    OperationResult doDisconnect();

    /**
     * @brief Try to send a keep alive message to the Command Server
     * @return the OperationResult.
     */
    OperationResult doAlive();

    /**
     * @brief Request the time from a Command Server.
     * @param datetime, the resulting datetime obtained from the Command Server.
     * @return the OperationResult.
     */
    OperationResult doGetServerTime(std::string& datetime);

    /**
     * @brief Send a command to the Command Server.
     * @return the OperationResult.
     */
    OperationResult sendCommand(const RequestData&, CommandReply&);

    /**
     * @brief Virtual destructor to ensure proper cleanup when the derived class is destroyed.
     * @warning The client will stop if is running but in this case the `onClientStop` callback can't be executed.
     */
    virtual ~CommandClientBase() override;

protected:

    /**
     * @brief onClientStart will be called when the client starts. Must be overriden.
     */
    virtual void onClientStart() = 0;

    /**
     * @brief onClientStop will be called when the client stops. Must be overriden.
     */
    virtual void onClientStop() = 0;

    /**
     * @brief onWaitingReply will be called when the client sends a command and it is waiting for reply. Must be overriden.
     */
    virtual void onWaitingReply() = 0;

    /**
     * @brief onDeadServer will be called when the server response timeout is exceeded. Must be overriden.
     */
    virtual void onDeadServer() = 0;

    /**
     * @brief onConnected will be called when the client is connected to a server. Must be overriden.
     */
    virtual void onConnected() = 0;

    /**
     * @brief onDisconnected will be called when the client is disconnected from a server. Must be overriden.
     */
    virtual void onDisconnected() = 0;

    /**
     * @brief onInvalidMsgReceived will be called when an invalid message is received as reply. Must be overriden.
     * @param rep, the faulty reply received.
     */
    virtual void onInvalidMsgReceived(const CommandReply& rep) = 0;

    /**
     * @brief onReplyReceived will be called when a reply is received. Must be overriden.
     * @param rep, the reply received.
     */
    virtual void onReplyReceived(const CommandReply& rep) = 0;

    /**
     * @brief onSendingCommand will be called when the client is sending a command. Must be overriden.
     * @param data, the request data sent.
     */
    virtual void onSendingCommand(const RequestData& data) = 0;

    /**
     * @brief onClientError will be called whenever there is an error on the client. Must be overriden.
     * @param error, the generated error.
     * @param ext_info, a string with a description of the error.
     */
    virtual void onClientError(const zmq::error_t& error, const std::string& ext_info) = 0;

    template <typename CmdId, typename... Args>
    zmqutils::serverclient::RequestData prepareRequest(CmdId command, const Args&... args)
    {
        zmqutils::serverclient::RequestData command_msg(static_cast<zmqutils::serverclient::ServerCommand>(command));

        if constexpr (sizeof...(args) > 0)
            command_msg.params_size = zmqutils::serializer::BinarySerializer::fastSerialization(
                command_msg.params, std::forward<const Args&>(args)...);

        return command_msg;
    }

    template <typename... Args>
    zmqutils::serverclient::OperationResult executeCommand(const zmqutils::serverclient::RequestData &request,
                                                           Args&... args)
    {
        zmqutils::serverclient::CommandReply reply;
        zmqutils::serverclient::OperationResult op_res = this->sendCommand(request, reply);

        if (zmqutils::serverclient::OperationResult::COMMAND_OK == op_res)
        {
            try
            {
                zmqutils::serializer::BinarySerializer::fastDeserialization(
                    reply.params.get(), reply.params_size, std::forward<Args&>(args)...);
            }
            catch(...)
            {
                op_res = zmqutils::serverclient::OperationResult::BAD_PARAMETERS;
            }
        }

        return op_res;
    }

private:

    OperationResult recvFromSocket(CommandReply&repl, zmq::socket_t *recv_socket, zmq::socket_t *close_socket);

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
    std::future<OperationResult> fut_recv_send_;   ///< Future that stores the recv status for send command.
    std::future<OperationResult> fut_recv_alive_;  ///< Future that stores the recv status for auto alive.

    // Auto alive functionality.
    std::future<void> auto_alive_future_;
    std::condition_variable auto_alive_cv_;

    // Usefull flags.
    std::atomic_bool flag_client_working_;    ///< Flag for check the client working status.
    std::atomic_bool flag_autoalive_enabled_; ///< Flag for enables or disables the automatic sending of alive messages.
    std::atomic_bool flag_alive_callbacks_;   ///< Flag for enables or disables the callbacks for alive messages.

    // Configurable parameters.
    std::atomic_uint server_alive_timeout_;    ///< Tiemout for consider a server dead (in msec).
    std::atomic_uint send_alive_period_;       ///< Server reconnection number of attempts.
};

}} // END NAMESPACES.
// =====================================================================================================================
