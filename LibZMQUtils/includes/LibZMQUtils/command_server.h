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

using zmqutils::common::CommandReqId;
using zmqutils::common::CommandRepId;


struct HostClient
{
    std::string client_id;                      ///< Dinamic host client identification -> [ip//name//pid]
    std::string client_pid;                     ///< PID of the host client process.
    std::string client_ip;                      ///< Host client ip.
    std::string client_name;                    ///< Host client name.
    std::string client_info;                    ///< Host client information. // TODO
    utils::HRTimePointStd last_connection;      ///< Host client last connection time.
};

struct CommandExecReq
{
    CommandExecReq(CommandReqId id):
        command_id(id),
        params(nullptr),
        params_size(0),
        client_ip(""),
        client_name(""){}

    CommandExecReq():
        command_id(-1),
        params(nullptr),
        params_size(0),
        client_ip(""),
        client_name(""){}

    std::string client_ip;
    std::string client_name;
    CommandReqId command_id;
    std::unique_ptr<std::uint8_t> params;
    size_t params_size;
};



class LIBZMQUTILS_EXPORT CommandServerBase
{

public:

    // These command ids MUST NOT be used for custom commands. They are special and reserved.
    static const CommandReqId kNoCommand;
    static const CommandReqId kConnectCommand;
    static const CommandReqId kDisconnectCommand;
    static const CommandReqId kAliveCommand;


    enum class CommandResult : std::uint32_t
    {
        COMMAND_OK,
        INTERNAL_ZMQ_ERROR,
        EMPTY_MSG,
        EMPTY_CLIENT_IP,
        EMPTY_CLIENT_NAME,
        EMPTY_PARAMS,
        TIMEOUT_REACHED,
        INVALID_PARTS,
        UNKNOWN_COMMAND,
        INVALID_COMMAND,
        NOT_CONNECTED,
        ALREADY_DISCONNECTED,
        ALREADY_CONNECTED,
        BAD_PARAMETERS,
        COMMAND_FAILED,
        NOT_IMPLEMENTED
    };

    // Specific base callbacks.
    using OnDeadClientCallback = std::function<void(const CommandExecReq&)>;    /// Base callback for dead client.
    using OnConnectCallback = std::function<void(const CommandExecReq&)>;     /// Base callback for connection.
    using OnDisconnectCallback = std::function<void(const CommandExecReq&)>;  /// Base callback for disconnection.
    using OnAliveCallback = std::function<void(const CommandExecReq&)>;       /// Base callback for alive message.

    using CommandCallback = std::function<CommandResult(const void*, size_t, void*&, size_t&)>;



    CommandServerBase(const std::string &listen_address, unsigned port);

    const unsigned& getServerPort() const;

    const std::string& getServerAddress() const;

    const std::string& getServerEndpoint() const;

    const std::future<void>& getServerWorkerFuture() const;

    void startServer();

    void stopServer();

    void setCommandCallback(CommandReqId, CommandCallback);

    void setDeadClientCallback(OnDeadClientCallback functor);


    /**
     * @brief Virtual destructor.
     * This destructor is virtual to ensure proper cleanup when the derived class is destroyed.
     */
    virtual ~CommandServerBase();

protected:

    /**
     * @brief Internal base connect callback.
     * @param The CommandExecReq object representing the command execution request.
     * @warning This base callback does nothing. It is implemented to prevent this class
     *          from being pure virtual unnecessarily.
     */
    virtual void onNewConnection(const CommandExecReq&);

    /**
     * @brief Internal base disconnect callback.
     * @param The CommandExecReq object representing the command execution request.
     * @warning This base callback does nothing. It is implemented to prevent this class
     *          from being pure virtual unnecessarily.
     */
    virtual void onDisconnected(const CommandExecReq&);

    /**
     * @brief Internal base command received callback.
     * @param The CommandExecReq object representing the command execution request.
     * @warning This base callback does nothing. It is implemented to prevent this class
     *          from being pure virtual unnecessarily.
     */
    virtual void onCommandReceived(const CommandExecReq&);

    /**
     * @brief Internal base server error callback.
     * @param The CommandExecReq object representing the command execution request.
     * @warning This base callback does nothing. It is implemented to prevent this class
     *          from being pure virtual unnecessarily.
     */
    virtual void onServerError(const zmq::error_t &error, const std::string& ext_info = "");

private:

    // Internal base command execution function.
    CommandRepId execConnect(const CommandExecReq&);
    CommandRepId execDisconnect(const CommandExecReq &cmd_req);


    static void prepareCommandResult(CommandRepId res, std::unique_ptr<uint8_t>& data_out);

    CommandRepId executeCommand(const CommandExecReq& cmd_req, void *&data_out, size_t &out_size_bytes);

    void serverWorker();
    
    CommandResult recvFromSocket(CommandExecReq &msg);

    void resetSocket();

    // ZMQ socket and context.
    zmq::context_t *context_;
    zmq::socket_t* main_socket_;

    // Endpoint data.
    std::string server_endpoint_;   ///< Final server endpoint.
    std::string server_address_;    ///< Server address.
    unsigned server_port_;          ///< Server port.

    std::future<void> server_worker_future_;
    std::atomic_bool server_working_;
    std::atomic_bool client_present_;
    std::atomic_bool disconnect_requested_;

    std::map<CommandReqId, CommandCallback> commands_;

    // Specific base callback containers.
    OnDeadClientCallback dead_client_callback_;
    OnConnectCallback connect_callback_;
    OnDisconnectCallback disconnect_callback_;

};

} // END NAMESPACES.
// =====================================================================================================================
