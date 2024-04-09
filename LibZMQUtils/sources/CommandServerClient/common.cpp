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
 * @file common.cpp
 * @brief This file contains common elements for the Command Server Client module.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/CommandServerClient/common.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace serverclient{
// =====================================================================================================================

HostInfo::HostInfo(const utils::UUID& uuid,
                   const std::string& ip,
                   const std::string& pid,
                   const std::string& hostname,
                   const std::string& name ):
    uuid(uuid),
    ip(ip),
    pid(pid),
    hostname(hostname),
    name(name)
{}

std::string HostInfo::toJsonString() const
{
    std::stringstream ss;
    ss << "{\n"
       << "\t\"uuid\": \"" << this->uuid.toRFC4122String() << "\",\n"
       << "\t\"ip\": \"" << this->ip << "\",\n"
       << "\t\"pid\": \"" << this->pid << "\",\n"
       << "\t\"hostname\": " << this->hostname << ",\n"
       << "\t\"name\": \"" << this->name << "\"\n"
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
    result(ServerResult::COMMAND_OK)
{}

}} // END NAMESPACES.
// =====================================================================================================================
