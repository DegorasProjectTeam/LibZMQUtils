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
 * @file debug_clbk_command_server_base.h
 * @brief This file contains the declaration of the DebugClbkCommandServerBase class and related.
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
#include "LibZMQUtils/CommandServerClient/command_server/clbk_command_server_base.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace reqrep{
// =====================================================================================================================

/**
 * @brief The DebugClbkCommandServerBase class implements a ClbkCommandServerBase that includes internal callbacks that
 * prints all the input and output data in each internal callback call to facilitate debugging and development. At any
 * time you can toggle inheritance between DebugClbkCommandServerBase and the original ClbkCommandServerBase one to
 * monitor what is happening on the screen. This class is for support and does not imply that a robust logging system
 * should not be used in the override implementation of the system being developed.
 */
class LIBZMQUTILS_EXPORT DebugClbkCommandServerBase : public ClbkCommandServerBase
{
public:

    DebugClbkCommandServerBase(unsigned port, const std::string& local_addr = "*", const std::string& server_name = "",
                               const std::string& server_version = "", const std::string& server_info = "");

protected:

    // -----------------------------------------------------------------------------------------------------------------
    using CommandServerBase::registerReqProcFunc;
    using ClbkCommandServerBase::registerCallback;
    // -----------------------------------------------------------------------------------------------------------------

    // Internal overrided custom command received callback.
    virtual void onCustomCommandReceived(zmqutils::reqrep::CommandRequest&) override;

    // Internal overrided start callback.
    virtual void onServerStart() override;

    // Internal overrided close callback.
    virtual void onServerStop() override;

    // Internal waiting command callback.
    virtual void onWaitingCommand() override;

    // Internal dead client callback.
    virtual void onDeadClient(const zmqutils::reqrep::CommandClientInfo&) override;

    // Internal overrided connect callback.
    virtual void onConnected(const zmqutils::reqrep::CommandClientInfo&) override;

    // Internal overrided disconnect callback.
    virtual void onDisconnected(const zmqutils::reqrep::CommandClientInfo&) override;

    // Internal overrided command received callback.
    virtual void onCommandReceived(const zmqutils::reqrep::CommandRequest&) override;

    // Internal overrided bad command received callback.
    virtual void onInvalidMsgReceived(const zmqutils::reqrep::CommandRequest&) override;

    // Internal overrided sending response callback.
    virtual void onSendingResponse(const zmqutils::reqrep::CommandReply&) override;

    // Internal overrided server error callback.
    virtual void onServerError(const zmq::error_t&, const std::string& ext_info) override;

private:

    std::string generateStringHeader(const std::string& clbk_name, const std::vector<std::string>& data);
};

}} // END NAMESPACES.
// =====================================================================================================================
