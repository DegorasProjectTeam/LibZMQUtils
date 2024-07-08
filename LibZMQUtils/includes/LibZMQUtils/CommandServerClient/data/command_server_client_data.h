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
 * @file command_server_client_data.h
 * @brief This file contains the definition of common data elements for the CommandServerClient module.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
#include "LibZMQUtils/Utilities/utils.h"
#include "LibZMQUtils/Utilities/uuid_generator.h"
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace reqrep{
// =====================================================================================================================

// SERVER - CLIENT COMMON ALIAS
// =====================================================================================================================
using CommandType = std::int32_t;           ///< Type used for the ServerCommand enumeration.
using ResultType = std::int32_t;            ///< Type used for the OperationResult enumeration.
// =====================================================================================================================

// SERVER - CLIENT COMMON ENUMS AND CONSTEXPR
// =====================================================================================================================

/**
 * @enum ServerCommand
 * @brief Enumerates the possible commands of a base command server. They can be extended in a subclass.
 * @warning Commands -1 to 50 ids must not be used for custom commands, they are special and reserved.
 * @warning Only positive commands ids will be acepted by the server.
 * @warning Messages with the command -1, sentinel value or a reserved commands are considered invalid.
 */
enum class ServerCommand : CommandType
{
    INVALID_COMMAND     = -1, ///< Invalid server command.
    REQ_CONNECT         = 0,  ///< Request to connect to the server.
    REQ_DISCONNECT      = 1,  ///< Request to disconnect from the server.
    REQ_ALIVE           = 2,  ///< Request to check if the server is alive and for notify that the client is alive too.
    REQ_GET_SERVER_TIME = 3,  ///< Request to get the server ISO 8601 UTC datetime (uses the system clock).
    REQ_PING            = 4,  ///< Request to ping server.
    END_IMPL_COMMANDS   = 5,  ///< Sentinel value indicating the end of the base implemented commands (invalid command).
    END_BASE_COMMANDS   = 50  ///< Sentinel value indicating the end of the base commands (invalid command).
};

/**
 * @enum OperationResult
 * @brief Enumerates the possible results of a base command operation. They can be extended in a subclass.
 * @warning Results 0 to 50 ids must not be used for custom results, they are special and reserved.
 * @warning Only positive results ids are allowed.
 */
enum class OperationResult : ResultType
{
    INVALID_RESULT           = -1, ///< Invalid operation result.
    COMMAND_OK               = 0,  ///< The command was executed successfully.
    INTERNAL_ZMQ_ERROR       = 1,  ///< An internal ZeroMQ error occurred.
    EMPTY_MSG                = 2,  ///< The message is empty.
    INVALID_CLIENT_IP        = 3,  ///< The client IP is invalid.
    INVALID_SERVER_UUID      = 4,  ///< The server UUID is invalid (could be invalid, missing or empty).
    DISCONNECTED_FROM_SERVER = 5,  ///< The server forced the client disconnection (for example due to dead condition).
    EMPTY_PARAMS             = 6,  ///< The command parameters are missing or empty.
    TIMEOUT_REACHED          = 7,  ///< The operation timed out, the client could be dead.
    INVALID_PARTS            = 8,  ///< The message has invalid parts.
    UNKNOWN_COMMAND          = 9,  ///< The command is not recognized.
    INVALID_MSG              = 10, ///< The message is invalid.
    CLIENT_NOT_CONNECTED     = 11, ///< Not connected to the target.
    ALREADY_CONNECTED        = 12, ///< Already connected to the target.
    BAD_PARAMETERS           = 13, ///< The provided parameters are invalid (deserialization fail).
    COMMAND_FAILED           = 14, ///< The command execution failed in the server (internal error).
    NOT_IMPLEMENTED          = 15, ///< The command is known but not implemented.
    EMPTY_EXT_CALLBACK       = 16, ///< The associated external callback is empty. Used in ClbkCommandServerBase.
    INVALID_EXT_CALLBACK     = 17, ///< The associated external callback is invalid. Used in ClbkCommandServerBase.
    INVALID_CLIENT_UUID      = 18, ///< The client UUID is invalid (could be invalid, missing or empty).
    CLIENT_STOPPED           = 19, ///< The client is stopped.
    MAX_CLIENTS_REACH        = 20, ///< The server has reached the maximum number of clients allowed.
    COMMAND_NOT_ALLOWED      = 21, ///< The command is not allowed to be executed.                                  TODO
    CLIENT_VERSION_NOT_COMP  = 22, ///< The version of the client is not compatible with the server version.        TODO
    END_BASE_RESULTS         = 50  ///< Sentinel value indicating the end of the base server results.
};

// Usefull const expressions.

/// Minimum valid base enum command identifier (related to ServerCommand enum).
constexpr int kMinBaseCmdId = static_cast<int>(ServerCommand::INVALID_COMMAND) + 1;

/// Maximum valid base enum command identifier (related to ServerCommand enum).
constexpr int kMaxBaseCmdId = static_cast<int>(ServerCommand::END_BASE_COMMANDS) - 1;

/// Maximum number of strings for representing each base enum command identifier (related to ServerCommand enum).
constexpr int kMaxBaseCmdSrings = static_cast<int>(ServerCommand::END_BASE_COMMANDS) + 1;

/// Minimum valid base enum result identifier (related to OperationResult enum).
constexpr int kMinBaseResultId = static_cast<int>(OperationResult::INVALID_RESULT) + 1;

/// Maximum valid base enum result identifier (related to OperationResult enum).
constexpr int kMaxBaseResultId = static_cast<int>(OperationResult::END_BASE_RESULTS) - 1;

/// Maximum number of strings for representing each base enum result identifier (related to OperationResult enum).
constexpr int kMaxBaseResultSrings = static_cast<int>(OperationResult::END_BASE_RESULTS) + 1;

// Lookup arrays for trasnsform the enumerations to string.

/// Lookup array with strings that represents the different ServerCommand enum values.
static constexpr std::array<const char*, kMaxBaseCmdSrings>  ServerCommandStr
{
    "REQ_CONNECT",
    "REQ_DISCONNECT",
    "REQ_ALIVE",
    "REQ_GET_SERVER_TIME",
    "REQ_PING",
    "END_IMPL_COMMANDS",
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

/// Lookup array with strings that represents the different OperationResult enum values.
static constexpr std::array<const char*, kMaxBaseResultSrings>  OperationResultStr
{
    "COMMAND_OK - Command executed.",
    "INTERNAL_ZMQ_ERROR - Internal ZeroMQ error.",
    "EMPTY_MSG - Message is empty.",
    "INVALID_CLIENT_IP - Client IP missing or empty.",
    "INVALID_SERVER_UUID - The server UUID is invalid (could be invalid, missing or empty).",
    "DISCONNECTED_FROM_SERVER - The server forced the client disconnection (for example due to dead condition).",
    "EMPTY_PARAMS - Command parameters missing or empty.",
    "TIMEOUT_REACHED - Operation timed out.",
    "INVALID_PARTS - Command has invalid parts.",
    "UNKNOWN_COMMAND - Command is not recognized.",
    "INVALID_MSG - The message is invalid.",
    "NOT_CONNECTED - Not connected to the server.",
    "ALREADY_CONNECTED - Already connected to the server.",
    "BAD_PARAMETERS - Provided parameters are invalid (deserialization fail).",
    "COMMAND_FAILED - Command execution failed in the server (internal server error).",
    "NOT_IMPLEMENTED - The command is known but not implemented.",
    "EMPTY_EXT_CALLBACK - The associated external callback for the command is empty.",
    "INVALID_EXT_CALLBACK - The associated external callback for the command is invalid.",
    "INVALID_CLIENT_UUID - The client UUID is invalid (could be invalid, missing or empty).",
    "CLIENT_STOPPED - The client is stopped.",
    "MAX_CLIENTS_REACH - The server has reached the maximum number of clients allowed.",
    "COMMAND_NOT_ALLOWED - The command is not allowed to be executed.",
    "CLIENT_VERSION_NOT_COMP - The version of the client is not compatible with the server version.",
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
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "END_BASE_RESULTS"
};

// =====================================================================================================================

// SERVER - CLIENT COMMON DATA STRUCTS
// =====================================================================================================================

struct LIBZMQUTILS_EXPORT RequestData : serializer::BinarySerializedData
{};

struct LIBZMQUTILS_EXPORT ReplyData : serializer::BinarySerializedData
{};

struct LIBZMQUTILS_EXPORT CommandRequest
{
    CommandRequest();

    CommandRequest(ServerCommand command, const utils::UUID& uuid, const std::string& timestamp, RequestData&& data);

    /**
     * @brief Resets the CommandRequest clearing all the contents.
     */
    void clear();

    // Struct data.
    ServerCommand command;    ///< Command to be executed in the server.
    utils::UUID client_uuid;  ///< Client UUID unique identification.
    RequestData data;         ///< Request data with the associated command request parameters.
    std::string timestamp;    ///< ISO8601 timestamp that represents the time when the message was created in client.
    utils::HRTimePointStd tp; ///< Time point that represents the time when the message was created in client.
};

struct LIBZMQUTILS_EXPORT CommandReply
{
    // TODO All the data that needs the real response from the server must be optional.

    CommandReply();

    /**
     * @brief Resets the CommandReply clearing all the contents.
     */
    void clear();

    // Struct data.
    ServerCommand command;    ///< Command whose execution generated this reply data.
    utils::UUID server_uuid;  ///< Server UUID unique identification.
    OperationResult result;   ///< Reply result from the server.
    ReplyData data;           ///< Reply data. Can be empty depending on the result of executing the command.
    std::string timestamp;    ///< ISO8601 timestamp that represents the time when the message was created in server.
    utils::HRTimePointStd tp; ///< Time point that represents the time when the message was created in server.
    utils::MsStd elapsed;     ///< Elapsed time between sending the request and receiving the response.
};

// =====================================================================================================================

// SERVER - CLIENT COMMON HELPER FUNCTIONS
// =====================================================================================================================

// =====================================================================================================================
}} // END NAMESPACE
// =====================================================================================================================
