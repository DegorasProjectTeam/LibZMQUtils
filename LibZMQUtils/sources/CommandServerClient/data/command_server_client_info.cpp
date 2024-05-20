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
 * @file server_client_info.cpp
 * @brief This file contains the implementation for the CommandClientInfo and CommandServerInfo structs.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <sstream>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/CommandServerClient/data/command_server_client_info.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace reqrep{
// =====================================================================================================================

CommandClientInfo::CommandClientInfo(const utils::UUID& uuid, const std::string& ip, const std::string& pid,
                       const std::string& hostname, const std::string& name , const std::string& info,
                       const std::string& version):
    uuid(uuid),
    ip(ip),
    pid(pid),
    hostname(hostname),
    name(name),
    info(info),
    version(version)
{}

CommandClientInfo::CommandClientInfo(utils::UUID& uuid, std::string& ip, std::string& pid,
                                     std::string& hostname, std::string& name , std::string& info,
                                     std::string& version):
    uuid(std::move(uuid)),
    ip(std::move(ip)),
    pid(std::move(pid)),
    hostname(std::move(hostname)),
    name(std::move(name)),
    info(std::move(info)),
    version(std::move(version))
{}

size_t CommandClientInfo::serialize(serializer::BinarySerializer &serializer) const
{
    return serializer.write(this->uuid.getBytes(), this->ip, this->pid, this->hostname,
                            this->name, this->info, this->version);
}

void CommandClientInfo::deserialize(serializer::BinarySerializer &serializer)
{
    std::array<std::byte, 16> uuid_bytes;
    serializer.read(uuid_bytes, this->ip, this->pid, this->hostname, this->name, this->info, this->version);
    this->uuid = utils::UUID(uuid_bytes);
}

size_t CommandClientInfo::serializedSize() const
{
    return Serializable::calcTotalSize(this->uuid.getBytes(), this->ip, this->pid, this->hostname,
                                       this->name, this->info, this->version);
}

std::string CommandClientInfo::toJsonString() const
{
    std::stringstream ss;

    ss << "{"
       << "\"uuid\":\"" << this->uuid.toRFC4122String() << "\","
       << "\"ip\":\"" << this->ip << "\","
       << "\"pid\":\"" << this->pid << "\","
       << "\"hostname\":\"" << this->hostname << "\","
       << "\"name\":\"" << this->name << "\","
       << "\"info\":\"" << this->info << "\","
       << "\"version\":\"" << this->version << "\","
       << "\"last_seen\":\"" << utils::timePointToIso8601(this->last_seen) << "\""
       << "}";

    return ss.str();
}

CommandServerInfo::CommandServerInfo(const utils::UUID &uuid, unsigned int port, const std::string &endpoint, const std::string &hostname,
                                     const std::string &name, const std::string &info, const std::string &version,
                                     const std::vector<std::string> &ips) :
    uuid(uuid),
    port(port),
    endpoint(endpoint),
    hostname(hostname),
    name(name),
    info(info),
    version(version),
    ips(ips)
{}

CommandServerInfo::CommandServerInfo(utils::UUID &uuid, unsigned& port, std::string &endpoint, std::string &hostname,
                                     std::string &name, std::string &info, std::string &version,
                                     std::vector<std::string> &ips) :
    uuid(std::move(uuid)),
    port(std::move(port)),
    endpoint(std::move(endpoint)),
    hostname(std::move(hostname)),
    name(std::move(name)),
    info(std::move(info)),
    version(std::move(version)),
    ips(std::move(ips))
{}

std::string CommandServerInfo::toJsonString() const
{
    std::stringstream ss;

    ss << "{"
       << "\"uuid\":\"" << this->uuid.toRFC4122String() << "\","
       << "\"port\":" << this->port << ","
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

std::string CommandClientInfo::toString() const
{
    // Containers.
    std::stringstream ss;

    // Generate the string.
    ss << "Client UUID:     " << this->uuid.toRFC4122String() << std::endl;
    ss << "Client Ip:       " << this->ip                     << std::endl;
    ss << "Client PID:      " << this->pid                    << std::endl;
    ss << "Client Hostname: " << this->hostname               << std::endl;
    ss << "Client Name:     " << this->name                   << std::endl;
    ss << "Client Info:     " << this->info                   << std::endl;
    ss << "Client Version:  " << this->version                << std::endl;
    ss << "Client Seen:     " << utils::timePointToIso8601(this->last_seen);

    // Return the string.
    return ss.str();
}

std::string CommandServerInfo::toString() const
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
    ss << "Server UUID:      " << this->uuid.toRFC4122String() << std::endl;
    ss << "Server Port:      " << this->port << std::endl;
    ss << "Server Endpoint:  " << this->endpoint << std::endl;
    ss << "Server Hostname:  " << this->hostname << std::endl;
    ss << "Server Name:      " << this->name << std::endl;
    ss << "Server Info:      " << this->info << std::endl;
    ss << "Server Version:   " << this->version << std::endl;
    ss << "Server Addresses: " << ip_list;

    // Return the string.
    return ss.str();
}

}} // END NAMESPACES.
// =====================================================================================================================
