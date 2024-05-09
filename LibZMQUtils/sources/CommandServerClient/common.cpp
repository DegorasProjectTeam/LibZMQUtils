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
 * @file common.cpp
 * @brief This file contains the implementation common elements for the CommandServerClient module.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <sstream>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/CommandServerClient/common.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace serverclient{
// =====================================================================================================================

ClientInfo::ClientInfo(const utils::UUID& uuid, const std::string& ip, const std::string& pid,
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

size_t ClientInfo::serialize(serializer::BinarySerializer &serializer) const
{
    return serializer.write(this->uuid, this->ip, this->pid, this->hostname, this->name, this->info, this->version);
}

void ClientInfo::deserialize(serializer::BinarySerializer &serializer)
{
    serializer.read(this->uuid, this->ip, this->pid, this->hostname, this->name, this->info, this->version);
}

size_t ClientInfo::serializedSize() const
{
    return Serializable::calcTotalSize(this->uuid, this->ip, this->pid, this->hostname,
                                       this->name, this->info, this->version);
}

std::string ClientInfo::toJsonString() const
{
    std::stringstream ss;

    ss << "{"
       << "\"uuid\":\"" << this->uuid.toRFC4122String() << "\","
       << "\"ip\":\"" << this->ip << "\","
       << "\"pid\":\"" << this->pid << "\","
       << "\"hostname\":\"" << this->hostname << "\","
       << "\"name\":\"" << this->name << "\","
       << "\"info\":\"" << this->info << "\","
       << "\"version\":\"" << this->version << "\""
       << "}";

    return ss.str();
}

RequestData::RequestData(ServerCommand id) :
    command(id),
    params(nullptr),
    params_size(0){}

RequestData::RequestData() :
    command(ServerCommand::INVALID_COMMAND),
    params(nullptr),
    params_size(0){}

CommandRequest::CommandRequest():
    command(ServerCommand::INVALID_COMMAND),
    params(nullptr),
    params_size(0)
{}

CommandReply::CommandReply():
    params(nullptr),
    params_size(0),
    server_result(OperationResult::COMMAND_OK)
{}

ServerInfo::ServerInfo(unsigned int port, const std::string &endpoint, const std::string &hostname,
                       const std::string &name, const std::string &info, const std::string &version,
                       const std::vector<std::string> &ips) :
    port(port),
    endpoint(endpoint),
    hostname(hostname),
    name(name),
    info(info),
    version(version),
    ips(ips)
{}

std::string ServerInfo::toJsonString() const
{
    std::stringstream ss;

    ss << "{"
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

}} // END NAMESPACES.
// =====================================================================================================================
