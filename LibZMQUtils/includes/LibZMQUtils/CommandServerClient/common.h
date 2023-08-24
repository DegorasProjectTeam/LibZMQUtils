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
 * @version 2308.2
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
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Utilities/utils.h"
#include "LibZMQUtils/Utilities/uuid_generator.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace common{
// =====================================================================================================================

// CONSTANTS
// =====================================================================================================================
constexpr int kDefaultClientAliveTimeoutMsec = 8000;    ///< Default timeout for consider a client dead.
constexpr int kDefaultServerAliveTimeoutMsec = 5000;    ///< Default timeout for consider a server dead.
constexpr unsigned kServerReconnTimes = 10;             ///< Server reconnection default number of attempts.
constexpr unsigned kClientAlivePeriodMsec = 1000;       ///< Default period for sending alive commands.
constexpr int kZmqEFSMError = 156384765;                ///< ZMQ EFSM error.
// =====================================================================================================================

// CONVENIENT ALIAS, ENUMERATIONS AND CONSTEXPR
// =====================================================================================================================

using CommandType = std::int32_t;   ///< Type used for the BaseServerCommand enumeration.
using ResultType = std::int32_t;    ///< Type used for the BaseServerResult enumeration.

// TODO getServerInfo
// TODO getServerTime
// TODO Control the maximum number of clients.
// TODO Dinamic configuration for the constants.

/**
 * @enum ServerCommand
 * @brief Enumerates the possible commands of a base command server. They can be extended in a subclass.
 * @warning Commands -1 to 30 ids must not be used for custom commands, they are special and reserved.
 * @warning Only positive commands ids will be acepted by the server.
 * @warning Messages with the command 0, sentinel value or a reserved commands are considered invalid.
 */
enum class ServerCommand : CommandType
{
    INVALID_COMMAND   = -1, ///< Invalid command.
    REQ_CONNECT       = 0,  ///< Request to connect to the server.
    REQ_DISCONNECT    = 1,  ///< Request to disconnect from the server.
    REQ_ALIVE         = 2,  ///< Request to check if the server is alive and for notify that the client is alive too.
    RESERVED_COMMANDS = 3,  ///< Sentinel value indicating the start of the reserved commands (not is as a valid msg).
    END_BASE_COMMANDS = 30  ///< Sentinel value indicating the end of the base commands (not is as a valid msg).
};

/**
 * @enum ServerResult
 * @brief Enumerates the possible results of a base command operation. They can be extended in a subclass.
 * @warning Results 0 to 30 ids must not be used for custom results, they are special and reserved.
 * @warning Only positive results ids are allowed.
 */
enum class ServerResult : ResultType
{
    COMMAND_OK             = 0,  ///< The command was executed successfully.
    INTERNAL_ZMQ_ERROR     = 1,  ///< An internal ZeroMQ error occurred.
    EMPTY_MSG              = 2,  ///< The message is empty.
    INVALID_CLIENT_IP      = 3,  ///< The client IP is invalid. TODO
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
    EMPTY_EXT_CALLBACK     = 17, ///< The associated external callback is empty. Used in ClbkCommandServerBase.
    INVALID_EXT_CALLBACK   = 18, ///< The associated external callback is invalid. Used in ClbkCommandServerBase.
    INVALID_CLIENT_UUID    = 19, ///< The client UUID is invalid (could be invalid, missing or empty).
    END_BASE_RESULTS       = 30  ///< Sentinel value indicating the end of the base server results.
};


// TODO MORE CASES RELATED TO THE CLIENT
enum class ClientResult : ResultType
{
    COMMAND_OK = 0,               ///< The command was executed successfully.
    INTERNAL_ZMQ_ERROR     = 1,   ///< An internal ZeroMQ error occurred.
    EMPTY_MSG              = 2,   ///< The message is empty.
    EMPTY_PARAMS           = 6,   ///< The result parameters are missing or empty.
    TIMEOUT_REACHED        = 7,   ///< The operation timed out, the server could be dead.
    INVALID_PARTS          = 8,   ///< The command has invalid parts.
    INVALID_MSG            = 10,  ///< The message is invalid.
    CLIENT_STOPPED         = 17,  ///< The client is stopped.
    END_BASE_RESULTS       = 30   ///< Sentinel value indicating the end of the base client resutls (not is a valid result).

};

// Usefull const expressions.
constexpr int kMinBaseCmdId = static_cast<int>(ServerCommand::INVALID_COMMAND) + 1;
constexpr int kMaxBaseCmdId = static_cast<int>(ServerCommand::END_BASE_COMMANDS) - 1;

static constexpr std::array<const char*, 31>  ServerCommandStr
{
    "REQ_CONNECT",
    "REQ_DISCONNECT",
    "REQ_ALIVE",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "END_BASE_COMMANDS"
};

static constexpr std::array<const char*, 31>  ServerResultStr
{
    "COMMAND_OK - Command executed.",
    "INTERNAL_ZMQ_ERROR - Internal ZeroMQ error.",
    "EMPTY_MSG - Message is empty.",
    "INVALID_CLIENT_IP - Client IP missing or empty.",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "EMPTY_PARAMS - Command parameters missing or empty.",
    "TIMEOUT_REACHED - Operation timed out.",
    "INVALID_PARTS - Command has invalid parts.",
    "UNKNOWN_COMMAND - Command is not recognized.",
    "INVALID_MSG - The message is invalid.",
    "NOT_CONNECTED - Not connected to the server.",
    "ALREADY_CONNECTED - Already connected to the server.",
    "BAD_PARAMETERS - Provided parameters are invalid.",
    "COMMAND_FAILED - Command execution failed.",
    "NOT_IMPLEMENTED - Command is not implemented.",
    "BAD_NO_PARAMETERS - The provided number of parameters are invalid.",
    "EMPTY_EXT_CALLBACK - The associated external callback for the command is empty.",
    "INVALID_EXT_CALLBACK - The associated external callback for the command is invalid.",
    "INVALID_CLIENT_UUID - The client UUID is invalid (could be invalid, missing or empty).",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT"
};

static constexpr std::array<const char*, 31>  ClientResultStr
{
    "COMMAND_OK - Command executed.",
    "INTERNAL_ZMQ_ERROR - Internal ZeroMQ error.",
    "EMPTY_MSG - Message is empty.",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "TIMEOUT_REACHED - Operation timed out.",
    "INVALID_PARTS - Command has invalid parts.",
    "RESERVED_BASE_RESULT",
    "INVALID_MSG - The message is invalid.",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "CLIENT_STOPPED - The client is stopped.",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT"
};

// =====================================================================================================================

// COMMON STRUCTS
// =====================================================================================================================

struct LIBZMQUTILS_EXPORT HostInfo
{
    HostInfo() = default;
    
    HostInfo(const utils::UUID& uuid, const std::string& ip, const std::string& pid,
                   const std::string& hostname, const std::string& name = "");

    std::string toJsonString() const;

    // Identifier.
    utils::UUID uuid;                  ///< Unique client host UUID.
    // Basic information
    std::string ip;                    ///< Host client ip.
    std::string pid;                   ///< PID of the host client process.
    std::string hostname;              ///< Host client name.
    std::string name;                  ///< Client name, optional.
    // Others.
    utils::SCTimePointStd last_seen;   ///< Host client last connection time. Used by servers.
};

struct LIBZMQUTILS_EXPORT CommandRequest
{
    CommandRequest();

    utils::UUID client_uuid;
    ServerCommand command;
    std::unique_ptr<std::byte> params;
    size_t params_size;
};

struct LIBZMQUTILS_EXPORT CommandReply
{
    CommandReply();

    std::unique_ptr<std::byte> params;
    size_t params_size;
    ServerResult result;
};

struct LIBZMQUTILS_EXPORT RequestData
{
    RequestData(ServerCommand id);

    RequestData();

    ServerCommand command;                   ///< Command to be sent.
    std::unique_ptr<std::byte> params;
    size_t params_size;
};

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
