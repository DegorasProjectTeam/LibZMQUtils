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
 * @file common.h
 * @brief This file contains common elements for the whole library.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2307.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <cstring>
#include <memory>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/libzmqutils_global.h"
#include "LibZMQUtils/Utilities/utils.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace common{
// =====================================================================================================================

// CONSTANTS
// =====================================================================================================================
constexpr int kDefaultClientAliveTimeoutMsec = 8000;   ///< Default timeout for consider a client dead.
constexpr int kDefaultServerAliveTimeoutMsec = 3000;   ///< Default timeout for consider a server dead.
constexpr unsigned kServerReconnTimes = 10;            ///< Server reconnection default number of attempts.
constexpr unsigned kClientAlivePeriodMsec = 1000;       ///< Default period for sending alive commands.
constexpr int kZmqEFSMError = 156384765;               ///< ZMQ EFSM error.
// =====================================================================================================================

// CONVENIENT ALIAS, ENUMERATIONS AND CONSTEXPR
// =====================================================================================================================

using CommandType = std::uint32_t;   ///< Type used for the BaseServerCommand enumeration.
using ResultType = std::uint32_t;    ///< Type used for the BaseServerResult enumeration.

/**
 * @enum BaseServerCommand
 * @brief Enumerates the possible commands of a base command server. They can be extended in a subclass.
 * @warning Commands 0 to 10 ids must not be used for custom commands, they are special and reserved.
 * @warning Only positive commands will be acepted by the server.
 * @warning Messages with the command 0, sentinel value or a reserved commands are considered invalid.
 */
enum class ServerCommand : CommandType
{
    INVALID_COMMAND   = 0,  ///< Invalid command.
    REQ_CONNECT       = 1,  ///< Request to connect to the server.
    REQ_DISCONNECT    = 2,  ///< Request to disconnect from the server.
    REQ_ALIVE         = 3,  ///< Request to check if the server is alive and for notify that the client is alive too.
    RESERVED_COMMANDS = 4,  ///< Sentinel value indicating the start of the reserved commands (not is as a valid msg).
    END_BASE_COMMANDS = 10  ///< Sentinel value indicating the end of the base commands (not is as a valid msg).
};

/**
 * @enum BaseServerCommandResult
 * @brief Enumerates the possible results of a base command operation. They can be extended in a subclass.
 * @warning These results must not be used for custom results, they are special and reserved.
 */
enum class ServerResult : ResultType
{
    COMMAND_OK             = 0,  ///< The command was executed successfully.
    INTERNAL_ZMQ_ERROR     = 1,  ///< An internal ZeroMQ error occurred.
    EMPTY_MSG              = 2,  ///< The message is empty.
    EMPTY_CLIENT_IP        = 3,  ///< The client IP is missing or empty.
    EMPTY_CLIENT_NAME      = 4,  ///< The client name is missing or empty.
    EMPTY_CLIENT_PID       = 5,  ///< The client pid is missing or empty.
    EMPTY_PARAMS           = 6,  ///< The command parameters are missing or empty.
    TIMEOUT_REACHED        = 7,  ///< The operation timed out, the client could be dead.
    INVALID_PARTS          = 8,  ///< The message has invalid parts.
    UNKNOWN_COMMAND        = 9,  ///< The command is not recognized.
    INVALID_MSG            = 10, ///< The message is invalid.
    CLIENT_NOT_CONNECTED   = 11, ///< Not connected to the target.
    ALREADY_CONNECTED      = 12, ///< Already connected to the target.
    BAD_PARAMETERS         = 13, ///< The provided parameters are invalid.
    COMMAND_FAILED         = 14, ///< The command execution failed.
    NOT_IMPLEMENTED        = 15, ///< The command is not implemented.
    BAD_NO_PARAMETERS      = 16, ///< The provided number of parameters are invalid.
    END_BASE_ERRORS        = 20  ///< Sentinel value indicating the end of the base errors (not is a valid error).
};


// TODO MORE CASES RELATED TO THE CLIENT
enum class ClientResult : ResultType
{
    COMMAND_OK = 0,
    INTERNAL_ZMQ_ERROR     = 1,   ///< An internal ZeroMQ error occurred.
    EMPTY_MSG              = 2,   ///< The message is empty.
    EMPTY_PARAMS           = 6,   ///< The result parameters are missing or empty.
    TIMEOUT_REACHED        = 7,   ///< The operation timed out, the server could be dead.
    INVALID_PARTS          = 8,   ///< The command has invalid parts.
    INVALID_MSG            = 10,  ///< The message is invalid.
    CLIENT_STOPPED         = 17,  ///< The client is stopped.

    END_BASE_ERRORS        = 20  ///< Sentinel value indicating the end of the base errors (not is a valid error).

};

// Usefull const expressions.
constexpr int kMinBaseCmdId = static_cast<int>(ServerCommand::INVALID_COMMAND) + 1;
constexpr int kMaxBaseCmdId = static_cast<int>(ServerCommand::END_BASE_COMMANDS) - 1;

static constexpr std::array<const char*, 11>  ServerCommandStr
{
    "INVALID_COMMAND",
    "REQ_CONNECT",
    "REQ_DISCONNECT",
    "REQ_ALIVE",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "END_BASE_COMMANDS"
};

static constexpr std::array<const char*, 21>  ServerResultStr
{
    "COMMAND_OK - Command executed.",
    "INTERNAL_ZMQ_ERROR - Internal ZeroMQ error.",
    "EMPTY_MSG - Message is empty.",
    "EMPTY_CLIENT_IP - Client IP missing or empty.",
    "EMPTY_CLIENT_NAME - Client name missing or empty.",
    "EMPTY_CLIENT_PID - Client pid missing or empty.",
    "EMPTY_PARAMS - Command parameters missing or empty.",
    "TIMEOUT_REACHED - Operation timed out.",
    "INVALID_PARTS - Command has invalid parts.",
    "UNKNOWN_COMMAND - Command is not recognized.",
    "INVALID_COMMAND - Command is invalid.",
    "NOT_CONNECTED - Not connected to the server.",
    "ALREADY_CONNECTED - Already connected to the server.",
    "BAD_PARAMETERS - Provided parameters are invalid.",
    "COMMAND_FAILED - Command execution failed.",
    "NOT_IMPLEMENTED - Command is not implemented.",
    "RESERVED_BASE_ERROR",
    "RESERVED_BASE_ERROR",
    "RESERVED_BASE_ERROR",
    "RESERVED_BASE_ERROR",
    "RESERVED_BASE_ERROR"
};

// =====================================================================================================================

// COMMON STRUCTS
// =====================================================================================================================

struct LIBZMQUTILS_EXPORT HostClient
{
    HostClient() = default;

    HostClient(const HostClient&) = default;

    HostClient(HostClient&&) = default;

    HostClient& operator=(const HostClient&) = default;

    HostClient& operator=(HostClient&&) = default;

    HostClient(const std::string& ip, const std::string& name,
                   const std::string& pid, const std::string& info = "");

    // Struct members.
    std::string id;                          ///< Dinamic host client identification -> [ip//name//pid]
    std::string ip;                          ///< Host client ip.
    std::string hostname;                    ///< Host client name.
    std::string pid;                         ///< PID of the host client process.
    std::string info;                        ///< Host client information.
    utils::SCTimePointStd last_connection;   ///< Host client last connection time.
};

struct CommandRequest
{
    CommandRequest():
        command(ServerCommand::INVALID_COMMAND),
        params(nullptr),
        params_size(0)
    {}

    HostClient client;
    ServerCommand command;
    std::unique_ptr<std::byte> params;
    zmq::multipart_t raw_msg;
    size_t params_size;
};

struct CommandReply
{
    CommandReply():
        params(nullptr),
        params_size(0),
        result(ServerResult::COMMAND_OK)
    {}

    std::unique_ptr<std::byte> params;
    zmq::multipart_t raw_msg;
    size_t params_size;
    ServerResult result;
};

struct LIBZMQUTILS_EXPORT RequestData
{
    RequestData(CommandType id) :
        command(id),
        params(nullptr),
        params_size(0){}

    RequestData() :
        command(static_cast<CommandType>(ServerCommand::INVALID_COMMAND)),
        params(nullptr),
        params_size(0){}

    CommandType command;
    std::unique_ptr<std::byte> params;
    size_t params_size;
};

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
