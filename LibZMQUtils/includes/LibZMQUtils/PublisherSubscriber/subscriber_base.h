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
 * @file command_server_base.h
 * @brief This file contains the declaration of the CommandServerBase class and related.
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
#include <map>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Global/zmq_context_handler.h"
#include "LibZMQUtils/PublisherSubscriber/common.h"
#include "LibZMQUtils/InternalHelpers/network_helpers.h"
#include "LibZMQUtils/Utilities/uuid_generator.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

// =====================================================================================================================
using common::ServerResultStr;
using common::SubscriberResult;
using internal_helpers::network::NetworkAdapterInfo;
using utils::UUID;
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
 * It is capable of managing multiple client connections, processing command requests, and sending responses. The
 * server also provides optional functionalities such as checking the alive status of connected clients.
 *
 * @section Pattern
 *
 * This class extends the ZeroMQ's standard REQ-REP pattern, also known as the Request-Reply pattern, to allow the
 * client to send commands to the server, which then processes these commands using appropriate callbacks for
 * execution. In a typical REQ-REP pattern, there is a strict send-and-receive order between the client and server. The
 * client (REQ) sends a request to the server and then waits for the reply. Similarly, the server (REP) waits for a
 * request, and once it  receives one, it sends a reply. This cycle then repeats in a strict alternating order,
 * ensuring that each request receives a corresponding reply.
 *
 * This strict request-reply cycle is essential when controlling certain hardware devices or low-level software modules,
 * where the order of commands and their corresponding responses is critical. By ensuring a strict request-reply order,
 * we can maintain consistent control over the devices and modules and reduce the risk of command conflicts or overlaps.
 *
 * In the extended pattern provided by this class, each request from the client is essentially a command that the
 * server must execute. To handle this, we define a set of commands that the client can send, and we provide
 * corresponding callback functions on the server to execute when it receives these commands. The result of the
 * command execution is then sent back to the client as the reply.
 *
 * By extending the pattern in this way, we create a flexible and robust framework for controlling a wide range of
 * hardware devices and software modules, while maintaining the strict request-reply order that ensures reliable and
 * consistent operation.
 *
 * It is important to mention that, for other cases in which the strict request-reply cycle is not necessary, other
 * approaches could be more interesting, such as the use of an infrastructure based on RPC (Remote Procedure Call).
 *
 * @section Case Of Use
 *
 * This communication pattern is particularly beneficial when controlling generic hardware devices like PLC or
 * microcontroller based devices, FPGA devices, generic custom robots etc. Also can be used in specialized devices,
 * like telescope mounts, domes, SLR Range Gate Generators (RGG), etc.
 *
 * In all these examples, the concatenation between replies and responses is crucial. For example, this base server is
 * used in the ROA SLR Station in San Fernando, Spain, for control the RGG, the telescope mount, dome, and other
 * specialized low-level software modules.
 *
 * @section Design
 *
 * This base class is designed to be inherited by subclasses that provide specific implementations for various callback
 * functions to handle server events such as start/stop of the server, client connections/disconnections, receiving
 * invalid or custom commands, and server errors. This design allows for creating specialized servers for different use
 * cases while keeping the core logic generic and reusable.
 *
 * The server operates asynchronously, with main server tasks running in a separate thread. It is capable of managing
 * multiple client connections, processing command requests, and sending responses. The server also provides optional
 * functionalities such as checking the alive status of connected clients.
 *
 * @section Usage
 *
 * To use this class, create a subclass and override the callback functions according to your needs. Also you can define
 * the custom commands and the custom errors related with the sublcass, as well as extend the containers that contains
 * the string representation of the commands and errors.
 *
 * Then, create an instance of your subclass, and use the startServer and stopServer methods to control the server's
 * operation. You can query the server's state and information using the various getters. You can also use
 * setClientStatusCheck to control the checking of clients' alive status.
 *
 * A similar usage pattern applies to the CommandClientBase class, which is meant to interact with a CommandServerBase
 * instance. CommandClientBase is also designed to be subclassed with callback methods to be overridden for specific
 * client behaviors. Therefore, a typical usage scenario involves creating subclassed instances of both classes
 * CommandServerBase and CommandClientBase, where the server handles commands sent by the client.
 *
 * Remember that error handling must be done in the subclass. That is, if an unexpected error occurs on the server, the
 * server will not try to resolve it itself, nor will it try to stop the server. The subclasses are in charge of this
 * management, which can be different depending on each use case.
 *
 * @section Hierarchy
 *
 * The following functions need to be overriden in your subclass:
 *
 * - validateCustomCommand(ServerCommand)
 *
 * The following callbacks need to be overridden in your subclass:
 *
 * - onServerStop()
 * - onServerStart()
 * - onWaitingCommand()
 * - onConnected(const HostClient&)
 * - onDisconnected(const HostClient&)
 * - onDeadClient(const HostClient&)
 * - onInvalidMsgReceived(const CommandRequest&)
 * - onCommandReceived(const CommandRequest&)
 * - onCustomCommandReceived(const CommandRequest&, CommandReply&)
 * - onServerError(const zmq::error_t &error, const std::string& ext_info)
 * - onSendingResponse(const CommandReply&)
 *
 * Among these callbacks, onCustomCommandReceived is crucial because it handles all custom commands. This function
 * receives a command request and a command reply as parameters. It is intended to deserialize the input parameters
 * from the command request, execute the custom command, and then serialize the output parameters into the command
 * reply. Therefore, it is necessary to override this function to add handling for the custom commands specific to
 * your server's application.
 *
 * Finally, also remember add the virtual destructor to the subclass.
 *
 * @note This class is not directly useful on its own. Instead, it is intended to be subclassed and its callback
 * methods overridden to implement the desired server behavior.
 *
 * @warning Client-Specific Data:
 *
 * This server includes client-specific information, including IP address, hostname, Process ID (PID), and
 * client name, in its communications. This approach aids the server in identifying which client issues commands,
 * thereby improving operational visibility and control. Client-specific data can be useful in generating
 * detailed logs which can be analyzed to gain insights into system behavior, user activity, and potential issues.
 *
 * Although in certain contexts it may not be advisable or necessary to include such client information, we recommend
 * retaining this feature for this server class. This enhances the robustness and traceability of server-client
 * interactions, particularly in environments where precise command control and operational accountability are crucial.
 *
 * @warning Security Measures:
 *
 * This server implementation does not provide any built-in security measures, such as authentication or encryption.
 * As a result, the server is potentially vulnerable to unauthorized access or eavesdropping. It is critical to control
 * the server's network connections using external means, like a firewall or VPN. Always ensure that the network
 * environment in which the server operates is secure.
 *
 * @warning Overridden callbacks with computationally intensive operations:
 *
 * When creating a subclass, ensure that blocking or computationally intensive operations are not present within the
 * overridden callbacks. Blocking the server thread can affect the server's performance and responsiveness. If
 * complex tasks are necessary, consider performing them asynchronously or using separate threads.
 *
 * However, it is necessary to remember that this pattern is designed for the control of low-level hardware equipment
 * and similar, so cases like the previous one should not be common.
 *
 * @todo Future Enhancements:
 *
 * Future versions of this server should include built-in security measures. Specifically, we plan to implement
 * support for ZeroMQ's security mechanisms, such as CurveZMQ for public-key encryption and ZAP for authentication.
 * These enhancements will provide a robust layer of security and significantly reduce the risk of unauthorized access.
 *
 * Add a method to easily send large data divided in chunks with progress communication.
 *
 * @see ServerCommand, ServerResult, CommandRequest, CommandReply, CommandClientBase, onCustomCommandReceived
 */
class SubscriberBase : public ZMQContextHandler
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
     * @param server_name Optional parameter to specify the server name. By default is empty.
     *
     * @note The server created with this constructor will be a base server and it doesn't have the complete
     *       implementation of specific request-response logic. It is intended to be subclassed to provide
     *       custom request handling. You can implement the `onCustomCommandReceived` function as an internal
     *       callback in the subclass to handle incoming requests and provide the desired response logic.
     *
     * @warning When specifying the `local_addr`, ensure it is a valid IP address present on the system.
     *          Incorrect or unavailable addresses may result in connection failures.
     */
    LIBZMQUTILS_EXPORT SubscriberBase();


    LIBZMQUTILS_EXPORT const std::string& getTopicFilter() const;


    /**
     * @brief Get the future associated with the server's worker thread.
     *
     * This function returns a const reference to a std::future<void> object representing the asynchronous
     * worker thread that is running the server. The std::future object can be used to check the status of
     * the worker thread or wait for it to complete.
     *
     * @return A const reference to the server's worker thread future.
     */
    LIBZMQUTILS_EXPORT const std::future<void>& getWorkerFuture() const;

    /**
     * @brief Get a const reference to the map of connected clients.
     *
     * This function returns a const reference to a std::map<std::string, HostClient> representing the list of
     * connected clients. Each entry in the map consists of a string key (client identifier) and a `HostClient`
     * object containing information about the connected client.
     *
     * @return A const reference to the map of connected clients.
     */
    LIBZMQUTILS_EXPORT const std::map<UUID, common::PublisherInfo>& getSubscribedPublishers() const;

    /**
     * @brief Check if the server is currently working.
     *
     * This function returns a boolean value indicating whether the server is
     * currently active and working. If the server is working, it means it is
     * processing incoming connections or performing its intended tasks.
     *
     * @return True if the server is working, false otherwise.
     */
    LIBZMQUTILS_EXPORT bool isWorking() const;

    /**
     * @brief Starts the command server.
     *
     * If the server is already running, the function does nothing. Otherwise, it creates the ZMQ
     * context if it doesn't exist and launches the server worker in a separate thread.
     *
     * @return True if the server started, false otherwise.
     */
    LIBZMQUTILS_EXPORT bool startSubscriber();

    /**
     * @brief Stops the command server.
     *
     * If the server is already stopped, the function does nothing. Otherwise
     * deletes the ZMQ context and cleans up the connected clients.
     */
    LIBZMQUTILS_EXPORT void stopSubscriber();

    void subscribe(const std::string &pub_endpoint);

    void unsubscribe(const UUID &pub_uuid);

    void addTopicFilter(const std::string &filter);

    void removeTopicFilter(const std::string &filter);



    /**
     * @brief Virtual destructor.
     *
     * This destructor is virtual to ensure proper cleanup when the derived class is destroyed.
     */
    LIBZMQUTILS_EXPORT virtual ~SubscriberBase() override;

protected:

    // -----------------------------------------------------------------------------------------------------------------
    using ProcessFunction = std::function<void(const common::PubSubMsg&)>; ///< Process function alias.
    using ProcessFunctionsMap = std::unordered_map<std::string, ProcessFunction>;    ///< Process function map alias.
    // -----------------------------------------------------------------------------------------------------------------

    /**
     * @brief Register a function to process `CommandRequest` request from a custom server command.
     *
     * This function allows you to register a function that will process the CommandRequest requests from a custom
     * server command. The process function must take two parameters: a constant reference to a `CommandRequest` object
     * and a reference to a `CommandReply` object.
     *
     * The registered function will be called automatically when a request with a custom command arrives at the server.
     *
     * @tparam ClassT The class type of the object that contains the member function to be called.
     *
     * @param command The custom server command that the function will process replies for.
     * @param obj A pointer to the instance of the object that contains the member function to be called.
     * @param func The member function to call when the server command receives a request.
     *
     * @warning The `func` function must be a member function of the class pointed to by `obj` and take a constant
     *          reference to a `CommandRequest` object and a reference to a `CommandReply` object as parameters.
     */
    template <typename ClassT>
    void registerRequestProcFunc(std::string topic, ClassT* obj,
                                 void(ClassT::*func)(const common::PubSubMsg&))
    {
        this->process_fnc_map_[topic] = [obj, func](const common::PubSubMsg& msg)
        {
            (obj->*func)(msg);
        };
    }

    /**
     * @brief Base server start callback. Subclasses must override this function.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    LIBZMQUTILS_EXPORT virtual void onSubscriberStart() = 0;

    /**
     * @brief Base server stop callback. Subclasses must override this function.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    LIBZMQUTILS_EXPORT virtual void onSubscriberStop() = 0;

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
    LIBZMQUTILS_EXPORT virtual void onWaitingCommand() = 0;

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
    LIBZMQUTILS_EXPORT virtual void onSubscribe(const common::PublisherInfo &pub_info) = 0;

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
    LIBZMQUTILS_EXPORT virtual void onUnsubscribe(const common::PublisherInfo &pub_info) = 0;

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
    LIBZMQUTILS_EXPORT virtual void onInvalidMsgReceived(const common::PubSubMsg&) = 0;

    /**
     * @brief Base command received callback. Subclasses must override this function.
     *
     * @param The CommandRequest object representing the command execution request.
     *
     * @warning This internal callback must be used for log or similar purposes. For specific custom command
     *          functionalities use the internal `onCustomCommandReceived`.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    LIBZMQUTILS_EXPORT virtual void onMsgReceived(const common::PubSubMsg&) = 0;

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
    LIBZMQUTILS_EXPORT virtual void onSubscriberError(const zmq::error_t &error, const std::string& ext_info = "") = 0;

private:

    // Internal helper for stop the server.
    void internalStopSubscriber();

    // Server worker (will be executed asynchronously).
    void serverWorker();

    // Function for receive data from the client.
    SubscriberResult recvFromSocket(common::PubSubMsg&);

    // Function for reset the socket.
    void resetSocket();


    // -----------------------------------------------------

    // ZMQ socket.
    zmq::socket_t* socket_;    ///< ZMQ server socket.
    zmq::socket_t* socket_pub_close_;
    UUID socket_close_uuid_;

    // Subscruiber uuid
    UUID sub_uuid_;

    // Mutex.
    mutable std::mutex mtx_;        ///< Safety mutex.
    mutable std::mutex depl_mtx_;   ///< Server deploy mutex.

    // Future and condition variable for the server worker.
    std::future<void> fut_worker_;     ///< Future that stores the server worker status.
    std::condition_variable cv_server_depl_;  ///< Condition variable to notify the deployment status of the server.

    // Clients container.
    std::map<UUID, common::PublisherInfo> subscribed_publishers_;   ///< Dictionary with the connected clients.

    // Process functions container.
    ProcessFunctionsMap process_fnc_map_;        ///< Container with the internal factory process function.

    // Useful flags.
    std::atomic_bool flag_working_;       ///< Flag for check the server working status.

};

} // END NAMESPACES.
// =====================================================================================================================
