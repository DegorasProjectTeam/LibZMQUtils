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
 * @version 2309.5
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
constexpr int kZmqEFSMError = 156384765;                      ///< ZMQ EFSM error.
// =====================================================================================================================

// CONVENIENT ALIAS, ENUMERATIONS AND CONSTEXPR
// =====================================================================================================================

using ResultType = std::int32_t;    ///< Type used for the BaseServerResult enumeration.

/**
 * @enum ServerResult
 * @brief Enumerates the possible results of a base command operation. They can be extended in a subclass.
 * @warning Results 0 to 30 ids must not be used for custom results, they are special and reserved.
 * @warning Only positive results ids are allowed.
 */
enum class SubscriberResult : ResultType
{
    MSG_OK                 = 0,  ///< The msg received is OK.
    INTERNAL_ZMQ_ERROR     = 1,  ///< An internal ZeroMQ error occurred.
    EMPTY_MSG              = 2,  ///< The message is empty.
    EMPTY_PARAMS           = 6,  ///< The command parameters are missing or empty.
    INVALID_PARTS          = 8,  ///< The message has invalid parts.
    UNKNOWN_COMMAND        = 9,  ///< The command is not recognized.
    INVALID_MSG            = 10, ///< The message is invalid.
    BAD_PARAMETERS         = 13, ///< The provided parameters are invalid.
    BAD_NO_PARAMETERS      = 16, ///< The provided number of parameters are invalid.
    EMPTY_EXT_CALLBACK     = 17, ///< The associated external callback is empty. Used in ClbkCommandServerBase.
    INVALID_EXT_CALLBACK   = 18, ///< The associated external callback is invalid. Used in ClbkCommandServerBase.
    INVALID_PUB_UUID       = 19, ///< The publisher UUID is invalid (could be invalid, missing or empty).
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
    COMMAND_FAILED         = 14,  ///< The command execution failed in the server (internal error).
    CLIENT_STOPPED         = 17,  ///< The client is stopped.
    END_BASE_RESULTS       = 30   ///< Sentinel value indicating the end of the base client results.

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
    "COMMAND_FAILED - Command execution failed in the server (internal server error).",
    "NOT_IMPLEMENTED - Command is not implemented and registered in server.",
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

struct PublisherInfo
{
    LIBZMQUTILS_EXPORT PublisherInfo() = default;
    
    LIBZMQUTILS_EXPORT PublisherInfo(utils::UUID uuid, std::string endpoint, std::string name = "");

    LIBZMQUTILS_EXPORT std::string toJsonString() const;

    // Identifier.
    utils::UUID uuid;                  ///< Unique publisher host UUID.
    // Basic information
    std::string endpoint;              ///< Publisher endpoint.
    std::string name;                  ///< Publisher name, optional.

};

struct PubSubData
{
    LIBZMQUTILS_EXPORT PubSubData();

    std::string topic;
    std::unique_ptr<std::byte[]> data;
    size_t data_size;
};

struct PubSubMsg
{
    LIBZMQUTILS_EXPORT PubSubMsg() = default;
    LIBZMQUTILS_EXPORT PubSubMsg(const PublisherInfo &pub_info);

    PublisherInfo pub_info;
    PubSubData data;
};


// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
