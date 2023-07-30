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
 * @file command_server_base.cpp
 * @brief This file contains the implementation of the CommandServerBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2307.1
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
#include <stdio.h>
#include <zmq/zmq_addon.hpp>
#include <zmq/zmq.h>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/CommandServerClient/command_server_base.h"
#include "LibZMQUtils/Utilities/utils.h"
#include "LibZMQUtils/Utilities/binary_serializer.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

CommandServerBase::CommandServerBase(unsigned int port, const std::string& local_addr) :
    context_(nullptr),
    server_socket_(nullptr),
    server_endpoint_("tcp://" + local_addr + ":" + std::to_string(port)),
    server_port_(port),
    flag_server_working_(false),
    flag_check_clients_alive_(true),
    flag_alive_callbacks_(true)
{
    // Get the adapters.
    std::vector<utils::NetworkAdapterInfo> interfcs = utils::getHostIPsWithInterfaces();
    // Store the adapters.
    if(local_addr == "*")
        this->server_adapters_ = interfcs;
    else
    {
        for(const auto& intrfc : interfcs)
        {
            if(intrfc.ip == local_addr)
                this->server_adapters_.push_back(intrfc);
        }
    }
}

const std::future<void> &CommandServerBase::getServerWorkerFuture() const {return this->fut_server_worker_;}

const std::map<std::string, HostClientInfo> &CommandServerBase::getConnectedClients() const
{return this->connected_clients_;}

void CommandServerBase::setClientStatusCheck(bool)
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);
    // Disable the client alive checking.
    this->flag_check_clients_alive_ = false;
    if(this->server_socket_)
        this->server_socket_->set(zmq::sockopt::rcvtimeo, -1);
}

void CommandServerBase::setAliveCallbacksEnabled(bool flag){this->flag_alive_callbacks_ = flag;}

const unsigned& CommandServerBase::getServerPort() const {return this->server_port_;}

const std::vector<utils::NetworkAdapterInfo>& CommandServerBase::getServerAddresses() const
{return this->server_adapters_;}

const std::string& CommandServerBase::getServerEndpoint() const {return this->server_endpoint_;}

bool CommandServerBase::startServer()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // If server is already started, do nothing
    if (this->flag_server_working_)
        return true;

    // Create the ZMQ context.
    if (!this->context_)
        this->context_ = new zmq::context_t(1);

    // Launch server worker in other thread.
    this->fut_server_worker_ = std::async(std::launch::async, &CommandServerBase::serverWorker, this);

    // Wait for the server deployment.
    std::unique_lock<std::mutex> depl_lock(this->depl_mtx_);
    this->cv_server_depl_.wait(depl_lock);

    // Return the server status.
    return this->flag_server_working_;
}

void CommandServerBase::stopServer()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // If server is already stopped, do nothing.
    if (!this->flag_server_working_)
        return;

    // Set the shared working flag to false (is atomic).
    this->flag_server_working_ = false;

    // Delete the context.
    if (this->context_)
    {
        delete this->context_;
        this->context_ = nullptr;
    }

    // Safe sleep.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Delete the socket.
    if(!this->server_socket_)
    {
        delete this->server_socket_;
        this->server_socket_ = nullptr;
    }

    // Safe sleep.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Clean the clients.
    this->connected_clients_.clear();
}

CommandServerBase::~CommandServerBase()
{
    // Stop the server (this function also deletes the pointers).
    this->stopServer();
}

ServerResult CommandServerBase::execReqConnect(const CommandRequest& cmd_req)
{
    // Safe mutex lock.
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Check if the client is already connected.
    auto it = this->connected_clients_.find(cmd_req.client.id);
    if(it != this->connected_clients_.end())
        return ServerResult::ALREADY_CONNECTED;

    // Add the new client.
    this->connected_clients_[cmd_req.client.id] = cmd_req.client;

    // Update the timeout of the main socket.
    if(this->flag_check_clients_alive_)
        this->updateServerTimeout();

    // Call to the internal callback.
    this->onConnected(cmd_req.client);

    // All ok.
    return ServerResult::COMMAND_OK;
}

ServerResult CommandServerBase::execReqDisconnect(const CommandRequest& cmd_req)
{
    // Safe mutex lock.
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Get the client.
    auto it = this->connected_clients_.find(cmd_req.client.id);

    // Remove the client from the map of connected clients.
    this->connected_clients_.erase(it);

    // Call to the internal callback.
    this->onDisconnected(cmd_req.client);

    // Update the timeout of the main socket.
    if(this->flag_check_clients_alive_)
        this->updateServerTimeout();

    // All ok.
    return ServerResult::COMMAND_OK;
}

void CommandServerBase::serverWorker()
{
    // Containers.
    ServerResult result;
    CommandRequest request;
    CommandReply reply;

    // Start server socket
    this->resetSocket();

    // Server worker loop.
    // If there is no client connected wait for a client to connect or for an exit message. If there
    // is a client connected set timeout, so if no command comes in time, check the last time connection
    // for each client. The loop can be stopped (in a safe way) if using the stopServer() function.
    while(this->server_socket_ && this->flag_server_working_)
    {
        // Call to the internal waiting command callback (check first the last request).
        if (request.command != ServerCommand::REQ_ALIVE || this->flag_alive_callbacks_)
            this->onWaitingCommand();

        // Clean the containers.
        request = CommandRequest();
        reply = CommandReply();

        // Receive the data.
        result = this->recvFromSocket(request);

        // Check all the clients status.
        if(this->flag_check_clients_alive_)
            this->checkClientsAliveStatus();

        // Process the data.
        if(result == ServerResult::COMMAND_OK && !this->flag_server_working_)
        {
            // In this case, we will close the server. Call to the internal callback.
            this->onServerStop();
        }
        else if(result == ServerResult::TIMEOUT_REACHED)
        {
            // DO NOTHING.
        }
        else if (result != ServerResult::COMMAND_OK)
        {
            // Internal callback.
            this->onInvalidMsgReceived(request);

            // Store the reply result..
            reply.result = result;

            // Send response callback.
            this->onSendingResponse(reply);

            // Prepare the message.
            utils::BinarySerializer serializer;
            size_t size_res = serializer.write(result);
            zmq::const_buffer buffer_res(serializer.release(), size_res);

            // Send the response.
            try
            {
                this->server_socket_->send(buffer_res, zmq::send_flags::none);
            }
            catch (const zmq::error_t &error)
            {
                // Check if we want to close the server.
                // The error code is for ZMQ EFSM error.
                if(!(error.num() == common::kZmqEFSMError && !this->flag_server_working_))
                    this->onServerError(error, "Error while sending a response.");
            }
        }
        else if (result == ServerResult::COMMAND_OK)
        {
            // Reply id buffer.
            //std::unique_ptr<std::uint8_t> rep_id_buff;

            // Execute the command.
            this->processCommand(request, reply);

            // Binary serializer.
            utils::BinarySerializer serializer;

            // Prepare the multipart msg.
            zmq::multipart_t multipart_msg;

            // Prepare the command result.
            size_t size = serializer.write(reply.result);
            multipart_msg.addmem(serializer.release(), size);

            // Specific data.
            if(reply.result == ServerResult::COMMAND_OK && reply.params_size != 0)
            {
                // Prepare the custom response.
                zmq::message_t message_rep_custom(reply.params.get(), reply.params_size);
                multipart_msg.add(std::move(message_rep_custom));
            }

            // Sending callback.
            if (request.command != ServerCommand::REQ_ALIVE || this->flag_alive_callbacks_)
                this->onSendingResponse(reply);

            // Send the message.
            try
            {
                multipart_msg.send(*this->server_socket_);
            }
            catch (const zmq::error_t &error)
            {
                // Check if we want to close the server.
                // The error code is for ZMQ EFSM error.
                if(!(error.num() == common::kZmqEFSMError && !this->flag_server_working_))
                    this->onServerError(error, "Error while sending a response.");
            }
        }
    }

    // Delete pointers for clean finish the worker.
    if (this->server_socket_)
    {
        delete this->server_socket_;
        this->server_socket_ = nullptr;
    }
}

ServerResult CommandServerBase::recvFromSocket(CommandRequest& request)
{
    // Result variable.
    ServerResult result = ServerResult::COMMAND_OK;

    // Containers.
    bool recv_result;
    zmq::multipart_t multipart_msg;

    // Try to receive data. If an execption is thrown, receiving fails and an error code is generated.
    try
    {
        // Wait the command.
        recv_result = multipart_msg.recv(*(this->server_socket_));
    }
    catch(zmq::error_t& error)
    {
        // Check if we want to close the server.
        // The error code is for ZMQ EFSM error.
        if(error.num() == common::kZmqEFSMError && !this->flag_server_working_)
            return ServerResult::COMMAND_OK;

        // Else, call to error callback.
        this->onServerError(error, "Error while receiving a request.");
        return ServerResult::INTERNAL_ZMQ_ERROR;
    }

    // Check for empty msg or timeout reached.
    if (multipart_msg.empty() && !recv_result)
        return ServerResult::TIMEOUT_REACHED;
    else if (multipart_msg.empty())
        return ServerResult::EMPTY_MSG;

    // Check the multipart msg size.
    if (multipart_msg.size() == 4 || multipart_msg.size() == 5)
    {
        // Auxiliar containers.
        std::string ip;
        std::string hostname;
        std::string pid;

        // Get the multipart data.
        zmq::message_t message_ip = multipart_msg.pop();
        zmq::message_t message_hostname = multipart_msg.pop();
        zmq::message_t message_pid = multipart_msg.pop();
        zmq::message_t message_command = multipart_msg.pop();

        // Get the sizes.
        size_t ip_size_bytes = message_ip.size();
        size_t host_size_bytes = message_hostname.size();
        size_t pid_size_bytes = message_pid.size();
        size_t command_size_bytes = message_command.size();

        // First get the ip data.
        if (ip_size_bytes > 0)
            ip = std::string(static_cast<char*>(message_ip.data()), ip_size_bytes);
        else
            return ServerResult::EMPTY_CLIENT_IP;

        // Get the hostname data.
        if (host_size_bytes > 0)
            hostname = std::string(static_cast<char*>(message_hostname.data()), host_size_bytes);
        else
            return ServerResult::EMPTY_CLIENT_NAME;

        // Get the pid data.
        if (host_size_bytes > 0)
            pid = std::string(static_cast<char*>(message_pid.data()), pid_size_bytes);
        else
            return ServerResult::EMPTY_CLIENT_PID;

        // Update the client info.
        request.client = HostClientInfo(ip, hostname, pid);
        request.client.last_connection = std::chrono::steady_clock::now();

        // Update the last connection if the client is connected.
        this->updateClientLastConnection(request.client.id);

        // Get the command.
        if (command_size_bytes == sizeof(ServerCommand))
        {
            // Auxiliar command container.
            std::int32_t raw_command;

            // Deserialize.
            utils::BinarySerializer::fastDeserialization(message_command.data(), sizeof(ServerCommand), raw_command);

            // Validate the command.
            if(CommandServerBase::validateCommand(raw_command))
                request.command = static_cast<ServerCommand>(raw_command);
            else
            {
                request.command = ServerCommand::INVALID_COMMAND;
                return ServerResult::INVALID_MSG;
            }
        }
        else
        {
            request.command = ServerCommand::INVALID_COMMAND;
            return ServerResult::INVALID_MSG;
        }

        // If there is still one more part, they are the parameters.
        if (multipart_msg.size() == 1)
        {
            // Get the message and the size.
            zmq::message_t message_params = multipart_msg.pop();

            // Check the parameters.
            if(message_params.size() > 0)
            {
                // Get and store the parameters data.
                utils::BinarySerializer serializer(message_params.data(), message_params.size());
                request.params = serializer.moveUnique(request.params_size);
            }
            else
                return ServerResult::EMPTY_PARAMS;
        }
    }
    else
        return ServerResult::INVALID_PARTS;

    // Return the result.
    return result;
}

bool CommandServerBase::validateCommand(int raw_command)
{
    // Auxiliar variables.
    bool result = false;
    int reserved_cmd = static_cast<int>(common::ServerCommand::RESERVED_COMMANDS);
    int end_base_cmd = static_cast<int>(common::ServerCommand::END_BASE_COMMANDS);
    // Check if the command is valid.
    if (raw_command >= common::kMinBaseCmdId && raw_command < reserved_cmd)
        result = true;
    else if(raw_command > end_base_cmd)
        result = true;
    return result;
}

void CommandServerBase::processCommand(const CommandRequest& request, CommandReply& reply)
{
    // First of all, call to the internal callback.
    this->onCommandReceived(request);

    // Process the different commands.
    // 1 - Process is the connect request.
    // 2 - If the command is other, check if the client is connected to the server.
    // 3 - If it is, check if the command is valid.
    // 4 - If valid, process the rest of the base commands or the custom command.
    if (ServerCommand::REQ_CONNECT == request.command)
    {
        reply.result = this->execReqConnect(request);
    }
    else if(this->connected_clients_.find(request.client.id) == this->connected_clients_.end())
    {
        reply.result = ServerResult::CLIENT_NOT_CONNECTED;
    }
    else if (ServerCommand::REQ_DISCONNECT == request.command)
    {
        reply.result = this->execReqDisconnect(request);
    }
    else if (ServerCommand::REQ_ALIVE == request.command)
    {
        reply.result = ServerResult::COMMAND_OK;
    }
    else
    {
        // Custom command, so call the internal callback.
        this->onCustomCommandReceived(request, reply);

        // Chek for an invalid msg.
        if(reply.result == ServerResult::INVALID_MSG)
            this->onInvalidMsgReceived(request);
    }
}

void CommandServerBase::checkClientsAliveStatus()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Auxiliar containers.
    std::vector<std::string> dead_clients;
    std::chrono::milliseconds timeout(common::kDefaultClientAliveTimeoutMsec);
    std::chrono::milliseconds min_remaining_time = timeout;

    // Get the current time.
    utils::SCTimePointStd now = std::chrono::steady_clock::now();

    // Check each connection.
    for(auto& client : this->connected_clients_)
    {
        // Get the last connection time.
        const auto& last_conn = client.second.last_connection;
        // Check if the client reaches the timeout checking the last connection time.
        auto since_last_conn = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_conn);
        if(since_last_conn >= timeout)
        {
            // If dead, call the onDead callback and quit the client from the map.
            this->onDeadClient(client.second);
            dead_clients.push_back(client.first);
        }
        else
        {
            // If the client is not dead, check the minor timeout of the client to set
            // with the remain time to reach the timeout.
            min_remaining_time = std::min(min_remaining_time, timeout - since_last_conn);
        }
    }

    // Remove dead clients from the map.
    for(auto& client : dead_clients)
    {
        this->connected_clients_.erase(client);
    }

    // Disable the timeout if no clients remains or set the socket timeout
    // to the minimum remaining time to the timeout among all clients.
    if(this->connected_clients_.empty())
    {
        this->server_socket_->set(zmq::sockopt::rcvtimeo, -1);
    }
    else
    {
        this->server_socket_->set(zmq::sockopt::rcvtimeo, static_cast<int>(min_remaining_time.count()));
    }
}

void CommandServerBase::updateClientLastConnection(const std::string &id)
{
    // Safe mutex lock.
    std::unique_lock<std::mutex> lock(this->mtx_);
    // Update the client last connection.
    auto client_itr = this->connected_clients_.find(id);
    if(client_itr != this->connected_clients_.end())
        client_itr->second.last_connection = std::chrono::steady_clock::now();
}

void CommandServerBase::updateServerTimeout()
{
    // Calculate the minor timeout to set it into the socket.
    auto min_timeout = std::min_element(this->connected_clients_.begin(), this->connected_clients_.end(),
        [](const auto& a, const auto& b)
        {
        auto diff_a = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - a.second.last_connection);
        auto diff_b = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - b.second.last_connection);
        return diff_a.count() < diff_b.count();
        });

    if (min_timeout != this->connected_clients_.end())
    {
        auto remain_time = common::kDefaultClientAliveTimeoutMsec - std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::steady_clock::now() - min_timeout->second.last_connection).count();
        this->server_socket_->set(zmq::sockopt::rcvtimeo, std::max(0, static_cast<int>(remain_time)));
    }
    else
    {
        this->server_socket_->set(zmq::sockopt::rcvtimeo, -1);
    }
}

void CommandServerBase::resetSocket()
{
    // Auxiliar variables.
    int last_error_code = 0;
    const zmq::error_t* last_error;
    unsigned reconnect_count = common::kServerReconnTimes;

    // Delete the previous socket.
    if (this->server_socket_)
    {
        delete this->server_socket_;
        this->server_socket_ = nullptr;
    }

    // Try creating a new socket.
    do
    {
        try
        {
            // Create the ZMQ rep socket.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            this->server_socket_ = new zmq::socket_t(*this->context_, zmq::socket_type::rep);
            this->server_socket_->bind(this->server_endpoint_);
            this->server_socket_->set(zmq::sockopt::linger, 0);

            // Update the working flag and calls to the callback.
            this->onServerStart();
            this->flag_server_working_ = true;
            this->cv_server_depl_.notify_all();
        }
        catch (const zmq::error_t& error)
        {
            // Delete the socket and store the last error.
            if (this->server_socket_)
            {
                delete this->server_socket_;
                this->server_socket_ = nullptr;
            }
            last_error = &error;
            last_error_code = error.num();
            reconnect_count--;

            if (reconnect_count <= 0 || last_error_code != EADDRINUSE)
            {
                // Update the working flag and calls to the callback.
                this->onServerError(*last_error, "Error during socket creation.");
                this->flag_server_working_ = false;
                this->cv_server_depl_.notify_all();
                return;
            }
        }
    } while (reconnect_count > 0 && !this->flag_server_working_);
}

void CommandServerBase::onCustomCommandReceived(const CommandRequest&, CommandReply& rep)
{
    rep.result = ServerResult::NOT_IMPLEMENTED;
}

} // END NAMESPACES.
// =====================================================================================================================
