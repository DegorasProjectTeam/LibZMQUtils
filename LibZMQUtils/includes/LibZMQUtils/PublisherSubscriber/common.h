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
#include <cstring>
#include <memory>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Utilities/uuid_generator.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

// CONVENIENT ALIAS, ENUMERATIONS AND CONSTEXPR
// =====================================================================================================================

using ResultType = std::int32_t;    ///< Type used for the BaseServerResult enumeration.
using TopicType = std::string;

/**
 * @enum SubscriberResult
 * @brief Enumerates the possible results of a base message receive operation. They can be extended in a subclass.
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
    INVALID_MSG            = 10, ///< The message is invalid.
    NOT_IMPLEMENTED        = 15, ///< The message process function is not implemented.
    EMPTY_EXT_CALLBACK     = 17, ///< The associated external callback is empty. Used in ClbkCommandServerBase.
    INVALID_EXT_CALLBACK   = 18, ///< The associated external callback is invalid. Used in ClbkCommandServerBase.
    INVALID_PUB_UUID       = 19, ///< The publisher UUID is invalid (could be invalid, missing or empty).
    END_BASE_RESULTS       = 30  ///< Sentinel value indicating the end of the base server results.
};

/**
 * @enum PublisherResult
 * @brief Enumerates the possible results of a base send message operation. They can be extended in a subclass.
 * @warning Results 0 to 30 ids must not be used for custom results, they are special and reserved.
 * @warning Only positive results ids are allowed.
 */
enum class PublisherResult : ResultType
{
    MSG_OK = 0,               ///< The command was executed successfully.
    INTERNAL_ZMQ_ERROR     = 1,   ///< An internal ZeroMQ error occurred.
    EMPTY_MSG              = 2,   ///< The message is empty.
    INVALID_PARTS          = 8,   ///< The command has invalid parts.
    INVALID_MSG            = 10,  ///< The message is invalid.
    PUBLISHER_STOPPED      = 17,  ///< The publisher is stopped.
    END_BASE_RESULTS       = 30   ///< Sentinel value indicating the end of the base publisher results.

};

/**
 * @brief String description of values contained in SubscriberResult enum.
 */
static constexpr std::array<const char*, 31>  SubscriberResultStr
{
    "MSG_OK - Message succesfully received.",
    "INTERNAL_ZMQ_ERROR - Internal ZeroMQ error.",
    "EMPTY_MSG - Message is empty.",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "EMPTY_PARAMS - Command parameters missing or empty.",
    "INVALID_PARTS - Command has invalid parts.",
    "RESERVED_BASE_RESULT",
    "INVALID_MSG - The message is invalid.",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "NOT_IMPLEMENTED - Message process function is not implemented and registered in subscriber.",
    "RESERVED_BASE_RESULT",
    "EMPTY_EXT_CALLBACK - The associated external callback for the message is empty.",
    "INVALID_EXT_CALLBACK - The associated external callback for the message is invalid.",
    "INVALID_PUBLISHER_UUID - The publisher UUID is invalid (could be invalid, missing or empty).",
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

/**
 * @brief String description of values contained in PublisherResult enum.
 */
static constexpr std::array<const char*, 31>  PublisherResultStr
{
    "MSG_OK - Message succesfully received.",
    "INTERNAL_ZMQ_ERROR - Internal ZeroMQ error.",
    "EMPTY_MSG - Message is empty.",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "INVALID_PARTS - Message has invalid parts.",
    "RESERVED_BASE_RESULT",
    "INVALID_MSG - The message is invalid.",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "PUBLISHER_STOPPED - The publisher is stopped.",
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

/**
 * @brief The PublisherInfo struct holds the information of a specific publisher.
 */
struct PublisherInfo
{

    /**
     * @brief PublisherInfo constructor.
     * @param uuid
     * @param endpoint
     * @param name
     */
    LIBZMQUTILS_EXPORT PublisherInfo(utils::UUID uuid, std::string endpoint, std::string name = "");

    // Default constructor, copy and move
    PublisherInfo() = default;
    PublisherInfo(const PublisherInfo&) = default;
    PublisherInfo(PublisherInfo&&) = default;
    PublisherInfo& operator=(const PublisherInfo&) = default;
    PublisherInfo& operator=(PublisherInfo&&) = default;

    /**
     * @brief Converts publisher info into a Json string.
     * @return a Json string representing the publisher info.
     */
    LIBZMQUTILS_EXPORT std::string toJsonString() const;

    // Identifier.
    utils::UUID uuid;                  ///< Unique publisher host UUID.
    // Basic information
    std::string endpoint;              ///< Publisher endpoint.
    std::string name;                  ///< Publisher name, optional.

};

/**
 * @brief The PubSubData struct contains the data of a message exchanged between publisher and subscribers.
 */
struct PubSubData
{
    LIBZMQUTILS_EXPORT PubSubData();

    TopicType topic;
    std::unique_ptr<std::byte[]> data;
    size_t data_size;
};

/**
 * @brief The PubSubMsg struct represents a message exchanged between publisher and subscribers. It includes data and
 * publisher info.
 */
struct PubSubMsg
{

    LIBZMQUTILS_EXPORT PubSubMsg(const PublisherInfo &pub_info);

    PubSubMsg() = default;

    PublisherInfo pub_info;
    PubSubData data;
};


// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
