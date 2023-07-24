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
#include <future>
#include <map>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/libzmqutils_global.h"
#include "LibZMQUtils/common.h"
#include "LibZMQUtils/utils.h"
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

using common::BaseServerResultStr;
using common::CommandReply;
using common::CommandRequest;
using common::CmdRequestId;
using common::CmdReplyRes;
using common::BaseServerCommand;
using common::BaseServerResult;
using common::HostClientInfo;
using utils::NetworkAdapterInfo;



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
     * @note The server created with this constructor will be a base server and may not have the complete
     *       implementation of specific request-response logic. It is intended to be subclassed to provide
     *       custom request handling. You can implement the "onRequestReceived" function as an internal callback
     *       in the subclass to handle incoming requests and provide the desired response logic. Also you
     *       can set external callbacks to handle each specific request. Both approach will work,
     *
     * @warning When specifying the `local_addr`, ensure it is a valid IP address or network interface
     *          name present on the system. Incorrect or unavailable addresses may result in connection
     *          failures.
     */
    CommandServerBase(unsigned port, const std::string &local_addr = "*");

    const unsigned& getServerPort() const;

    const std::vector<NetworkAdapterInfo> &getServerAddresses() const;

    const std::string& getServerEndpoint() const;

    const std::future<void>& getServerWorkerFuture() const;

    void startServer();

    void stopServer();

    /**
     * @brief Virtual destructor.
     * This destructor is virtual to ensure proper cleanup when the derived class is destroyed.
     */
    virtual ~CommandServerBase();

protected:

    /**
     * @brief Internal base stop callback.
     * @warning This function is a base callback that does nothing by default. It is implemented to prevent
     *          this class from being pure virtual unnecessarily. Subclasses should override this function.
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onServerStop() = 0;

    /**
     * @brief Internal base start callback.
     * @warning This function is a base callback that does nothing by default. It is implemented to prevent
     *          this class from being pure virtual unnecessarily. Subclasses should override this function.
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onServerStart() = 0;

    /**
     * @brief Internal waiting command callback.
     * @warning This function is a base callback that does nothing by default. It is implemented to prevent
     *          this class from being pure virtual unnecessarily. Subclasses should override this function.
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onWaitingCommand() = 0;

    /**
     * @brief Internal base connect callback.
     * @param The CommandExecReq object representing the command execution request.
     * @warning This function is a base callback that does nothing by default. It is implemented to prevent
     *          this class from being pure virtual unnecessarily. Subclasses should override this function.
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onNewConnection(const CommandRequest&) = 0;

    /**
     * @brief Internal base disconnect callback.
     * @param The CommandExecReq object representing the command execution request.
     * @warning This function is a base callback that does nothing by default. It is implemented to prevent
     *          this class from being pure virtual unnecessarily. Subclasses should override this function.
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onDisconnected(const CommandRequest&) = 0;


    virtual void onDeadClient() = 0;

    virtual void onInvalidMsgReceived(const CommandRequest&) = 0;

    /**
     * @brief Internal base command received callback.
     *
     * @param The CommandExecReq object representing the command execution request.
     *
     * @warning This internal callback must be used for log or similar purposes. For specific custom command
     *          functionalities use the internal onCustomCommandReceived or set an external callback.
     *
     * @warning This function is a base callback that does nothing by default. It is implemented to prevent
      *          this class from being pure virtual unnecessarily. Subclasses should override this function.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onCommandReceived(const CommandRequest&) = 0;

    /**
     * @brief Internal base custom command received callback.
     *
     * This function is a base callback that does nothing by default. It is implemented to prevent
     * this class from being pure virtual unnecessarily. Subclasses should override this function.
     *
     * @warning All internal callbacks, including this one, must be non-blocking and have minimal
     *          computation time. Blocking or computationally intensive operations within internal
     *          callbacks can significantly affect the server's performance and responsiveness.
     *          If complex tasks are required, it is recommended to perform them asynchronously
     *          to avoid blocking the server's main thread. Consider using separate threads or
     *          asynchronous mechanisms to handle time-consuming tasks.
     *
     * @note The `onWaitingCommand` function is intended to be called during the server's main loop
     *       when there are no incoming requests to process. Subclasses may implement this function
     *       to perform periodic checks, cleanup tasks, or other non-blocking activities while waiting
     *       for requests.
     *
     * @see onRequestReceived
     * @see onServerError
     */
    virtual void onCustomCommandReceived(const CommandRequest&, CommandReply&);

    /**
     * @brief Internal base server error callback.
     *
     * @param The `zmq::error_t` object representing the error that occurred.
     *
     * @param Optional additional information or context related to the error.
     *                 It is an empty string by default.
     *
     * @note The `zmq::error_t` class provides information about ZeroMQ errors. You can
     *       access the error code, description, and other details using the methods
     *       provided by `zmq::error_t`.
     *
     * @warning This function is a base callback that does nothing by default. It is implemented to prevent
     *          this class from being pure virtual unnecessarily. Subclasses should override this function.
     *
     * @warning If this function is not overridden in subclasses, it will not handle
     *          server errors, and errors may not be handled properly.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onServerError(const zmq::error_t &error, const std::string& ext_info = "") = 0;

    virtual void onSendingResponse(const CommandReply&) = 0;

private:

    // Helper for prepare the result message.
    static void prepareCommandResult(BaseServerResult, std::unique_ptr<uint8_t>& data_out);

    // Helper for check if the base command is valid.
    static bool validateCommand(int raw_command);

    // Server worker. Will be execute asynchronously.
    void serverWorker();

    // Process command class.
    void processCommand(const CommandRequest&, CommandReply&);

    // Internal connect execution process.
    BaseServerResult execReqConnect(const CommandRequest&);

    // Internal disconnect execution process.
    BaseServerResult execReqDisconnect(const CommandRequest&);

    // Function for receive data from the client.
    BaseServerResult recvFromSocket(CommandRequest&);

    void resetSocket();

    // ZMQ socket and context.
    zmq::context_t *context_;
    zmq::socket_t* main_socket_;

    // Endpoint data.
    std::string server_endpoint_;                                     ///< Final server endpoint.
    std::vector<utils::NetworkAdapterInfo> server_listen_adapters_;   ///< Listen server adapters.
    unsigned server_port_;                                            ///< Server port.

    // Mutex.
    std::mutex mtx_;

    std::future<void> server_worker_future_;
    std::atomic_bool server_working_;
    std::atomic_bool client_present_;
    std::atomic_bool disconnect_requested_;
};

} // END NAMESPACES.
// =====================================================================================================================
