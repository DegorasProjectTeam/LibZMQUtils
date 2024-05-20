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
 * @file publisher_subscriber_info.cpp
 * @brief This file contains the implementation for PublisherInfo and SubscriberInfo structures.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <sstream>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/PublisherSubscriber/data/publisher_subscriber_info.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

PublisherInfo::PublisherInfo(unsigned port, const utils::UUID& uuid, const std::string& endpoint,
                             const std::string &hostname, const std::string &name, const std::string &info,
                             const std::string &version, const std::vector<std::string> &ips) :
    port(port),
    uuid(uuid),
    endpoint(endpoint),
    hostname(hostname),
    name(name),
    info(info),
    version(version),
    ips(ips)
{}

PublisherInfo::PublisherInfo(unsigned port, const utils::UUID& uuid, const std::string& endpoint) :
    port(port),
    uuid(uuid),
    endpoint(endpoint)
{}

PublisherInfo::PublisherInfo(unsigned& port, utils::UUID& uuid, std::string& endpoint,
                             std::string &hostname, std::string &name, std::string &info,
                             std::string &version, std::vector<std::string>& ips) :
    port(std::move(port)),
    uuid(std::move(uuid)),
    endpoint(std::move(endpoint)),
    hostname(std::move(hostname)),
    name(std::move(name)),
    info(std::move(info)),
    version(std::move(version)),
    ips(std::move(ips))
{}

std::string PublisherInfo::toJsonString() const
{
    std::stringstream ss;

    ss << "{"
       << "\"port\":" << this->port << ","
       << "\"uuid\":" << this->uuid.toRFC4122String() << ","
       << "\"endpoint\":\"" << this->endpoint << "\","
       << "\"hostname\":\"" << this->hostname << "\","
       << "\"name\":\"" << this->name << "\","
       << "\"info\":\"" << this->info << "\","
       << "\"version\":\"" << this->version << "\","
       << "\"ips\":[";

    // Add each IP address in the "ips" vector to the JSON array
    for (size_t i = 0; i < this->ips.size(); ++i)
    {
        ss << "\"" << this->ips[i] << "\"";
        if (i != this->ips.size() - 1)
            ss << ",";
    }
    ss << "]" << "}";
    return ss.str();
}

std::string PublisherInfo::toString() const
{
    // Containers.
    std::stringstream ss;
    std::string ip_list, separator(" - ");

    // Get the IPs.
    for(const auto& ip : this->ips)
    {
        ip_list.append(ip);
        ip_list.append(separator);
    }
    if (!ip_list.empty() && separator.length() > 0)
        ip_list.erase(ip_list.size() - separator.size(), separator.size());

    // Generate the string.
    ss << "Publisher Port:      "        << this->port                   << std::endl;
    ss << "Publisher UUID:      "        << this->uuid.toRFC4122String() << std::endl;
    ss << "Publisher Endpoint:  "        << this->endpoint               << std::endl;
    ss << "Publisher Hostname:  "        << this->hostname               << std::endl;
    ss << "Publisher Name:      "        << this->name                   << std::endl;
    ss << "Publisher Info:      "        << this->info                   << std::endl;
    ss << "Publisher Version:      "     << this->version                << std::endl;
    ss << "Publisher Server Addresses: " << ip_list;

    return ss.str();
}

std::string SubscriberInfo::toJsonString() const
{
    std::stringstream ss;

    ss << "{"
       << "\"uuid\":\"" << this->uuid.toRFC4122String() << "\","
       << "\"name\":\"" << this->name << "\","
       << "\"info\":\"" << this->info << "\","
       << "\"version\":\"" << this->version << "\","
       << "}";

    return ss.str();
}

std::string SubscriberInfo::toString() const
{
    // Containers.
    std::stringstream ss;

    // Generate the string.
    ss << "Subscriber UUID:     " << this->uuid.toRFC4122String() << std::endl;
    ss << "Subscriber Name:     " << this->name                   << std::endl;
    ss << "Subscriber Info:     " << this->info                   << std::endl;
    ss << "Subscriber Version:  " << this->version;

    // Return the string.
    return ss.str();
}

}} // END NAMESPACES.
// =====================================================================================================================
