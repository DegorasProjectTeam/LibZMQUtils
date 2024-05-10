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
 * @file command_client_base.cpp
 * @brief This file contains the implementation of the CommandClientBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <thread>
#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>
// =====================================================================================================================

// ZMQ INCLUDES
// =====================================================================================================================
#include <zmq.hpp>
#include <zmq_addon.hpp>
// =====================================================================================================================
// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/CommandServerClient/command_client_base.h"
#include "LibZMQUtils/InternalHelpers/network_helpers.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
#include "LibZMQUtils/Global/constants.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils {
namespace serverclient {
// =====================================================================================================================

CommandClientBase::CommandClientBase(const std::string& server_endpoint,
                                     const std::string& client_name ,
                                     const std::string& client_version,
                                     const std::string& client_info,
                                     const std::string& net_interface) :
    server_endpoint_(server_endpoint),
    client_socket_(nullptr),
    recv_close_socket_(nullptr),
    req_close_socket_(nullptr),
    flag_client_closed_(true),
    flag_client_working_(false),
    flag_autoalive_enabled_(false),
    flag_alive_callbacks_(false),
    flag_server_connected_(false),
    server_alive_timeout_(kDefaultServerAliveTimeoutMsec),
    send_alive_period_(kDefaultClientSendAlivePeriodMsec)
{    
    // Auxiliar variables and containers.
    std::string ip, hostname, pid;
    internal_helpers::network::NetworkAdapterInfo sel_interf;

    // Generate a unique UUID (v4) for the client.
    utils::UUID uuid = utils::UUIDGenerator::getInstance().generateUUIDv4();

    // Get the client interfaces.
    std::vector<internal_helpers::network::NetworkAdapterInfo> interfcs =
        internal_helpers::network::getHostIPsWithInterfaces();

    // Check if we have active interfaces.
    if(interfcs.empty())
    {
        std::string module = "[LibZMQUtils,CommandServerClient,CommandClientBase] ";
        throw std::invalid_argument(module + "No active network interfaces found.");
    }

    // If no interface name provided, use the first active one.
    if (net_interface.empty())
    {
        // Store the interface.
        sel_interf = interfcs.front();
    }
    else
    {
        // Search the interface we need.
        auto it = std::find_if(interfcs.begin(), interfcs.end(),
                               [&net_interface](const internal_helpers::network::NetworkAdapterInfo& info)
                               {return info.name == net_interface;});

        // Check if the interface exists.
        if (it == interfcs.end())
        {
            std::string module = "[LibZMQUtils,CommandServerClient,CommandClientBase] ";
            throw std::invalid_argument(module + "Network interface not found <" + net_interface + ">.");
        }

        // Store the interface.
        sel_interf = *it;
    }

    // Store the ip, pid and hostname.
    ip = sel_interf.ip;
    hostname = internal_helpers::network::getHostname();
    pid = std::to_string(internal_helpers::network::getCurrentPID());

    // Store all the client info.
    this->client_info_ = ClientInfo(uuid, ip, pid, hostname, client_name, client_version, client_info);
}

CommandClientBase::~CommandClientBase()
{
    // Force the stop client execution.
    // Warning: In this case the onClientStop callback can't be executed.
    this->internalStopClient();
}

const ClientInfo &CommandClientBase::getClientInfo() const
{
    return this->client_info_;
}

bool CommandClientBase::startClient()
{
    // If server is already started, do nothing
    if (this->client_socket_)
        return false;

    // Start the client.
    return this->internalResetClient() ? (this->onClientStart(), true) : false;
}

void CommandClientBase::stopClient()
{    
    // Atomic.
    // If server is already stopped, do nothing.
    if (!this->flag_client_working_)
        return;

    // Safe mutex.
    {
        std::unique_lock<std::mutex> lock(this->client_close_mtx_);

        // Call to the internal stop.
        this->internalStopClient();
    }

    // Call to the internal callback.
    this->onClientStop();
}

bool CommandClientBase::resetClient()
{
    // Call to the internal method.
    return this->internalResetClient();
}

bool CommandClientBase::internalResetClient()
{
    // Lock.
    this->mtx_.lock();

    // Close the previous sockets to flush.
    this->deleteSockets();

    // Create the ZMQ socket.
    try
    {
        // Zmq client socket.
        this->client_socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::req);
        this->client_socket_->connect(this->server_endpoint_);
        this->client_socket_->set(zmq::sockopt::linger, 0);

        // Bind the PUSH close socket to an internal endpoint.
        recv_close_socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::pull);
        recv_close_socket_->bind("inproc://close"+this->client_info_.uuid.toRFC4122String());
        recv_close_socket_->set(zmq::sockopt::linger, 0);

        // Connect the PULL close socket to the same internal endpoint.
        req_close_socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::push);
        req_close_socket_->connect("inproc://close"+this->client_info_.uuid.toRFC4122String());
        req_close_socket_->set(zmq::sockopt::linger, 0);

        // Update the working flag.
        this->flag_client_working_ = true;
        this->flag_client_closed_ = false;
    }
    catch (const zmq::error_t &error)
    {
        // Delete the sockets.
        this->deleteSockets();

        // Update the working flag.
        this->flag_client_working_ = false;
        this->flag_client_closed_ = true;

        // Unlock.
        this->mtx_.unlock();

        // Call to the internal callback.
        this->onClientError(error, "CommandClientBase: Error while creating the client.");
        return false;
    }

    // Unlock.
    this->mtx_.unlock();

    // All ok
    return true;
}

void CommandClientBase::setAliveCallbacksEnabled(bool enable)
{
    this->flag_alive_callbacks_ = enable;
}

void CommandClientBase::setServerAliveTimeout(const std::chrono::milliseconds &timeout)
{
    this->server_alive_timeout_ = static_cast<unsigned>(timeout.count());
}

void CommandClientBase::setSendAlivePeriod(const std::chrono::milliseconds &period)
{
    this->send_alive_period_ = static_cast<unsigned>(period.count());
}

void CommandClientBase::disableAutoAlive()
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    this->stopAutoAlive();
}

const std::string &CommandClientBase::getServerEndpoint() const
{
    // NOTE: Mutex is not neccesary here.
    return this->server_endpoint_;
}

bool CommandClientBase::isWorking() const{return this->flag_client_working_;}

void CommandClientBase::startAutoAlive()
{
    if(!this->flag_autoalive_enabled_)
    {
        this->flag_autoalive_enabled_ = true;
        this->auto_alive_future_ = std::async(std::launch::async, [this]{this->aliveWorker();});
    }
}

void CommandClientBase::stopAutoAlive()
{
    if (this->flag_autoalive_enabled_)
    {
        this->flag_autoalive_enabled_ = false;
        this->auto_alive_cv_.notify_all();
        // If the auto alive is waiting for a response.
        if(this->auto_alive_future_.valid() &&
            this->auto_alive_future_.wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
        {
            // Create a pull socket to send close request
            zmq::socket_t *pull_close_socket = nullptr;
            try
            {
                pull_close_socket = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::rep);
                pull_close_socket->bind("inproc://close" + this->client_info_.uuid.toRFC4122String() + "_autoalive");
                pull_close_socket->set(zmq::sockopt::linger, 0);
                // Send the close msg and wait for the future.
                pull_close_socket->send(zmq::message_t(), zmq::send_flags::none);
                this->auto_alive_future_.wait();
                delete pull_close_socket;

            } catch (...)
            {
                delete pull_close_socket;
            }
        }
    }
}

OperationResult CommandClientBase::sendCommand(const RequestData& request, CommandReply& reply)
{
    // Clean the reply.
    reply = CommandReply();

    // Check if we start the client.
    if (!this->client_socket_)
        return OperationResult::CLIENT_STOPPED;

    // Send the command.
    try
    {
        // Prepare the multipart msg.
        zmq::multipart_t multipart_msg(this->prepareMessage(request));

        // Call to the internal sending command callback.
        if (static_cast<ServerCommand>(request.command) != ServerCommand::REQ_ALIVE || this->flag_alive_callbacks_)
            this->onSendingCommand(request);

        // Send the msg.
        multipart_msg.send(*this->client_socket_);
    }
    catch (const zmq::error_t &error)
    {
        // Call to the error callback and stop the client for safety.
        this->onClientError(error, "CommandClientBase: Error while sending a request. Stopping the client.");
        this->internalStopClient();
        return OperationResult::INTERNAL_ZMQ_ERROR;
    }

    // Now we need to wait the server response.

    // Call to the internal waiting command callback.
    if (static_cast<ServerCommand>(request.command) != ServerCommand::REQ_ALIVE || this->flag_alive_callbacks_)
        this->onWaitingReply();

    // Receive the data.
    this->fut_recv_send_ = std::async(std::launch::async, &CommandClientBase::recvFromSocket, this,
                                      std::ref(reply), this->client_socket_, this->recv_close_socket_);

    // Retrieve the result and reset the future
    while (this->fut_recv_send_.wait_for(10ms) != std::future_status::ready);

    // Use the cv for notify the auto alive worker.
    if (this->flag_autoalive_enabled_)
        this->auto_alive_cv_.notify_one();

    // Check if the client stopped.
    if (reply.server_result == OperationResult::CLIENT_STOPPED ||
        reply.server_result == OperationResult::INTERNAL_ZMQ_ERROR)
    {
        this->flag_autoalive_enabled_ = false;
        return reply.server_result;
    }

    // Check if was a timeout.
    if (reply.server_result == OperationResult::TIMEOUT_REACHED)
    {
        // Call to the internall callback.
        this->onDeadServer(this->connected_server_info_.value());

        // If was  connected, call to the disconnected callback.
        if(this->flag_server_connected_)
        {
            this->onDisconnected(this->connected_server_info_.value());
            this->flag_server_connected_ = false;
        }

        // NOTE: The client reset is neccesary for flush the ZMQ internal
        this->internalResetClient();

        // Delete the server.
        this->connected_server_info_.reset();

        // Return the result.
        return reply.server_result;
    }

    // Check if was ok.
    if(reply.server_result == OperationResult::COMMAND_OK)
    {
        // Internal callback.
        this->onReplyReceived(reply);
    }
    else if(reply.server_result == OperationResult::CLIENT_NOT_CONNECTED && this->flag_server_connected_)
    {
        // In this case the server force the disconnection.
        reply.server_result = OperationResult::DISCONNECTED_FROM_SERVER;
        this->flag_server_connected_ = false;
        // Internal callbacks.
        this->onDisconnected({});
        this->onBadOperation(reply);
    }
    else
    {
        // Internal callback.
        this->onBadOperation(reply);
    }

    // Update the last seen momment.
    std::unique_lock<std::mutex> lock(this->mtx_);
    this->client_info_.last_seen = std::chrono::steady_clock::now();

    // Return the result.
    return reply.server_result;
}

void CommandClientBase::recvFromSocket(CommandReply& reply,
                                       zmq::socket_t* recv_socket,
                                       zmq::socket_t *close_socket)
{
    // Prepare the poller.
    std::vector<zmq::pollitem_t> items = {
          { static_cast<void*>(*recv_socket), 0, ZMQ_POLLIN, 0 },
          { static_cast<void*>(*close_socket), 0, ZMQ_POLLIN, 0 }};

    // Start time for check the timeout.
    auto start = std::chrono::steady_clock::now();

    // Poller loop.
    while(true)
    {
        try
        {
            // Use zmq::poll to set a timeout for receiving a message
            zmq::poll(items.data(), items.size(), std::chrono::milliseconds(10));

            // Check if we must to close.
            if(!this->flag_client_working_ || (items[1].revents & ZMQ_POLLIN))
            {
                reply.server_result = OperationResult::CLIENT_STOPPED;
                return;
            }

            // Calculate elapsed time.
            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            // Check for timeout.
            if (elapsed.count() >= this->server_alive_timeout_)
            {
                reply.server_result = OperationResult::TIMEOUT_REACHED;
                return;
            }

            // We have data.
            if (items[0].revents & ZMQ_POLLIN)
            {
                // Message has been received, try to process it
                zmq::multipart_t multipart_msg;
                multipart_msg.recv(*recv_socket);

                // Check for empty msg or timeout reached.
                if (multipart_msg.empty())
                {
                    reply.server_result = OperationResult::EMPTY_MSG;
                    return;
                }

                // Check the multipart msg size.
                if (multipart_msg.size() != 1 && multipart_msg.size() != 2)
                {
                    reply.server_result = OperationResult::INVALID_PARTS;
                    return;
                }

                // Get the multipart data.
                zmq::message_t msg_res = multipart_msg.pop();

                // Check the result size.
                if (msg_res.size() != sizeof(serializer::SizeUnit) + sizeof(ResultType))
                {
                    reply.server_result = OperationResult::INVALID_MSG;
                    return;
                }

                // Get the command.
                serializer::BinarySerializer::fastDeserialization(msg_res.data(), msg_res.size(), reply.server_result);

                // If there is still one more part, they are the parameters.
                if (multipart_msg.size() == 1)
                {
                    // Get the message and the size.
                    zmq::message_t msg_params = multipart_msg.pop();

                    // Check the parameters.
                    if(msg_params.size() == 0)
                    {
                        reply.server_result = OperationResult::EMPTY_PARAMS;
                        return;
                    }

                    // Get and store the parameters data.
                    serializer::BinarySerializer serializer(msg_params.data(), msg_params.size());
                    reply.params_size = serializer.moveUnique(reply.params);
                }

                // All ok.
                return;
            }
        }
        catch(zmq::error_t& error)
        {
            // Check if we want too close the client.
            // The error code is for ZMQ EFSM error.
            if(error.num() == kZmqEFSMError && !this->flag_client_working_)
            {
                reply.server_result = OperationResult::CLIENT_STOPPED;
                return;
            }

            // Call to the error callback and stop the client for safety.
            this->onClientError(error, "CommandClientBase: Error while receiving a reply. Stopping the client.");
            this->internalStopClient();

            // Store the error result.
            reply.server_result = OperationResult::INTERNAL_ZMQ_ERROR;
            return;
        }
    }
}

void CommandClientBase::deleteSockets()
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
    if(this->recv_close_socket_)
    {
        delete this->recv_close_socket_;
        this->recv_close_socket_ = nullptr;
    }
}

void CommandClientBase::internalStopClient()
{
    // If server is already stopped, do nothing.
    if (!this->flag_client_working_)
        return;

    // Set the shared working flag to false (is atomic).
    this->flag_client_working_ = false;

    // If the client is waiting a response.
    if(this->fut_recv_send_.valid() &&
        this->fut_recv_send_.wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
    {
        // Send the close msg and wait the future.
        this->req_close_socket_->send(zmq::message_t(), zmq::send_flags::none);
        this->fut_recv_send_.wait();
    }

    // If autoalive is enabled stop it.
    if(this->flag_autoalive_enabled_)
        this->stopAutoAlive();

    // Delete the sockets.
    this->deleteSockets();

    // Update the close flag.
    this->flag_client_closed_ = true;

    // Safe sleep.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

void CommandClientBase::aliveWorker()
{
    // Request and reply.
    RequestData request;
    CommandReply reply;

    // Alive socket.
    zmq::socket_t *alive_socket = nullptr;
    zmq::socket_t *pull_close_socket = nullptr;

    // First time flag.
    bool first_time = true;

    // Update the request.
    request.command = ServerCommand::REQ_ALIVE;

    // Create the ZMQ auxiliar alive socket.
    try
    {
        // Create the alive socket.
        alive_socket = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::req);
        alive_socket->connect(this->server_endpoint_);
        alive_socket->set(zmq::sockopt::linger, 0);

        // Connect the pull close socket to the internal endpoint.
        pull_close_socket = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::pull);
        pull_close_socket->connect("inproc://close" + this->client_info_.uuid.toRFC4122String() + "_autoalive");
        pull_close_socket->set(zmq::sockopt::linger, 0);
    }
    catch (const zmq::error_t &error)
    {
        // Safe mutex lock
        std::unique_lock<std::mutex> lock(this->mtx_);

        // Call to the error callback and stop the client for safety.
        this->onClientError(error, "CommandClientBase: Error while creating automatic alive worker. Stopping the client.");
        if(alive_socket)
            delete alive_socket;
        if (pull_close_socket)
            delete pull_close_socket;
        this->flag_autoalive_enabled_ = false;
        this->internalStopClient();
        return;
    }

    // Worker alive loop.
    while(this->flag_autoalive_enabled_)
    {
        // Mutex for cv.
        std::mutex m;
        std::unique_lock<std::mutex> lk(m);

        // Wait the for the timeout or the cv.
        if(!first_time)
        {
            // Avoid unnecesary alive messages.
            auto res = this->auto_alive_cv_.wait_for(lk, std::chrono::milliseconds(this->send_alive_period_));
            if (std::cv_status::timeout != res)
                continue;
        }
        else
            first_time = false;

        // Check if the client is active.
        if(!this->isWorking())
            continue;

        // Send the command.
        try
        {
            // Prepare the multipart msg. The msg id will be the same of the original client socket.
            zmq::multipart_t multipart_msg(this->prepareMessage(request));

            // Call to the internal sending command callback.
            if (this->flag_alive_callbacks_)
                this->onSendingCommand(request);

            // Send the msg.
            multipart_msg.send(*alive_socket);
        }
        catch (const zmq::error_t &error)
        {
            // Call to the error callback and stop the client for safety.
            this->onClientError(error, "CommandClientBase: Error while sending automatic alive. Stopping the client.");

            // If was  connected, call to the disconnected callback.
            if(this->flag_server_connected_)
            {
                this->onDisconnected({});
                this->flag_server_connected_ = false;
            }

            // Safe mutex lock
            std::unique_lock<std::mutex> lock(this->mtx_);
            if(alive_socket)
                delete alive_socket;
            if (pull_close_socket)
                delete pull_close_socket;
            this->flag_autoalive_enabled_ = false;
            this->internalStopClient();
            return;
        }

        // Now we need to wait the server response.

        // Call to the internal waiting command callback.
        if(this->flag_alive_callbacks_)
            this->onWaitingReply();

        // Receive the data.
        this->fut_recv_alive_ = std::async(std::launch::async, &CommandClientBase::recvFromSocket, this,
                                           std::ref(reply), alive_socket, pull_close_socket);


        // Retrieve the result and reset the future
        while (this->fut_recv_alive_.wait_for(10ms) != std::future_status::ready);

        // Check the result.
        if (reply.server_result == OperationResult::CLIENT_STOPPED)
        {
            this->flag_autoalive_enabled_ = false;
        }
        else if(reply.server_result == OperationResult::TIMEOUT_REACHED)
        {
            // Call to the internall callback.
            this->onDeadServer(this->connected_server_info_.value());

            // If was  connected, call to the disconnected callback.
            if(this->flag_server_connected_)
            {
                this->onDisconnected(this->connected_server_info_.value());
                this->flag_server_connected_ = false;
            }

            // Disable autoalive.
            this->flag_autoalive_enabled_ = false;

            // NOTE: The client reset is neccesary for flush the ZMQ internal
            this->internalResetClient();

            // Delete the server.
            this->connected_server_info_.reset();
        }
        else if(reply.server_result == OperationResult::COMMAND_OK)
        {
            // Call to the internal callback.
            if(this->flag_alive_callbacks_)
                this->onReplyReceived(reply);
        }
        else if(reply.server_result != OperationResult::COMMAND_OK &&
                reply.server_result != OperationResult::INTERNAL_ZMQ_ERROR)
        {
            // Internal callback.
            this->onBadOperation(reply);
        }
    }
    
    // Delete sockets.
    if(alive_socket)
        delete alive_socket;

    if (pull_close_socket)
        delete pull_close_socket;
}

OperationResult CommandClientBase::doConnect(bool auto_alive)
{
    // Result.
    OperationResult result;

    // Containers.
    RequestData request;
    CommandReply reply;

    // Serializer.
    serializer::BinarySerializer serializer;

    // Serialize the parameters data.
    serializer.write(this->client_info_);

    // Update the request.
    request.command = ServerCommand::REQ_CONNECT;
    request.params_size = serializer.moveUnique(request.params);

    // Send the command.
    result = this->sendCommand(request, reply);

    // Check the result.
    if(result == OperationResult::COMMAND_OK && reply.server_result == OperationResult::COMMAND_OK && auto_alive)
        this->startAutoAlive();

    // Call to the callback and update flag..
    if(result == OperationResult::COMMAND_OK)
    {
        // Deserialize the server data.
        std::string hostname, name, info, version;
        serializer::BinarySerializer::fastDeserialization(std::move(reply.params), reply.params_size,
            hostname, name, info, version);

        // Get the server port.
        const std::string& endpoint = this->server_endpoint_;
        unsigned port = static_cast<unsigned>(std::stoi(endpoint.substr(endpoint.rfind(':') + 1)));

        // Emplace the server info.
        ServerInfo server_info(port, endpoint, hostname, name, info, version, std::vector<std::string>());
        this->connected_server_info_.emplace(server_info);

        // Update the flag and call to the internal callback.
        this->flag_server_connected_ = true;
        this->onConnected(this->connected_server_info_.value());
    }

    // Return the result.
    return result;
}

OperationResult CommandClientBase::doDisconnect()
{
    // Containers.
    RequestData request;
    CommandReply reply;

    // Stop always the auto alive worker.
    this->stopAutoAlive();

    // Update the request.
    request.command = ServerCommand::REQ_DISCONNECT;

    // Send the command.
    OperationResult res = this->sendCommand(request, reply);

    // Call the callback and update flag.
    if(res == OperationResult::COMMAND_OK && this->flag_server_connected_)
    {
        this->flag_server_connected_ = false;
        this->onDisconnected(this->connected_server_info_.value());

        // Delete the server.
        this->connected_server_info_.reset();
    }

    // Return the result
    return res;
}

OperationResult CommandClientBase::doAlive()
{
    // Containers.
    RequestData request;
    CommandReply reply;

    // Update the request.
    request.command = ServerCommand::REQ_ALIVE;

    // Send the command.
    return this->sendCommand(request, reply);
}

OperationResult CommandClientBase::doGetServerTime(std::string &datetime)
{
    // Containers.
    RequestData request;
    CommandReply reply;
    OperationResult result;

    // Update the request.
    request.command = ServerCommand::REQ_GET_SERVER_TIME;

    // Send the command.
    result = this->sendCommand(request, reply);

    // Check the result.
    if(result != OperationResult::COMMAND_OK)
        return result;

    // Get the ISO 8601 datetime string.
    serializer::BinarySerializer::fastDeserialization(std::move(reply.params), reply.params_size, datetime);

    // Return the result.
    return result;
}

zmq::multipart_t CommandClientBase::prepareMessage(const RequestData &request)
{
    // Serializer.
    serializer::BinarySerializer serializer;

    // Prepare the uuid message.
    size_t uuid_size = serializer.write(this->client_info_.uuid.getBytes());
    zmq::message_t msg_uuid(serializer.release(), uuid_size);

    // Preprare the command message.
    size_t cmd_size = serializer.write(request.command);
    zmq::message_t msg_command(serializer.release(), cmd_size);

    // Prepare the multipart msg.
    zmq::multipart_t multipart_msg;
    multipart_msg.add(std::move(msg_uuid));
    multipart_msg.add(std::move(msg_command));

    // Add command parameters if they exist
    if (request.params_size > 0)
    {
        // Prepare the command parameters
        zmq::message_t message_params(request.params.get(), request.params_size);
        multipart_msg.add(std::move(message_params));
    }

    // Return the multipart msg.
    return multipart_msg;
}

}} // END NAMESPACES.
// =====================================================================================================================
