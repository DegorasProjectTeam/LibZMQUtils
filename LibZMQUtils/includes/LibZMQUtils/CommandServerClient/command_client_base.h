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
 * @file command_client_base.h
 * @brief This file contains the declaration of the CommandClientBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2307.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <future>
#include <string>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/libzmqutils_global.h"
#include "LibZMQUtils/CommandServerClient/common.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

// =====================================================================================================================
using common::ServerCommand;
using common::ServerResult;
using common::ClientResult;
using common::CommandReply;
using common::CommandType;
using common::RequestData;
// =====================================================================================================================

class LIBZMQUTILS_EXPORT CommandClientBase
{

public:
    
    CommandClientBase(const std::string& server_endpoint, const std::string& client_name = "");
    
    bool startClient(const std::string& interface_name);

    void stopClient();

    bool resetClient();

    void setAliveCallbacksEnabled(bool);

    void setAutomaticAliveEnabled(bool);

    const common::HostClientInfo& getClientInfo() const;

    const std::string& getServerEndpoint() const;

    const std::string& getClientName() const;

    ClientResult sendCommand(const RequestData&, CommandReply&);

    /**
     * @brief Virtual destructor.
     * This destructor is virtual to ensure proper cleanup when the derived class is destroyed.
     */
    virtual ~CommandClientBase();

protected:

    /**
     * @brief Get the client's information.
     * @note This is an internal protected function that does not lock the safety mutex.
     * @return A constant reference to the client's HostClientInfo.
     */
    const common::HostClientInfo& internalGetClientInfo() const;

    virtual void onClientStart() = 0;

    virtual void onClientStop() = 0;

    virtual void onWaitingReply() = 0;

    virtual void onDeadServer() = 0;

    virtual void onConnected() = 0;

    virtual void onDisconnected() = 0;

    virtual void onInvalidMsgReceived(const CommandReply&) = 0;

    virtual void onReplyReceived(const CommandReply&) = 0;

    virtual void onSendingCommand(const RequestData&) = 0;

    virtual void onClientError(const zmq::error_t&, const std::string& ext_info) = 0;

private:


    ClientResult recvFromSocket(CommandReply&);

    void internalStop();

    void startAutoAlive();

    void stopAutoAlive();

    void sendAliveCallback();
    zmq::multipart_t prepareMessage(const RequestData &msg);

    // Internal client identification.
    common::HostClientInfo client_info_;       ///< External client information for identification.
    std::string client_name_;              ///< Internal client name. Will not be use as id.

    // Server endpoint.
    std::string server_endpoint_;

    // ZMQ context and socket.
    zmq::context_t *context_;
    zmq::socket_t *client_socket_;

    // Mutex.
    mutable std::mutex mtx_;

    std::future<void> auto_alive_future_;
    std::condition_variable auto_alive_cv_;

    // Usefull flags.
    std::atomic_bool flag_client_working_;   ///< Flag for check the client working status.
    std::atomic_bool flag_alive_woking_;     ///< Flag that enables and disables the automatic sending of alive messages.
    std::atomic_bool flag_alive_callbacks_;  ///< Flag that enables and disables the callbacks for alive messages.


};

} // END NAMESPACES.
// =====================================================================================================================
