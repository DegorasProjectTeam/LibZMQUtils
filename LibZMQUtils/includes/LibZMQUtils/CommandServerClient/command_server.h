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
 * @file command_server.h
 * @brief This file contains the declaration of the CommandServerBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2307.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <any>
#include <future>
#include <map>
#include <variant>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/libzmqutils_global.h"
#include "LibZMQUtils/CommandServerClient/common.h"
#include "LibZMQUtils/Utilities/utils.h"
#include "LibZMQUtils/Utilities/binary_serializer.h"
// =====================================================================================================================

// ZMQ DECLARATIONS
// =====================================================================================================================
namespace zmq
{
    class context_t;
    class socket_t;
}
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

// =====================================================================================================================
using common::ServerResultStr;
using common::CommandReply;
using common::CommandRequest;
using common::ServerCommand;
using common::ServerResult;
using common::HostClient;
using utils::NetworkAdapterInfo;
// =====================================================================================================================

/**
 * @class CommandServerBase
 *
 * @brief This class provides the base structure for a ZeroMQ based command server.
 *
 * The CommandServerBase class encapsulates the common logic and functionality for a server that communicates over the
 * ZeroMQ messaging infrastructure. It provides the basic mechanics for starting, stopping, and managing a server, and
 * for handling client connections, commands, and responses.
 *
 * This base class is designed to be inherited by subclasses that provide specific implementations for various callback
 * functions to handle server events such as the start/stop of the server, client connections/disconnections, receiving
 * invalid or custom commands, and server errors. This design allows the creation of specialized servers for different
 * use cases while keeping the core logic generic and reusable.
 *
 * The server created with this class operates asynchronously, with the main server tasks running in a separate thread.
 * It is capable of managing multiple client connections, processing command requests, and sending responses. The server
 * also provides optional functionalities such as checking the alive status of connected clients.
 *
 * @note This class is not directly useful on its own. Instead, it is intended to be subclassed and its callback methods
 *       overridden to implement the desired server behavior.
 *
 * @warning When creating a subclass, make sure to avoid blocking or computationally intensive operations within the
 *          overridden callbacks. Blocking the server thread can affect the server's performance and responsiveness. If
 *          complex tasks are needed, consider performing them asynchronously or using separate threads.
 *
 * @section Usage
 *
 * To use this class, create a subclass and override the callback functions according to your needs. Also you can define
 * the custom commands and the custom errors related with the sublcass, as well as extend the containers that contains
 * the string representation of the commands and errors.
 *
 * Then, create an instance of your subclass, and use the startServer and stopServer methods to control the server's
 * operation. You can query the server's state and information using the various getters (getServerPort,
 * getServerAddresses, getServerEndpoint, getServerWorkerFuture, getConnectedClients, and isWorking). You can also use
 * setClientStatusCheck to control the checking of clients' alive status.
 *
 * A similar usage pattern applies to the CommandClientBase class, which is meant to interact with a CommandServerBase
 * instance. CommandClientBase is also designed to be subclassed with callback methods to be overridden for specific
 * client behaviors. Therefore, a typical usage scenario involves creating subclassed instances of both classes
 * CommandServerBase and CommandClientBase, where the server handles commands sent by the client.
 *
 * @section Callbacks
 *
 * The following callbacks need to be overridden in your subclass:
 *
 * - onServerStop
 * - onServerStart
 * - onWaitingCommand
 * - onConnected(const HostClient&)
 * - onDisconnected(const HostClient&)
 * - onDeadClient(const HostClient&)
 * - onInvalidMsgReceived(const CommandRequest&)
 * - onCommandReceived(const CommandRequest&)
 * - onCustomCommandReceived(const CommandRequest&, CommandReply&)
 * - onServerError(const zmq::error_t &error, const std::string& ext_info = "")
 * - onSendingResponse(const CommandReply&)
 *
 * Among these callbacks, onCustomCommandReceived is crucial because it handles all custom commands. This function
 * receives a command request and a command reply as parameters. It is intended to deserialize the input parameters
 * from the command request, execute the custom command, and then serialize the output parameters into the command
 * reply. Therefore, it is necessary to override this function to add handling for the custom commands specific to
 * your server's application.
 *
 * @see ServerCommand, ServerResult, CommandRequest, CommandReply, CommandClientBase, onCustomCommandReceived
 */
class LIBZMQUTILS_EXPORT CommandServerBase
{

public:

    /**
     * @brief Base constructor for a ZeroMQ command server.
     *
     * This constructor initializes a ZeroMQ based command server with the specified port for listening to
     * incoming requests. Additionally, it allows specifying local addresses on which the server will accept
     * connections. By default, the server will accept connections on all available local addresses.
     *
     * @param port The port number on which the server will listen for incoming requests.
     *
     * @param local_addr Optional parameter to specify the local addresses on which the server will accept
     *                   connections. By default, it is set to "*", which means the server will accept
     *                   connections on all available local addresses.
     *
     * @note The server created with this constructor will be a base server and it doesn't have the complete
     *       implementation of specific request-response logic. It is intended to be subclassed to provide
     *       custom request handling. You can implement the "onCustomCommandReceived" function as an internal
     *       callback in the subclass to handle incoming requests and provide the desired response logic.
     *
     * @warning When specifying the `local_addr`, ensure it is a valid IP address present on the system.
     *          Incorrect or unavailable addresses may result in connection failures.
     */
    CommandServerBase(unsigned port, const std::string &local_addr = "*");

    /**
     * @brief Get the port number used by the server for incoming connections.
     * @return A const reference to the port number of the server.
     */
    const unsigned& getServerPort() const;

    /**
     * @brief Get the network adapter addresses used by the server.
     *
     * This function returns a const reference to a vector of NetworkAdapterInfo objects. Each NetworkAdapterInfo
     * object contains information about a network adapter used by the server for communication.
     *
     * @return A const reference to a vector of NetworkAdapterInfo objects.
     */
    const std::vector<NetworkAdapterInfo> &getServerAddresses() const;

    /**
     * @brief Get the endpoint of the server.
     *
     * This function returns a const reference to a string representing the server's
     * endpoint. The endpoint typically includes the IP address and port number.
     *
     * @return A const reference to the server's endpoint.
     */
    const std::string& getServerEndpoint() const;

    /**
     * @brief Get the future associated with the server's worker thread.
     *
     * This function returns a const reference to a std::future<void> object representing the asynchronous
     * worker thread that is running the server. The std::future object can be used to check the status of
     * the worker thread or wait for it to complete.
     *
     * @return A const reference to the server's worker thread future.
     */
    const std::future<void>& getServerWorkerFuture() const;

    /**
     * @brief Get a const reference to the map of connected clients.
     *
     * This function returns a const reference to a std::map<std::string, HostClient> representing the list of
     * connected clients. Each entry in the map consists of a string key (client identifier) and a HostClient
     * object containing information about the connected client.
     *
     * @return A const reference to the map of connected clients.
     */
    const std::map<std::string, HostClient>& getConnectedClients() const;

    /**
     * @brief Check if the server is currently working.
     *
     * This function returns a boolean value indicating whether the server is
     * currently active and working. If the server is working, it means it is
     * processing incoming connections or performing its intended tasks.
     *
     * @return True if the server is working, false otherwise.
     */
    bool isWorking() const{return this->flag_server_working_;}

    /**
     * @brief Enables or disables the client's alive status checking.
     *
     * Enables or disables the checking of the client's alive status. This is a very important
     * functionality in the context of critical systems that often use these types of servers.
     *
     * @param The desired status of the client's alive status checking (true to enable, false to disable).
     *
     * @warning It is strongly recommended to keep this check active, due to the critical nature of the systems
     *          that usually use this kind of servers. Disabling the client alive status check could result in
     *          unexpected behavior or system instability in case of sudden client disconnections or failures.
     */
    void setClientStatusCheck(bool);

    /**
     * @brief Enables or disables the server callbacks when an alive message is received.
     *
     * This function controls whether the server callbacks are called upon receipt of an alive message from the client.
     * By default, the server callbacks are enabled and will be invoked when an alive message is received.
     *
     * This is especially useful for debugging purposes. When debugging server behavior, the constant invocation of callbacks
     * upon receipt of alive messages can cause clutter in the debug output. Disabling these callbacks can help streamline the
     * debugging process and focus on the critical server functionality.
     *
     * @param [in] enabled Boolean flag that determines whether callbacks are enabled (true) or disabled (false).
     */
    void setAliveCallbacksEnabled(bool);

    /**
     * @brief Starts the command server.
     *
     * If the server is already running, the function does nothing. Otherwise, it creates the ZMQ
     * context if it doesn't exist and launches the server worker in a separate thread.
     */
    void startServer();

    /**
     * @brief Stops the command server.
     *
     * If the server is already stopped, the function does nothing. Otherwise
     * deletes the ZMQ context and cleans up the connected clients.
     */
    void stopServer();

    /**
     * @brief Virtual destructor.
     *
     * This destructor is virtual to ensure proper cleanup when the derived class is destroyed.
     */
    virtual ~CommandServerBase();



protected:

    /**
     * @brief Base server stop callback. Subclasses must override this function.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onServerStop() = 0;

    /**
     * @brief Base server start callback. Subclasses must override this function.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onServerStart() = 0;

    /**
     * @brief Base waiting command callback. Subclasses must override this function.
     *
     * @note This function is intended to be called during the server's main loop when there are no incoming
     *       requests to process. Subclasses may implement this function to perform periodic checks, cleanup
     *       tasks, or other non-blocking activities while waiting for requests.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onWaitingCommand() = 0;

    /**
     * @brief Base connected callback. Subclasses must override this function.
     *
     * @param The HostClient object representing the connected client.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onConnected(const HostClient&) = 0;

    /**
     * @brief Base disconnected callback. Subclasses must override this function.
     *
     * @param The HostClient object representing the disconnected client.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onDisconnected(const HostClient&) = 0;

    /**
     * @brief Base dead client callback. Subclasses must override this function.
     *
     * @param The HostClient object representing the dead client.
     *
     * @warning The overridden callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onDeadClient(const HostClient&) = 0;

    /**
     * @brief Base invalid message received callback. Subclasses must override this function.
     *
     * @param The CommandRequest object representing the invalid command request.
     *
     * @warning The overridden callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onInvalidMsgReceived(const CommandRequest&) = 0;

    /**
     * @brief Base command received callback. Subclasses must override this function.
     *
     * @param The CommandRequest object representing the command execution request.
     *
     * @warning This internal callback must be used for log or similar purposes. For specific custom command
     *          functionalities use the internal "onCustomCommandReceived".
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onCommandReceived(const CommandRequest&) = 0;

    /**
     * @brief Base custom command received callback. Subclasses must override this function.
     *
     * @param[in]  The CommandRequest object representing the command execution request.
     * @param[out] The CommandReply object representing the command execution reply.
     *
     * @note This function must process the CommandRequest (function parameter input) and update the CommandReply
     *       (function parameter output), especially the result code.

     * @warning All internal callbacks, including this one, must be non-blocking and have minimal
     *          computation time. Blocking or computationally intensive operations within internal
     *          callbacks can significantly affect the server's performance and responsiveness.
     *          If complex tasks are required, it is recommended to perform them asynchronously
     *          to avoid blocking the server's main thread. Consider using separate threads or
     *          asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onCustomCommandReceived(const CommandRequest&, CommandReply&);

    /**
     * @brief Base server error callback. Subclasses must override this function.
     *
     * @param The `zmq::error_t` object representing the error that occurred.
     *
     * @param Optional additional information or context related to the error. It is an empty string by default.
     *
     * @note The `zmq::error_t` class provides information about ZeroMQ errors. You can access the error code,
     *       description, and other details using the methods provided by `zmq::error_t`.
     *
     * @warning If this function is not overridden in subclasses, it will not handle server errors, and errors may not
     *          be handled properly.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onServerError(const zmq::error_t &error, const std::string& ext_info = "") = 0;

    /**
     * @brief Base sending response callback. Subclasses must override this function.
     *
     * @param The CommandReply object representing the command reply being sent.
     *
     * @warning The overridden callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onSendingResponse(const CommandReply&) = 0;

private:

    // Helper for prepare the ZMQ buffer.
    template<typename... Args>
    static zmq::const_buffer prepareZmqBuffer(const Args&... args)
    {
        utils::BinarySerializer serializer;
        size_t size = serializer.write(std::forward<const Args>(args)...);
        zmq::const_buffer buffer_res(serializer.release(), size);
        return buffer_res;
    }

    // Helper for prepare the result message.
    static void prepareCommandResult(ServerResult, std::unique_ptr<uint8_t>& data_out);

    // Helper for check if the base command is valid.
    static bool validateCommand(int raw_command);

    // Server worker. Will be execute asynchronously.
    void serverWorker();

    // Process command class.
    void processCommand(const CommandRequest&, CommandReply&);

    // Client status checker.
    void checkClientsAliveStatus();

    // Update client last connection.
    void updateClientLastConnection(const std::string& id);

    // Update the server timeout.
    void updateServerTimeout();

    // Internal connect execution process.
    ServerResult execReqConnect(const CommandRequest&);

    // Internal disconnect execution process.
    ServerResult execReqDisconnect(const CommandRequest&);

    // Function for receive data from the client.
    ServerResult recvFromSocket(CommandRequest&);

    // Function for reset the socket.
    void resetSocket();

    // ZMQ socket and context.
    zmq::context_t *context_;        ///< ZMQ context.
    zmq::socket_t* server_socket_;   ///< ZMQ server socket.

    // Endpoint data.
    std::string server_endpoint_;                                     ///< Final server endpoint.
    std::vector<utils::NetworkAdapterInfo> server_listen_adapters_;   ///< Listen server adapters.
    unsigned server_port_;                                            ///< Server port.

    // Mutex.
    mutable std::mutex mtx_;   ///< Safety mutex.

    // Future for the server worker.
    std::future<void> server_worker_future_;

    // Clients container.
    std::map<std::string, HostClient> connected_clients_;   ///< Dictionary with the connected clients.

    // Usefull flags.
    std::atomic_bool flag_server_working_;       ///< Flag for check the server working status.
    std::atomic_bool flag_check_clients_alive_;  ///< Flag that enables and disables the client status checking.
    std::atomic_bool flag_alive_callbacks_;      ///< Flag that enables and disables the callbacks for alive messages.



};

} // END NAMESPACES.
// =====================================================================================================================
