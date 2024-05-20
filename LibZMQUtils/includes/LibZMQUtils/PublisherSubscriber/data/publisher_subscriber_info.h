/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
 *   A modern open-source C++ library with high-level utilities based on the well-known ZeroMQ open-source universal   *
 *   messaging library. Includes custom command based server-client and publisher-subscriber with automatic binary     *
 *   serialization capabilities, specially designed for system infraestructure. Developed as a free software under the *
 *   context of Degoras Project for the Spanish Navy Observatory SLR station (SFEL) in San Fernando and, of course,    *
 *   for any other station that wants to use it!                                                                       *
 *                                                                                                                     *
 *   Copyright (C) 2024 Degoras Project Team                                                                           *
 *                      < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >                             *
 *                      < Jesús Relinque Madroñal >                                                                    *                                                            *
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
 * @file publisher_subscriber_info.h
 * @brief This file contains the declaration for the PublisherInfo and SubscriberInfo structs.
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

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Utilities/uuid_generator.h"
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

/**
 * @brief Represents information about a publisher.
 *
 */
struct LIBZMQUTILS_EXPORT PublisherInfo
{

    /**
     * @brief PublisherInfo constructor.
     * @param uuid
     * @param endpoint
     * @param name
     */
    PublisherInfo(const utils::UUID& uuid, unsigned port, const std::string& endpoint, const std::string& hostname,
                  const std::string& name, const std::string& info, const std::string& version,
                  const std::vector<std::string>& ips);

    PublisherInfo(const utils::UUID& uuid, unsigned port, const std::string& endpoint);

    PublisherInfo(utils::UUID& uuid, unsigned& port, std::string& endpoint, std::string& hostname, std::string& name,
                  std::string& info, std::string& version, std::vector<std::string>& ips);


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
    std::string toJsonString() const;

    /**
     * @brief Convert PublisherInfo to a formatted string (for debug purposes).
     */
    std::string toString() const;

    // Struct data.
    utils::UUID uuid;              ///< Unique publisher host UUID.
    unsigned port;                 ///< Publisher port.
    std::string endpoint;          ///< Final publisher endpoint.
    std::string hostname;          ///< Host publisher name.
    std::string name;              ///< Publisher name, optional.
    std::string info;              ///< Publisher information, optional.
    std::string version;           ///< Publisher version, optional.
    std::vector<std::string> ips;  ///< Vector of publisher ips.
};

/**
 * @brief The SubscriberInfo struct holds the information of a specific subscriber.
 */
struct LIBZMQUTILS_EXPORT SubscriberInfo
{
    // Default constructor, copy and move
    SubscriberInfo() = default;
    SubscriberInfo(const SubscriberInfo&) = default;
    SubscriberInfo(SubscriberInfo&&) = default;
    SubscriberInfo& operator=(const SubscriberInfo&) = default;
    SubscriberInfo& operator=(SubscriberInfo&&) = default;

    /**
     * @brief Converts subscriber info into a Json string.
     * @return a Json string representing the subscriber info.
     */
    std::string toJsonString() const;

    /**
     * @brief Convert SubscriberInfo to a formatted string (for debug purposes).
     */
    std::string toString() const;

    // Struct data.
    utils::UUID uuid;              ///< Unique Subscriber host UUID.
    std::string hostname;          ///< Host subscriber name.
    std::string name;              ///< Subscriber name, optional.
    std::string info;              ///< Subscriber information, optional.
    std::string version;           ///< Subscriber version, optional.
};

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
