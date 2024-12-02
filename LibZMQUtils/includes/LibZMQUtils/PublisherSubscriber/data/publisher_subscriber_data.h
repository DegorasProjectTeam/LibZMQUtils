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
 * @file publisher_subscriber_data.h
 * @brief This file contains the definition of common data elements for the PublisherSubscriber module.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
#include "LibZMQUtils/PublisherSubscriber/data/publisher_subscriber_info.h"
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

// PUBLISHER - SUBSCRIBER COMMON ALIAS
// =====================================================================================================================
using ResultType = std::int32_t;    ///< Type used for the BaseServerResult enumeration.
using TopicType = std::string;      ///< Type used for representing the publisher-subscriber topics.
using PriorityType = std::uint8_t;  ///< Type used for the BaseServerResult enumeration.
// =====================================================================================================================

// PUBLISHER - SUBSCRIBER COMMON ENUMS AND CONSTEXPR
// =====================================================================================================================

/**
 * @enum OperationResult
 * @brief Enumerates the possible results of a base publisher subscriber operation. They can be extended in a subclass.
 * @warning Results 0 to 50 ids must not be used for custom results, they are special and reserved.
 * @warning Only positive results ids are allowed.
 */
enum class OperationResult : ResultType
{
    INVALID_RESULT         = -1,  ///< Invalid operation result.
    OPERATION_OK           = 0,   ///< All the operation was ok (enqueue data or receive the data).
    INTERNAL_ZMQ_ERROR     = 1,   ///< An internal ZeroMQ error occurred.
    EMPTY_MSG              = 2,   ///< The message is empty.
    EMPTY_PARAMS           = 6,   ///< The command parameters are missing or empty.
    INVALID_PARTS          = 8,   ///< The message has invalid parts.
    BAD_PARAMETERS         = 13,  ///< The parameters sent are not valid.
    NOT_IMPLEMENTED        = 15,  ///< The message process function is not implemented.
    EMPTY_EXT_CALLBACK     = 16,  ///< The associated external callback is empty.
    INVALID_EXT_CALLBACK   = 17,  ///< The associated external callback is invalid.
    INVALID_PUB_UUID       = 18,  ///< The publisher UUID is invalid (could be invalid, missing or empty).
    PUBLISHER_STOPPED      = 19,  ///< The publisher is stopped.
    END_BASE_RESULTS       = 50   ///< Sentinel value indicating the end of the base server results.
};

enum class MessagePriority : PriorityType
{
    NoPriority       = 0,
    LowPriority      = 1,
    NormalPriority   = 2,
    HighPriority     = 3,
    CriticalPriority = 4
};

/// Minimum valid base enum result identifier (related to OperationResult enum).
constexpr int kMinBaseResultId = static_cast<int>(OperationResult::INVALID_RESULT) + 1;

/// Maximum valid base enum result identifier (related to OperationResult enum).
constexpr int kMaxBaseResultId = static_cast<int>(OperationResult::END_BASE_RESULTS) - 1;

/// Maximum number of strings for representing each base enum result identifier (related to OperationResult enum).
constexpr int kMaxBaseResultSrings = static_cast<int>(OperationResult::END_BASE_RESULTS) + 1;

/// Lookup array with strings that represents the different OperationResult enum values.
static constexpr std::array<const char*, kMaxBaseResultSrings>  OperationResultStr
{
    "OPERATION_OK - All the operation was ok (publish data or receive the data).",
    "INTERNAL_ZMQ_ERROR - Internal ZeroMQ error.",
    "EMPTY_MSG - Message is empty.",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "EMPTY_PARAMS - The data parameters missing or empty.",
    "RESERVED_BASE_RESULT",
    "INVALID_PARTS - The message has invalid parts.",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "RESERVED_BASE_RESULT",
    "BAD PARAMETERS - The parameters received are not valid.",
    "RESERVED_BASE_RESULT",
    "NOT_IMPLEMENTED - Message process function is not implemented and registered in subscriber.",
    "EMPTY_EXT_CALLBACK - The associated external callback for the message is empty.",
    "INVALID_EXT_CALLBACK - The associated external callback for the message is invalid.",
    "INVALID_PUB_UUID - The publisher UUID is invalid (could be invalid, missing or empty).",
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
    "RESERVED_BASE_RESULT"
};

// =====================================================================================================================

// PUBLISHER - SUBSCRIBER COMMON DATA STRUCTS
// =====================================================================================================================

/**
 * @brief The PublishedData struct contains the data of a message exchanged between publisher and subscribers.
 */
struct LIBZMQUTILS_EXPORT PublishedData : serializer::BinarySerializedData
{};

/**
 * @brief The PubSubMsg struct represents a message exchanged between publisher and subscribers. It includes data and
 * publisher info.
 */
struct LIBZMQUTILS_EXPORT PublishedMessage
{
    PublishedMessage();

    PublishedMessage(const TopicType& topic, const utils::UUID& uuid, const std::string& timestamp,
                     PublishedData&& data, MessagePriority priority = MessagePriority::NormalPriority);

    /**
     * @brief Resets the PublishedMessage clearing all the contents.
     */
    void clear();

    // Struct data.
    TopicType topic;             ///< Topic associated to the published message.
    MessagePriority priority;    ///< Priority associated to the published message.
    utils::UUID publisher_uuid;  ///< Publisher UUID unique identification.
    PublishedData data;          ///< Original binary serialized published data.
    std::string timestamp;       ///< ISO8601 string timestamp that represents the time when the message was created.
};

template <typename T>
struct LIBZMQUTILS_EXPORT PublishedMessageDeserialized
{
    PublishedMessageDeserialized()
    {
        this->clear();
    }

    PublishedMessageDeserialized(PublishedMessage& msg) :
        topic(std::move(msg.topic)),
        priority(std::move(msg.priority)),
        publisher_uuid(std::move(msg.publisher_uuid)),
        timestamp(std::move(msg.timestamp))
    {}

    void clear()
    {
        this->publisher_uuid.clear();
        this->topic.clear();
        this->data = T();
        this->timestamp.clear();
        this->priority = MessagePriority::NormalPriority;
    }

    // Struct data
    TopicType topic;             ///< Topic associated to the published message.
    MessagePriority priority;    ///< Priority associated to the published message.
    utils::UUID publisher_uuid;  ///< Publisher UUID unique identification.
    T data;                      ///< Deserialized published data.
    std::string timestamp;       ///< ISO8601 string timestamp that represents the time when the message was created.
};

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
