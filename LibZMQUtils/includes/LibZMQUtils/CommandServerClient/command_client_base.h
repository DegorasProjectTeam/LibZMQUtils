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
constexpr unsigned kDefaultServerAliveTimeoutMsec = 2000;     ///< Default timeout for consider a server dead (msec).
constexpr unsigned kDefaultClientSendAlivePeriodMsec = 1000;   ///< Default period for sending alive commands (msec).
// =====================================================================================================================

/**
 * @brief The CommandClientBase class implements a base class for a Command Client.
 */
class LIBZMQUTILS_EXPORT CommandClientBase : public ZMQContextHandler
{

public:
    
    /**
     * @brief Base constructor for a ZeroMQ command client.
     *
     *
     * @param server_endpoint The URL endpoint of the server with the port.
     * @param net_interface Name of the network interface to be used. If empty, the class will look for the best one.
     * @param client_name Optional parameter to specify the server name. By default is empty.
     * @param client_version Optional parameter to specify the server version (like "1.1.1"). By default is empty.
     * @param client_info Optional parameter to specify the server information. By default is empty.
     *
     */
    CommandClientBase(const std::string& server_endpoint,
                      const std::string& client_name = "",
                      const std::string& client_version = "",
                      const std::string& client_info = "",
                      const std::string& net_interface = "");
    
    /**
     * @brief Get the client info.
     * @return the client info.
     */
    const ClientInfo& getClientInfo() const;

    /**
     * @brief Get the server endpoint.
     * @return the server endpoint.
     */
    const std::string& getServerEndpoint() const;

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
     * @param timeout, the timeout in millliseconds.
     */
    void setServerAliveTimeout(const std::chrono::milliseconds& timeout);

    /**
     * @brief Set the period for automatically sending alive messages if active.
     * @param period, the period in milliseconds.
     */
    void setSendAlivePeriod(const std::chrono::milliseconds& period);

    /**
     * @brief If auto alive sending was enabled when connecting, stop the process.
     * @warning For enabling the process again, it is necessary to disconnect and connect again.
     */
    void disableAutoAlive();

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

protected:

    /**
     * @brief Base client start callback. Subclasses must override this function.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onClientStart() = 0;

    /**
     * @brief Base client stop callback. Subclasses must override this function.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onClientStop() = 0;

    /**
     * @brief Base waiting reply callback. Subclasses must override this function.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onWaitingReply() = 0;

    /**
     * @brief Base dead server callback. Subclasses must override this function.
     *
     * @param The ServerInfo object representing the dead server.
     *
     * @warning The overridden callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onDeadServer(const ServerInfo&) = 0;

    /**
     * @brief Base connected callback. Subclasses must override this function.
     *
     * @param The ServerInfo object representing the server where the client is connected.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onConnected(const ServerInfo&) = 0;

    /**
     * @brief Base disconnected callback. Subclasses must override this function.
     *
     * @param The ServerInfo object representing the server where the client is connected.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onDisconnected(const ServerInfo&) = 0;

    /**
     * @brief onBadOperation will be called when an invalid message is received as reply. Must be overriden.
     * @param rep, the faulty reply received.
     */
    virtual void onBadOperation(const CommandReply& rep) = 0;

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
    static zmqutils::serverclient::RequestData prepareRequest(CmdId command, const Args&... args)
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

    void recvFromSocket(CommandReply&repl, zmq::socket_t *recv_socket, zmq::socket_t *close_socket);

    void deleteSockets();

    void internalStopClient();

    bool internalResetClient();

    void startAutoAlive();

    void stopAutoAlive();

    void aliveWorker();

    zmq::multipart_t prepareMessage(const RequestData &msg);

    // Internal client identification.
    ClientInfo client_info_;       ///< External client information for identification.

    // Server information.
    ServerInfo connected_server_info_;  ///< Connected server information.

    // ZMQ sockets and endpoint.
    const std::string server_endpoint_;  ///< Server endpoint.
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
    std::future<void> fut_recv_send_;   ///< Future that stores the recv status for send command.
    std::future<void> fut_recv_alive_;  ///< Future that stores the recv status for auto alive.

    // Auto alive functionality.
    std::future<void> auto_alive_future_;
    std::condition_variable auto_alive_cv_;

    // Usefull flags.
    std::atomic_bool flag_client_working_;     ///< Flag for check the client working status.
    std::atomic_bool flag_autoalive_enabled_;  ///< Flag for enables or disables the automatic sending of alive messages.
    std::atomic_bool flag_alive_callbacks_;    ///< Flag for enables or disables the callbacks for alive messages.
    std::atomic_bool flag_server_connected_;   ///< Flag for check if, in a certain momment, the client was alive.

    // Configurable parameters.
    std::atomic_uint server_alive_timeout_;    ///< Tiemout for consider a server dead (in msec).
    std::atomic_uint send_alive_period_;       ///< Server reconnection number of attempts.
};

}} // END NAMESPACES.
// =====================================================================================================================
