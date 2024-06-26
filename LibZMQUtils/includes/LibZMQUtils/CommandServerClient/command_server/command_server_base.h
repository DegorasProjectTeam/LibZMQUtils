/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
 *   A modern open-source and cross-platform C++ library with high-level utilities based on the well-known ZeroMQ      *
 *   open-source universal messaging library. Includes a suite of modules that encapsulates the ZMQ communication      *
 *   patterns as well as automatic binary serialization capabilities, specially designed for system infraestructure.   *
 *   The library is suited for the quick and easy integration of new and old systems and can be used in different      *
 *   sectors and disciplines seeking robust messaging and serialization solutions.                                     *
 *                                                                                                                     *
 *   Developed as free software within the context of the Degoras Project for the Satellite Laser Ranging Station      *
 *   (SFEL) at the Spanish Navy Observatory (ROA) in San Fernando, Cádiz. The library is open for use by other SLR     *
 *   stations and organizations, so we warmly encourage you to give it a try and feel free to contact us anytime!      *
 *                                                                                                                     *
 *   Copyright (C) 2024 Degoras Project Team                                                                           *
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
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <future>
#include <string>
#include <map>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Global/zmq_context_handler.h"
#include "LibZMQUtils/InternalHelpers/network_helpers.h"
#include "LibZMQUtils/InternalHelpers/common_aliases_macros.h"
#include "LibZMQUtils/Utilities/uuid_generator.h"
#include "LibZMQUtils/CommandServerClient/data/command_server_client_data.h"
#include "LibZMQUtils/CommandServerClient/data/command_server_client_info.h"
// =====================================================================================================================

// ZMQ NAMESPACES
// =====================================================================================================================
namespace zmq
{
    class socket_t;
    class error_t;
}
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace reqrep{
// =====================================================================================================================

// CONSTANTS
// =====================================================================================================================
constexpr unsigned kDefaultClientAliveTimeoutMsec = 10000;    ///< Default timeout for consider a client dead (msec).
constexpr unsigned kDefaultServerReconnAttempts = 5;          ///< Default server reconnection number of attempts.
constexpr unsigned kDefaultMaxNumberOfClients = 1000;         ///< Default maximum number of connected clients.
// =====================================================================================================================

// MACROS
// =====================================================================================================================

#define M_SERVER_COMMAND_REGISTER_LOOKUP_COMMANDS_STRINGS(ENUM_TYPE, ...) \
static constexpr std::array<const char*, COUNT_ARGS(__VA_ARGS__)> ENUM_TYPE##LookupStr { __VA_ARGS__ }; \
    template<> \
    struct EnumStringLookupTrait<ENUM_TYPE> { \
        static constexpr auto& strings = ENUM_TYPE##LookupStr; \
};

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
 * @see ServerCommand, OperationResult, CommandRequest, CommandReply, CommandClientBase, onCustomCommandReceived
 */
//template<typename ServerCommandEnum = ServerCommand>
class LIBZMQUTILS_EXPORT CommandServerBase : public ZMQContextHandler
{

public:

    /**
     * @brief Base constructor for a ZeroMQ command server.
     *
     * This constructor initializes a ZeroMQ based command server with the specified server_port for listening to
     * incoming requests. Additionally, it allows specifying local addresses on which the server will accept
     * connections. By default, the server will accept connections on all available local addresses.
     *
     * @param server_port    The server_port number on which the server will listen for incoming requests.
     * @param server_iface   The interface on which the server will accept connections. By default, it listens on all
     *                       available interfaces ("*"). It can be identified by IP or name.
     * @param server_name    Optional parameter to specify the server name. By default is empty.
     * @param server_version Optional parameter to specify the server version (like "1.1.1"). By default is empty.
     * @param server_info    Optional parameter to specify the server information. By default is empty.
     *
     * @throws std::invalid_argument If no network interfaces matching the specified IP address are found.
     *
     * @note The server requires at least one valid IP address to function properly. If "ip_address" is set to "*",
     * it will listen on all available local interfaces. Otherwise, the server will only bind to the specified IP
     * address if it matches a valid interface.
     *
     * @note The server created with this constructor will be a base server and it doesn't have the complete
     * implementation of specific request-response logic. It is intended to be subclassed to provide custom request
     * handling. You can implement the `onCustomCommandReceived` function as an internal callback in the subclass to
     * handle incoming requests and provide the desired response logic.
     *
     * @warning When specifying the `ip_address`, ensure it is a valid IP address present on the system. Incorrect or
     * unavailable addresses may result in connection failures.
     */
    CommandServerBase(unsigned server_port,
                      const std::string& server_iface = "*",
                      const std::string& server_name = "",
                      const std::string& server_version = "",
                      const std::string& server_info = "");

    /**
     * @brief Get all the server information.
     * @return A const reference to the CommandServerInfo struct that contains all the server information.
     */
    const CommandServerInfo& getServerInfo() const;

    /**
     * @brief Get the network adapter addresses used by the server.
     *
     * This function returns a const reference to a vector of NetworkAdapterInfo objects. Each `NetworkAdapterInfo`
     * object contains information about a network adapter used by the server for communication.
     *
     * @return A const reference to a vector of NetworkAdapterInfo objects.
     */
    const std::vector<internal_helpers::network::NetworkAdapterInfo>& getServerAddresses() const;

    /**
     * @brief Retrieve a concatenated string of server IP addresses.
     *
     * This function generates a string that contains all server IP addresses from
     * the listen interfaces, separated by the specified separator. The list of IPs
     * is obtained from the server addresses provided by the `getServerAddresses()` function.
     *
     * @param separator A string used to separate each IP address in the resulting concatenated string.
     *
     * @return A string containing all server IPs, separated by the specified separator.
     */
    std::string getServerIpsStr(const std::string& separator) const;

    /**
     * @brief Retrieve a list of server IP addresses.
     *
     * This function gathers all server IP addresses from the listening interfaces
     * returned by the `getServerAddresses` method and stores them in a `std::vector`.
     *
     * @return std::vector<std::string> A vector containing all server IP addresses as strings.
     */
    std::vector<std::string> getServerIps() const;

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
     * connected clients. Each entry in the map consists of a string key (client identifier) and a `HostClient`
     * object containing information about the connected client.
     *
     * @return A const reference to the map of connected clients.
     */
    const std::map<utils::UUID, CommandClientInfo>& getConnectedClients() const;

    /**
     * @brief Check if the server is currently working.
     *
     * This function returns a boolean value indicating whether the server is
     * currently active and working. If the server is working, it means it is
     * processing incoming connections or performing its intended tasks.
     *
     * @return True if the server is working, false otherwise.
     */
    bool isWorking() const;

    /**
     * @brief Sets the client alive timeout.
     *
     * This function sets the timeout value (in milliseconds) for client keep-alive checks. If a client connection
     * remains inactive for longer than this timeout, it will be considered dead. This can be useful for managing idle
     * client connections.
     *
     * @param timeout The timeout value in milliseconds.
     *
     * @note A value of 0 automatically disables the client alive checks, but for enable the alive checking, you must
     * always use the function `setClientStatusCheck`.
     */
    void setClientAliveTimeout(const std::chrono::milliseconds& timeout);

    /**
     * @brief Sets the number of reconnection attempts.
     *
     * This function sets the number of attempts to reconnect when a connection is lost or fails. When a connection
     * attempt fails, the server will retry the connection until the specified number of attempts is reached. A value
     * of 0 means no reconnection attempts will be made.
     *
     * @param attempts The number of reconnection attempts.
     *
     * @note A value of 0 means no reconnection attempts will be made.
     */
    void setReconectionAttempts(unsigned attempts);

    /**
     * @brief Sets the maximum number of allowed client connections.
     *
     * This function sets the maximum number of allowed client connections. This value will only be modified if
     * the server is stopped. A value of 0 means no limits.
     *
     * @param clients Maximum number of allowed client connections at the same time.
     *
     * @note A value of 0 means no limits.
     */
    void setMaxNumberOfClients(unsigned clients);

    /**
     * @brief Enables or disables the client's alive status checking.
     *
     * Enables or disables the checking of the client's alive status. This is a very important
     * functionality in the context of critical systems that often use these types of servers.
     *
     * @param The desired status of the client's alive status checking (true to enable, false to disable).
     *
     * @warning It is strongly recommended to keep this check active, due to the critical nature of the systems that
     * usually use this kind of servers. Disabling the client alive status check could result in unexpected behavior
     * or system instability in case of sudden client disconnections or failures.
     */
    void setClientStatusCheck(bool);

    /**
     * @brief Enables or disables the server callbacks when an alive message is received.
     *
     * This function controls whether the server callbacks are called upon receipt of an alive message from the client.
     * By default, the server callbacks are enabled and will be invoked when an alive message is received.
     *
     * This is especially useful for debugging purposes. When debugging server behavior, the constant invocation of
     * callbacks upon receipt of alive messages can cause clutter in the debug output. Disabling these callbacks can
     * help streamline the debugging process and focus on the critical server functionality.
     *
     * @param [in] enabled Boolean flag that determines whether callbacks are enabled (true) or disabled (false).
     */
    void setAliveCallbacksEnabled(bool);

    /**
     * @brief Starts the command server.
     *
     * If the server is already running, the function does nothing. Otherwise, it creates the ZMQ
     * context if it doesn't exist and launches the server worker in a separate thread.
     *
     * @return True if the server started, false otherwise.
     */
    bool startServer();

    /**
     * @brief Stops the command server.
     *
     * If the server is already stopped, the function does nothing. Otherwise
     * deletes the ZMQ context and cleans up the connected clients.
     */
    void stopServer();

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
    std::string operationResultToString(OperationResult result) const;

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
    std::string operationResultToString(ResultType result) const;

    /**
     * @brief Virtual destructor.
     *
     * This destructor is virtual to ensure proper cleanup when the derived class is destroyed.
     */
    virtual ~CommandServerBase() override;

protected:

    // -----------------------------------------------------------------------------------------------------------------
    /// Alias for a function that allows process a command request and to generate the reply.
    using ProcessFunction = std::function<void(const CommandRequest&, CommandReply&)>;
    ///< Alias for a map that associates commands with process functions.
    using ProcessFunctionsMap = std::unordered_map<ServerCommand, ProcessFunction>;
    // -----------------------------------------------------------------------------------------------------------------

    /**
     * @brief Register a function to process `CommandRequest` request from a custom server command.
     *
     * This function allows you to register a function that will process the CommandRequest requests from a custom
     * server command. The process function must take two parameters: a constant reference to a `CommandRequest`
     * object and a reference to a `CommandReply` object.
     *
     * The registered function will be invoked automatically when a request for the specified command
     * is received by the server.
     *
     * @param command  The custom server command that the function will process replies for.
     * @param obj      A pointer to the instance of the object that contains the member function to be called.
     * @param function The member function to call when the server command receives a request.
     *
     * @warning The `func` function must be a member function of the class pointed to by `obj` and take a constant
     * reference to a `CommandRequest` object and a reference to a `CommandReply` object as parameters.
     */
    template <typename Cmd, typename ClassT>
    void registerReqProcFunc(Cmd command, ClassT* obj, void(ClassT::*func)(const CommandRequest&, CommandReply&))
    {
        this->process_fnc_map_[static_cast<ServerCommand>(command)] =
            [obj, func](const CommandRequest& request, CommandReply& reply)
        {
            (obj->*func)(request, reply);
        };
    }

    /**
     * @brief Register a function to process `CommandRequest` request from a custom server command.
     *
     * This function allows you to register a std::function that will process incoming `CommandRequest`
     * requests associated with a specified server command. The std::function must take two parameters:
     * a constant reference to a `CommandRequest` object and a reference to a `CommandReply` object.
     *
     * The function object is flexible and can encapsulate a lambda, free function, functor, or a bound
     * member function, providing significant flexibility in how the request is processed.
     *
     * The registered function will be invoked automatically when a request for the specified command
     * is received by the server.
     *
     * @param command  The custom server command that the function will process requests for.
     * @param function The function object to call when the server command receives a request. This function
     *                    object must match the signature `void(const CommandRequest&, CommandReply&)`.
     */
    template <typename Cmd>
    void registerReqProcFunc(Cmd command, std::function<void(const CommandRequest&, CommandReply&)> function)
    {
        this->process_fnc_map_[static_cast<ServerCommand>(command)] = function;
    }

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
     * @brief Validates a custom request.
     *
     * This function checks if a custom request is valid. The validation is implementation-specific and must be
     * provided by any class that derives from this one. If the custom request is found to be invalid by this
     * function, the internal callback method `onCustomCommandReceived` will not be invoked.
     *
     * The validation can be very simple, for example, checking if the internal custom request command exists.
     * Usually, this is enough for most scenarios, becaouse the complex checks (for example the parameters, the
     * existence of a callback function for the command, etc) are performed by the base server.
     *
     * @param request The `CommandRequest` request to be validated.
     *
     * @return Returns true if the request is valid; false otherwise.
     *
     * @note If you want handle in method `onCustomCommandReceived` requests that could be valid but the process
     * logic is not yet implemented, this function must return true for these commands.
     */
    virtual  bool validateCustomRequest(const CommandRequest& request) const = 0;

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
    virtual void onConnected(const CommandClientInfo&) = 0;

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
    virtual void onDisconnected(const CommandClientInfo&) = 0;

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
    virtual void onDeadClient(const CommandClientInfo&) = 0;

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
     *          functionalities use the internal `onCustomCommandReceived`.
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
     * @param[in]  The `CommandRequest` object representing the command execution request.
     *
     * @note This method is only called when the received command has been validated as a valid custom command
     *       by the validateCustomRequest method.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          server's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the server's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onCustomCommandReceived(CommandRequest&) = 0;

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

    /// Alias for a function that allows transform a ServerCommand to a string.
    using CommandToStringFunction = std::function<std::string(ServerCommand)>;

    /// Vector of NetworkAdapterInfo structs.
    using NetworkAdapterInfoV = std::vector<internal_helpers::network::NetworkAdapterInfo>;

    /// Helper for check if the base command is valid.
    static bool validateCommand(int raw_command);

    /// Intermal helper to get the server addresses.
    const NetworkAdapterInfoV &internalGetServerAddresses() const;

    /// Internal helper to stop the server.
    void internalStopServer();

    /// Server worker (will be execute asynchronously).
    void serverWorker();

    /// Process base command.
    void processCommand(CommandRequest&, CommandReply&);

    /// Process custom command.
    void processCustomCommand(CommandRequest& request, CommandReply& reply);

    /// Client status checker.
    void checkClientsAliveStatus();

    /// Update client last connection.
    void updateClientLastConnection(const utils::UUID &id);

    /// Update the server timeout.
    void updateServerTimeout();

    /// Function for receive data from the client.
    OperationResult recvFromSocket(CommandRequest&);

    /// Function for reset the socket.
    void resetSocket();

    // INTERNAL COMMANDS.

    /// Internal connect execution process.
    OperationResult execReqConnect(CommandRequest&, CommandReply &reply);

    /// Internal disconnect execution process.
    OperationResult execReqDisconnect(const CommandRequest&);

    /// Internal disconnect execution process.
    OperationResult execReqGetServerTime(CommandReply& reply);

    // -----------------------------------------------------

    // ZMQ data.
    zmq::socket_t* server_socket_;   ///< ZMQ server socket.
    zmq::error_t last_zmq_error_;    ///< Last ZMQ error.

    // Endpoint data and server info.
    NetworkAdapterInfoV server_adapters_;   ///< Listen server adapters.
    CommandServerInfo server_info_;         ///< Server information.

    // Mutex.
    mutable std::mutex mtx_;        ///< Safety mutex.
    mutable std::mutex depl_mtx_;   ///< Server deploy mutex.

    // Future and condition variable for the server worker.
    std::future<void> fut_server_worker_;     ///< Future that stores the server worker status.
    std::condition_variable cv_server_depl_;  ///< Condition variable to notify the deployment status of the server.

    // Clients container.
    std::map<utils::UUID, CommandClientInfo> connected_clients_;   ///< Dictionary with the connected clients.

    // Process functions containers.
    ProcessFunctionsMap process_fnc_map_;        ///< Container with the internal factory process function.

    // To string functions containers.
    CommandToStringFunction command_to_string_function_;  ///< Function to transform ServerCommand into strings.

    // Usefull flags.
    std::atomic_bool flag_server_working_;       ///< Flag for check the server working status.
    std::atomic_bool flag_check_clients_alive_;  ///< Flag that enables and disables the client status checking.
    std::atomic_bool flag_alive_callbacks_;      ///< Flag that enables and disables the callbacks for alive messages.

    // Server confg parameters.
    std::atomic_uint client_alive_timeout_;     ///< Tiemout for consider a client dead (in msec).
    std::atomic_uint server_reconn_attempts_;   ///< Server reconnection number of attempts.
    std::atomic_uint max_connected_clients_;    ///< Maximum number of connected clients.

    /// Specific class scope (for debug purposes).
    inline static const std::string kScope = "[LibZMQUtils,CommandServerClient,CommandServerBase]";
};

}} // END NAMESPACES.
// =====================================================================================================================
