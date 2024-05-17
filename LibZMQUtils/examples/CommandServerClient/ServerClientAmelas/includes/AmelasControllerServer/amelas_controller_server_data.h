/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *
 *   ExamplesLibZMQUtils related project.                                                                            *
 *                                                                                                        *
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
 * @file common.h
 * @brief EXAMPLE FILE - This file contains common elements for the example AmelasServer module.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/CommandServerClient>
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace communication{
// =====================================================================================================================

// Specific subclass commands (0 to 50 are reserved for the base server).
// WARNING: In our approach, the server commands must be always in order.
enum class AmelasServerCommand : zmqutils::reqrep::CommandType
{
    REQ_SET_HOME_POSITION        = 51,
    REQ_GET_HOME_POSITION        = 52,
    REQ_DO_OPEN_SEARCH_TELESCOPE = 53,
    REQ_DO_EXAMPLE_NOT_IMP       = 54,
    END_AMELAS_COMMANDS          = 80
};

// Extend the base command strings with those of the subclass.
static constexpr std::array<const char*, 5> AmelasServerCommandStr
{
    "REQ_SET_HOME_POSITION",
    "REQ_GET_HOME_POSITION",
    "REQ_DO_OPEN_SEARCH_TELESCOPE",
    "REQ_DO_EXAMPLE_NOT_IMP",
    "END_AMELAS_COMMANDS"
};

// Usefull const expressions.
constexpr std::int32_t kMinCmdId = static_cast<std::int32_t>(zmqutils::reqrep::ServerCommand::END_BASE_COMMANDS) + 1;
constexpr std::int32_t kMaxCmdId = static_cast<std::int32_t>(AmelasServerCommand::END_AMELAS_COMMANDS) - 1;

}} // END NAMESPACES.
// =====================================================================================================================
