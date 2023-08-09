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
 * @version 2308.2
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
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Global/zmq_context_handler.h"
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

class LIBZMQUTILS_EXPORT CommandClientBase : public ZMQContextHandler
{

public:
    
    CommandClientBase(const std::string& server_endpoint,
                      const std::string& client_name = "",
                      const std::string& interf_name = "");
    
    bool startClient();

    void stopClient();

    bool resetClient();

    void setAliveCallbacksEnabled(bool);

    void setAutomaticAliveEnabled(bool);

    const common::HostClientInfo& getClientInfo() const;

    const std::string& getServerEndpoint() const;

    const std::string& getClientName() const;

    ClientResult doConnect();

    ClientResult doDisconnect();

    ClientResult doAlive();

    ClientResult sendCommand(const RequestData&, CommandReply&);

    bool waitForClose(std::chrono::milliseconds timeout = std::chrono::milliseconds::zero())
    {
        std::unique_lock<std::mutex> lock(client_close_mtx_);
        if (timeout == std::chrono::milliseconds::zero()) {
            client_close_cv_.wait(lock, [this]{ return this->flag_client_closed_.load(); });
            return true;
        } else {
            return client_close_cv_.wait_for(lock, timeout, [this]{ return this->flag_client_closed_.load(); });
        }
    }

    /**
     * @brief Virtual destructor.
     * This destructor is virtual to ensure proper cleanup when the derived class is destroyed.
     */
    virtual ~CommandClientBase() override;

protected:

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

    void deleteSockets()
    {
        // Delete the pointers.
        if(this->client_socket_)
        {
            delete this->client_socket_;
            this->client_socket_ = nullptr;
        }
        if(this->req_close_socket_)
        {
            delete this->req_close_socket_;
            this->req_close_socket_ = nullptr;
        }
        if(this->rep_close_socket_)
        {
            delete this->rep_close_socket_;
            this->rep_close_socket_ = nullptr;
        }
    }

    void internalStopClient();

    bool internalResetClient();

    void startAutoAlive();

    void stopAutoAlive();

    void aliveWorker();

    zmq::multipart_t prepareMessage(const RequestData &msg);

    // Internal client identification.
    common::HostClientInfo client_info_;       ///< External client information for identification.
    std::string client_name_;                  ///< Internal client name. Will not be use as id.

    // Server endpoint.
    std::string server_endpoint_;              ///< Server endpoint.

    // ZMQ sockets.
    zmq::socket_t *client_socket_;      ///< ZMQ client socket.
    zmq::socket_t *rep_close_socket_;   ///< ZMQ auxiliar REP close socket.
    zmq::socket_t *req_close_socket_;   ///< ZMQ auxiliar REQ close socket.

    // Condition variables with associated flags.
    std::condition_variable client_close_cv_;
    std::atomic_bool flag_client_closed_;

    // Mutex.
    mutable std::mutex mtx_;                    ///< Safety mutex.
    mutable std::mutex client_close_mtx_;

    // Auto alive functionality.
    std::future<common::ClientResult> fut_recv_;     ///< Future that stores the client recv status.

    std::future<void> auto_alive_future_;
    std::condition_variable auto_alive_cv_;

    // Usefull flags.
    std::atomic_bool flag_client_working_;  ///< Flag for check the client working status.
    std::atomic_bool flag_autoalive_enabled_;    ///< Flag for enables or disables the automatic sending of alive messages.
    std::atomic_bool flag_alive_callbacks_; ///< Flag for enables or disables the callbacks for alive messages.
};

} // END NAMESPACES.
// =====================================================================================================================
