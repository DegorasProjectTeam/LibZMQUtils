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
 * @file command_server_client_info.h
 * @brief This file contains the declaration for the CommandClientInfo and CommandServerInfo structs.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
#include <cstring>
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

/**
 * @brief Represents information about a host client.
 *
 * The `CommandClientInfo` class stores information about a host client, including a unique client host UUID, IP address,
 * process ID (PID), hostname, and an optional client name. It also includes a timestamp to track the last time the
 * host client was seen, which is typically used by servers to monitor client connections.
 */
struct LIBZMQUTILS_EXPORT CommandClientInfo : public zmqutils::serializer::Serializable
{    
    /**
     * @brief Constructor for CommandClientInfo with specific parameters.
     * @param uuid      Unique client host UUID.
     * @param ip        Host client IP address.
     * @param pid       PID of the host client process.
     * @param hostname  Host client hostname.
     * @param name      Optional client name (default is an empty string).
     * @param info      Detailed client information.
     * @param version   Client version.
     */
    CommandClientInfo(const utils::UUID& uuid, const std::string& ip, const std::string& pid,
                      const std::string& hostname, const std::string& name, const std::string& info,
                      const std::string& version);

    CommandClientInfo(utils::UUID& uuid, std::string& ip, std::string& pid, std::string& hostname,
                      std::string& name, std::string& info, std::string& version);

    // Default constructors and operatos.
    CommandClientInfo() = default;
    CommandClientInfo(const CommandClientInfo&) = default;
    CommandClientInfo(CommandClientInfo&&) = default;
    CommandClientInfo& operator=(const CommandClientInfo&) = default;
    CommandClientInfo& operator=(CommandClientInfo&&) = default;

    size_t serialize(zmqutils::serializer::BinarySerializer& serializer) const final;

    void deserialize(zmqutils::serializer::BinarySerializer& serializer) final;

    size_t serializedSize() const final;

    /**
     * @brief Convert CommandClientInfo to a JSON-formatted string.
     */
    std::string toJsonString() const;

    /**
     * @brief Convert CommandClientInfo to a formatted string (for debug purposes).
     */
    std::string toString() const;

    // Struct data.
    utils::UUID uuid;                  ///< Unique client host UUID.
    std::string ip;                    ///< Host client ip.
    std::string pid;                   ///< PID of the host client process.
    std::string hostname;              ///< Host client name.
    std::string name;                  ///< Client name, optional.
    std::string info;                  ///< Client information, optional.
    std::string version;               ///< Client version, optional.
    utils::HRTimePointStd last_seen;   ///< Time point with the last moment that the client was seen by the server.
    utils::SCTimePointStd last_seen_steady; ///< Auxiliar steady time point to allow calculate time diferences.
};

/**
 * @brief Represents information about a server.
 *
 * This structure encapsulates various server details, such as port, endpoint, hostname, server name, and a collection
 * of associated IP addresses. It also provides a method to convert this information to a JSON-formatted string.
 */
struct LIBZMQUTILS_EXPORT CommandServerInfo
{
    /**
     * @brief Constructor for CommandServerInfo with specific parameters.
     * @param port     Server port.
     * @param endpoint Final server endpoint.
     * @param hostname Host server name.
     * @param name     Server name.
     * @param info     Detailed server information.
     * @param version  Server version.
     * @param ips      Vector of server IP addresses.
     */
    CommandServerInfo(const utils::UUID &uuid, unsigned port, const std::string& endpoint, const std::string& hostname,
                      const std::string& name, const std::string& info, const std::string& version,
                      const std::vector<std::string>& ips);

    /**
     * @brief Constructor for CommandServerInfo with specific parameters.
     * @param port     Server port.
     * @param endpoint Final server endpoint.
     * @param hostname Host server name.
     * @param name     Server name.
     * @param info     Detailed server information.
     * @param version  Server version.
     * @param ips      Vector of server IP addresses.
     */
    CommandServerInfo(utils::UUID &uuid, unsigned& port, std::string& endpoint, std::string& hostname,
                      std::string& name, std::string& info, std::string& version, std::vector<std::string>& ips);

    // Default constructors and operators.
    CommandServerInfo() = default;
    CommandServerInfo(const CommandServerInfo&) = default;
    CommandServerInfo(CommandServerInfo&&) = default;
    CommandServerInfo& operator=(const CommandServerInfo&) = default;
    CommandServerInfo& operator=(CommandServerInfo&&) = default;

    /**
     * @brief Convert CommandServerInfo to a JSON-formatted string.
     */
    std::string toJsonString() const;

    /**
     * @brief Convert CommandServerInfo to a formatted string (for debug purposes).
     */
    std::string toString(bool only_basic_info = false) const;

    // Struct data.
    utils::UUID uuid;                ///< Unique server UUID.
    unsigned port;                   ///< Server port.
    std::string endpoint;            ///< Final server endpoint.
    std::string hostname;            ///< Host server name.
    std::string name;                ///< Server name, optional.
    std::string info;                ///< Server information, optional.
    std::string version;             ///< Server version, optional.
    std::vector<std::string> ips;    ///< Vector of server ips.
    utils::HRTimePointStd last_seen; ///< Time point with the last moment that the server was seen by the client.
};

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
