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
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
#include "LibZMQUtils/CommandServerClient/data/command_server_client_data.h"
#include "LibZMQUtils/CommandServerClient/data/command_server_client_info.h"
// =====================================================================================================================

namespace zmq
{
    class socket_t;
}

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace reqrep{
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
     * @param server_endpoint The URL endpoint of the server with the port.
     * @param net_interface   Name of the network interface to be used. If empty, the class will look for the best one.
     * @param client_name     Optional parameter to specify the server name. By default is empty.
     * @param client_version  Optional parameter to specify the server version (like "1.1.1"). By default is empty.
     * @param client_info     Optional parameter to specify the server information. By default is empty.
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
    const CommandClientInfo& getClientInfo() const;

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
     * @brief Checks if the server is considered currently connected.
     *
     * This function checks the internal flag to determine if the client considers itself connected to the server.
     * However, it's important to note that if auto-alive checks are disabled, this function may return `true` even
     * when the actual connection has been lost. The actual connection state may not be accurately known until a new
     * request is sent to the server.
     *
     * @return True if the client considers itself connected, false if definitely disconnected.
     */
    bool isConnected() const;

    /**
     * @brief Determines if the server has been seen at any point.
     *
     * This function returns the status of whether the server has been detected or interacted with at some point.
     * If this function returns true, it indicates that the server was at least once available or responsive,
     * and it may be possible to check the last time the server was seen, depending on additional system capabilities.
     *
     * @return True if the server was seen at any time, false otherwise.
     */
    bool serverWasSeen();

    /**
     * @brief Determines if the server has been seen at any point and retrieves the last seen time point if so.
     *
     * This function locks the associated mutex to safely check the server's seen flag and, if the server
     * has been seen, updates the provided time point parameter with the time the server was last seen.
     * This allows callers to not only check if the server has been detected at any point but also to
     * retrieve the exact moment of the last interaction if it occurred.
     *
     * @param[out] tp Reference to a time point variable that will be set to the last seen time of the server.
     *                This parameter is only modified if the server was indeed seen.
     *
     * @return True if the server was seen at any time, allowing the tp parameter to be updated; false otherwise.
     */
    bool serverWasSeen(utils::HRTimePointStd& tp);

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
     * @param datetime The resulting datetime obtained from the Command Server.
     * @return The OperationResult.
     */
    OperationResult doGetServerTime(std::string& datetime);


    /**
     * @brief Send ping to server.
     * @param elapsed_time The time elapsed between sending ping and receiving answer.
     * @return The result of the operation.
     * @note elapsed_time is calculated whether if operation is successful or not.
     */
    OperationResult doPing(std::chrono::milliseconds &elapsed_time);

    /**
     * @brief Send a command to the Command Server.
     * @return the OperationResult.
     */
    OperationResult sendCommand(ServerCommand command, RequestData& request_data, CommandReply& reply);

    OperationResult sendCommand(ServerCommand command, CommandReply& reply);

    OperationResult sendCommand(ServerCommand command);

    /**
     * @brief Send a command to the Command Server.
     * @return the OperationResult.
     */
    template <typename T>
    OperationResult sendCommand(T command, RequestData& request_data, CommandReply& reply)
    {
        return this->sendCommand(static_cast<ServerCommand>(command), request_data, reply);
    }

    template <typename T>
    OperationResult sendCommand(T command, CommandReply& reply)
    {
        RequestData empty_data;
        return this->sendCommand(static_cast<ServerCommand>(command), empty_data, reply);
    }

    /**
     * @brief Checks if the given ServerCommand command is a base server command.
     *
     * This function checks if the specified command is within the valid range defined
     * by `ServerCommand::INVALID_COMMAND` and `ServerCommand::END_BASE_COMMANDS`.
     *
     * @param command The command to validate, typically received as input or parsed from a message.
     * @return Returns true if the command is within the base command range or false if it is outside the range.
     */
    bool isBaseCommand(ServerCommand command) const;

    /**
     * @brief Checks if the given command (as CommandType) is a base server command.
     *
     * This function checks if the specified command is within the valid range defined
     * by `ServerCommand::INVALID_COMMAND` and `ServerCommand::END_BASE_COMMANDS`.
     *
     * @param command The command to validate, typically received as input or parsed from a message.
     * @return Returns true if the command is within the base command range or false if it is outside the range.
     */
    bool isBaseCommand(CommandType command) const;

    /**
     * @brief Converts a ServerCommand to its string representation.
     *
     * This function takes a ServerCommand enum value and returns its corresponding string representation. If a custom
     * command-to-string function is registered, it will be used. If the command is invalid, "INVALID_COMMAND" will be
     * returned. If the command value is within a valid range, the corresponding string will be returned. Otherwise,
     * "UNKNOWN_COMMAND" will be returned.
     *
     * @param command The ServerCommand enum value to convert.
     * @return The string representation of the command.
     */
    std::string serverCommandToString(ServerCommand command) const;

    /**
     * @brief Converts a ServerCommand (as CommandType raw value) to its string representation.
     *
     * This function takes a CommandType raw value and returns its corresponding string representation. If a custom
     * command-to-string function is registered, it will be used. If the command is invalid, "INVALID_COMMAND" will be
     * returned. If the command value is within a valid range, the corresponding string will be returned. Otherwise,
     * "UNKNOWN_COMMAND" will be returned.
     *
     * @param command The CommandType raw value to convert.
     * @return The string representation of the command.
     */
    std::string serverCommandToString(CommandType command) const;

    /**
     * @brief Converts a OperationResult to its string representation.
     *
     * This function takes a OperationResult enum value and returns its corresponding string representation. If the
     * result is invalid, "INVALID_OPERATION_RESULT" will be returned. If the result value is within a valid range,
     * the corresponding string will be returned. Otherwise, "UNKNOWN_OPERATION_RESULT" will be returned.
     *
     * @param result The OperationResult enum value to convert.
     * @return The string representation of the result.
     */
    static std::string operationResultToString(OperationResult result);

    /**
     * @brief Converts a OperationResult (as ResultType raw value) to its string representation.
     *
     * This function takes a ResultType raw value and returns its corresponding string representation. If the result is
     * invalid, "INVALID_OPERATION_RESULT" will be returned. If the result value is within a valid range, the
     * corresponding string will be returned. Otherwise, "UNKNOWN_OPERATION_RESULT" will be returned.
     *
     * @param result The ResultType raw value to convert.
     * @return The string representation of the result.
     */
    static std::string operationResultToString(ResultType result);

    /**
     * @brief Virtual destructor to ensure proper cleanup when the derived class is destroyed.
     * @warning The client will stop if is running but in this case the `onClientStop` callback can't be executed.
     */
    virtual ~CommandClientBase() override;


protected:

    template <std::size_t N1>
    void registerCommandToStrLookup(const std::array<const char*, N1>& lookup_array)
    {
        // Extend the commands string array.
        auto ext_cmds = zmqutils::utils::joinArraysConstexpr(ServerCommandStr, lookup_array);

        // Process function lambda.
        auto lambda_get_cmd_str = [ext_cmds](ServerCommand command)
        {
            std::int32_t enum_val = static_cast<std::int32_t>(command);
            std::size_t idx = static_cast<std::size_t>(command);
            std::string cmd_str = "UNKNOWN_COMMAND";

            if (enum_val < 0)
                cmd_str = "INVALID_COMMAND";
            else if (idx < std::size(ext_cmds))
                cmd_str = ext_cmds[idx];

            return cmd_str;
        };

        // Store the function.
        this->command_to_string_function_ = lambda_get_cmd_str;
    }

    /**
     * @brief Prepare a binarized RequestData container with the data that the commands need for the execution.
     *
     * This function creates a `RequestData` object with a specific data. The parameters are serialized into the
     * container `RequestData` object, which can be sent to a server.
     *
     * @param args Arguments that will be binary serialized into the `RequestData` object.
     * @return The RequestData struct containing the serialized parameters.
     */
    template <typename... Args>
    static zmqutils::reqrep::RequestData prepareRequestData(const Args&... args)
    {
        RequestData data;
        if constexpr (sizeof...(args) > 0)
            data.size = zmqutils::serializer::BinarySerializer::fastSerialization(
                data.bytes, std::forward<const Args&>(args)...);
        return data;
    }

    /**
     * @brief Execute a command by sending a prepared request and handling the response.
     *
     * This function sends a previously prepared `RequestData` to the server and processes the returned response. If
     * the command execution is successful (`COMMAND_OK`), the returned parameters are deserialized into the provided
     * arguments. The function returns the OperationResult value.
     *
     * @param cmd     The command to be executed.
     * @param request The `RequestData` object representing the command to be executed.
     * @param args    Output parameters where the deserialized response data will be stored.
     *
     * @return The OperationResult result of the command execution.
     */
    template <typename Cmd, typename... Args>
    zmqutils::reqrep::OperationResult executeCommand(Cmd cmd, zmqutils::reqrep::RequestData& request, Args&... args)
    {
        // Prepare the reply container.
        zmqutils::reqrep::CommandReply reply;

        // Send the command.
        zmqutils::reqrep::OperationResult op_res = this->sendCommand(cmd, request, reply);

        // Check the operation result.
        if (zmqutils::reqrep::OperationResult::COMMAND_OK == op_res)
        {
            try
            {
                zmqutils::serializer::BinarySerializer::fastDeserialization(
                    reply.data.bytes.get(), reply.data.size, std::forward<Args&>(args)...);
            }
            catch(...)
            {
                op_res = zmqutils::reqrep::OperationResult::BAD_PARAMETERS;
            }
        }

        // Return the operation result.
        return op_res;
    }

    template <typename Cmd, typename... Args>
    zmqutils::reqrep::OperationResult executeCommand(Cmd cmd, Args&... args)
    {
        RequestData empty_data;
        return this->executeCommand(cmd, empty_data, std::forward<Args&>(args)...);
    }

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
     * @param The CommandServerInfo object representing the dead server.
     *
     * @warning The overridden callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onDeadServer(const CommandServerInfo&) = 0;

    /**
     * @brief Base connected callback. Subclasses must override this function.
     *
     * @param The CommandServerInfo object representing the server where the client is connected.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onConnected(const CommandServerInfo&) = 0;

    /**
     * @brief Base disconnected callback. Subclasses must override this function.
     *
     * @param The CommandServerInfo object representing the server where the client is connected.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onDisconnected(const CommandServerInfo&) = 0;

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
     * @param req, the request data sent.
     */
    virtual void onSendingCommand(const CommandRequest& req) = 0;

    /**
     * @brief onClientError will be called whenever there is an error on the client. Must be overriden.
     * @param error, the generated error.
     * @param ext_info, a string with a description of the error.
     */
    virtual void onClientError(const zmq::error_t& error, const std::string& ext_info) = 0;

private:

    /// Alias for a function that allows transform a ServerCommand to a string.
    using CommandToStringFunction = std::function<std::string(ServerCommand)>;

    /// Internal function for receive from socket.
    void recvFromSocket(CommandReply&repl, zmq::socket_t *recv_socket, zmq::socket_t *close_socket);

    /// Internal function to delete the sockets.
    void deleteSockets();

    /// Internal function to stop the client.
    void internalStopClient();

    /// Internal function to reset the client.
    bool internalResetClient();

    /// Internal function to start the auto alive functionality.
    void startAutoAlive();

    /// Internal function to stop the auto alive functionality.
    void stopAutoAlive();

    /// Internal alive function that acts as a thread worker.
    void aliveWorker();

    /// Internal static function for preparing the messages with the request data.
    static zmq::multipart_t prepareMessage(const CommandRequest& command_request);

    // Internal client identification.
    CommandClientInfo client_info_;       ///< External client information for identification.

    // Server information.
    CommandServerInfo connected_server_info_;  ///< Connected server information.

    // ZMQ sockets and endpoint.
    const std::string server_endpoint_;  ///< Server endpoint.
    zmq::socket_t *client_socket_;       ///< ZMQ client socket.
    zmq::socket_t *recv_close_socket_;   ///< ZMQ auxiliar socket for requesting to close.
    zmq::socket_t *req_close_socket_;    ///< ZMQ auxiliar socket for receiving the close request.

    // Condition variables with associated flags.
    std::condition_variable stopped_done_cv_;  ///< Stopped done condition variable.
    std::atomic_bool flag_client_closed_;      ///< Atomic flag associated to the stopped done condition variable.

    // Mutex.
    mutable std::mutex mtx_;                    ///< Safety mutex.
    mutable std::mutex client_close_mtx_;       ///< Safety mutex for closing client.

    // Futures for receiving response from send command and auto alive
    std::future<void> fut_recv_send_;   ///< Future that stores the recv status for send command.
    std::future<void> fut_recv_alive_;  ///< Future that stores the recv status for auto alive.

    // Auto alive functionality.
    std::future<void> auto_alive_future_;     ///< Future for the auto alive worker.
    std::condition_variable auto_alive_cv_;   ///< Auto alive condition variable for check status.

    // To string functions containers.
    CommandToStringFunction command_to_string_function_;  ///< Function to transform ServerCommand into strings.

    // Usefull flags.
    std::atomic_bool flag_client_working_;     ///< Flag for check the client working status.
    std::atomic_bool flag_autoalive_enabled_;  ///< Flag for enables or disables the automatic sending of alive messages.
    std::atomic_bool flag_alive_callbacks_;    ///< Flag for enables or disables the callbacks for alive messages.
    std::atomic_bool flag_server_connected_;   ///< Flag that indicates if the client considers connected to server.
    std::atomic_bool flag_server_seen_;        ///< Flag that is true if the server was seen in some momment.

    // Configurable parameters.
    std::atomic_uint server_alive_timeout_;    ///< Tiemout for consider a server dead (in msec).
    std::atomic_uint send_alive_period_;       ///< Server reconnection number of attempts.
};

}} // END NAMESPACES.
// =====================================================================================================================
