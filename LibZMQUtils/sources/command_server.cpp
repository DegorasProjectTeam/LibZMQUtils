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
 * @file command_server.cpp
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
#include "LibZMQUtils/command_server.h"
#include "LibZMQUtils/utils.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

CommandServerBase::CommandServerBase(unsigned int port, const std::string& local_addr) :
    context_(nullptr),
    main_socket_(nullptr),
    server_endpoint_("tcp://" + local_addr + ":" + std::to_string(port)),
    server_port_(port),
    server_working_(false),
    check_clients_alive_(true)
{
    // Get the adapters.
    std::vector<utils::NetworkAdapterInfo> interfcs = utils::getHostIPsWithInterfaces();
    // Store the adapters.
    if(local_addr == "*")
        this->server_listen_adapters_ = interfcs;
    else
    {
        for(const auto& intrfc : interfcs)
        {
            if(intrfc.ip == local_addr)
                this->server_listen_adapters_.push_back(intrfc);
        }
    }
}

const std::future<void> &CommandServerBase::getServerWorkerFuture() const {return this->server_worker_future_;}

const std::map<std::string, HostClient> &CommandServerBase::getConnectedClients() const
{return this->connected_clients_;}

void CommandServerBase::setClientStatusCheck(bool)
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);
    // Disable the client alive checking.
    this->check_clients_alive_ = false;
    if(this->main_socket_)
        this->main_socket_->set(zmq::sockopt::rcvtimeo, -1);
}

const unsigned& CommandServerBase::getServerPort() const {return this->server_port_;}

const std::vector<utils::NetworkAdapterInfo>& CommandServerBase::getServerAddresses() const
{return this->server_listen_adapters_;}

const std::string& CommandServerBase::getServerEndpoint() const {return this->server_endpoint_;}

void CommandServerBase::startServer()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // If server is already started, do nothing
    if (this->server_working_)
        return;

    // Create the ZMQ context.
    if (!this->context_)
        this->context_ = new zmq::context_t(1);

    // Launch server worker in other thread.
    this->server_worker_future_ = std::async(std::launch::async, &CommandServerBase::serverWorker, this);
}

void CommandServerBase::stopServer()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // If server is already stopped, do nothing.
    if (!this->server_working_)
        return;

    // Set the shared working flag to false (is atomic).
    this->server_working_ = false;

    // Delete the context.
    if (this->context_)
    {
        delete this->context_;
        context_ = nullptr;
    }

    // Clean the clients.
    this->connected_clients_.clear();
}

CommandServerBase::~CommandServerBase()
{
    // Stop the server (this function also deletes the pointers).
    this->stopServer();
}

BaseServerResult CommandServerBase::execReqConnect(const CommandRequest& cmd_req)
{
    // Safe mutex lock.
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Check if the client is already connected.
    auto it = this->connected_clients_.find(cmd_req.client.id);
    if(it != this->connected_clients_.end())
        return BaseServerResult::ALREADY_CONNECTED;

    // Add the new client.
    this->connected_clients_[cmd_req.client.id] = cmd_req.client;

    // Update the timeout of the main socket.
    if(this->check_clients_alive_)
        this->updateServerTimeout();

    // Call to the internal callback.
    this->onConnected(cmd_req.client);

    // All ok.
    return BaseServerResult::COMMAND_OK;
}

BaseServerResult CommandServerBase::execReqDisconnect(const CommandRequest& cmd_req)
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
    if(this->check_clients_alive_)
        this->updateServerTimeout();

    // All ok.
    return BaseServerResult::COMMAND_OK;
}

void CommandServerBase::serverWorker()
{
    // Auxiliar variables.
    BaseServerResult result;

    // Set the working flag to true.
    this->server_working_ = true;

    // Start server socket
    this->resetSocket();

    // Server worker loop.
    // If there is no client connected wait for a client to connect or for an exit message. If there
    // is a client connected set timeout, so if no command comes in time, check the last time connection
    // for each client. The loop can be stopped (in a safe way) if using the stopServer() function.
    while(this->main_socket_ && this->server_working_)
    {
        // Message container.
        CommandRequest cmd_request;

        // Result container.
        CommandReply cmd_reply;

        // Receive the data.
        result = this->recvFromSocket(cmd_request);

        // Check all the clients status.
        if(this->check_clients_alive_)
            this->checkClientsAliveStatus();

        // Process the data.
        if(result == BaseServerResult::COMMAND_OK && !this->server_working_)
        {
            // In this case, we will close the server. Call to the internal callback.
            this->onServerStop();
        }
        else if(result == BaseServerResult::TIMEOUT_REACHED)
        {
            // DO NOTHING.
        }
        else if (result != BaseServerResult::COMMAND_OK)
        {
            // Internal callback.
            this->onInvalidMsgReceived(cmd_request);

            // Prepare the message.
            std::uint8_t res_buff[sizeof(BaseServerResult)];
            utils::binarySerializeDeserialize(&result, sizeof(BaseServerResult), res_buff);
            zmq::message_t message_res(res_buff, sizeof(BaseServerResult));

            // Send response callback.
            cmd_reply.result = result;
            this->onSendingResponse(cmd_reply);

            // Send the response.
            try
            {
                this->main_socket_->send(message_res, zmq::send_flags::none);
            }
            catch (const zmq::error_t &error)
            {
                // Check if we want to close the server.
                // The error code is for ZMQ EFSM error.
                if(!(error.num() == common::kZmqEFSMError && !this->server_working_))
                    this->onServerError(error, "Error while sending a response.");
            }
        }
        else if (result == BaseServerResult::COMMAND_OK)
        {
            // Reply id buffer.
            std::unique_ptr<std::uint8_t> rep_id_buff;

            // Execute the command.
            this->processCommand(cmd_request, cmd_reply);

            // Prepare the command result.
            CommandServerBase::prepareCommandResult(cmd_reply.result, rep_id_buff);
            zmq::message_t message_rep_id(rep_id_buff.get(), sizeof(common::CmdReplyRes));

            // Prepare the multipart msg.
            zmq::multipart_t multipart_msg;
            multipart_msg.add(std::move(message_rep_id));

            // Specific data.
            if(cmd_reply.result == BaseServerResult::COMMAND_OK && cmd_reply.params_size != 0)
            {
                // Prepare the custom response.
                zmq::message_t message_rep_custom(cmd_reply.params.get(), cmd_reply.params_size);
                multipart_msg.add(std::move(message_rep_custom));
            }

            // Sending callback.
            this->onSendingResponse(cmd_reply);

            // Send the message.
            try
            {
                multipart_msg.send(*this->main_socket_);
            }
            catch (const zmq::error_t &error)
            {
                // Check if we want to close the server.
                // The error code is for ZMQ EFSM error.
                if(!(error.num() == common::kZmqEFSMError && !this->server_working_))
                    this->onServerError(error, "Error while sending a response.");
            }
        }
    }

    // Delete pointers for clean finish the worker.
    if (this->main_socket_)
    {
        delete this->main_socket_;
        this->main_socket_ = nullptr;
    }
}

BaseServerResult CommandServerBase::recvFromSocket(CommandRequest& request)
{
    // Result variable.
    BaseServerResult result = BaseServerResult::COMMAND_OK;

    // Containers.
    bool recv_result;
    zmq::multipart_t multipart_msg;

    // Try to receive data. If an execption is thrown, receiving fails and an error code is generated.
    try
    {
        // Call to the internal waiting command callback.
        this->onWaitingCommand();

        // Wait the command.
        recv_result = multipart_msg.recv(*(this->main_socket_));

        // Store the raw data.
        request.raw_msg = multipart_msg.clone();
    }
    catch(zmq::error_t& error)
    {
        // Check if we want to close the server.
        // The error code is for ZMQ EFSM error.
        if(error.num() == common::kZmqEFSMError && !this->server_working_)
            return BaseServerResult::COMMAND_OK;

        // Else, call to error callback.
        this->onServerError(error, "Error while receiving a request.");
        return BaseServerResult::INTERNAL_ZMQ_ERROR;
    }

    // Check for empty msg or timeout reached.
    if (multipart_msg.empty() && !recv_result)
        return BaseServerResult::TIMEOUT_REACHED;
    else if (multipart_msg.empty())
        return BaseServerResult::EMPTY_MSG;

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
            return BaseServerResult::EMPTY_CLIENT_IP;

        // Get the hostname data.
        if (host_size_bytes > 0)
            hostname = std::string(static_cast<char*>(message_hostname.data()), host_size_bytes);
        else
            return BaseServerResult::EMPTY_CLIENT_NAME;

        // Get the ip data.
        if (host_size_bytes > 0)
            pid = std::string(static_cast<char*>(message_pid.data()), pid_size_bytes);
        else
            return BaseServerResult::EMPTY_CLIENT_PID;

        // Update the client info.
        request.client = HostClient(ip, hostname, pid);
        request.client.last_connection = std::chrono::steady_clock::now();

        // Update the last connection if the client is connected.
        this->updateClientLastConnection(request.client.id);

        // Get the command.
        if (command_size_bytes == sizeof(CmdRequestId))
        {
            int raw_command;
            utils::binarySerializeDeserialize(message_command.data(), sizeof(BaseServerCommand), &raw_command);
            // Validate the command.
            if(CommandServerBase::validateCommand(raw_command))
                request.command = static_cast<BaseServerCommand>(raw_command);
            else
            {
                request.command = BaseServerCommand::INVALID_COMMAND;
                return BaseServerResult::INVALID_MSG;
            }
        }
        else
        {
            request.command = BaseServerCommand::INVALID_COMMAND;
            return BaseServerResult::INVALID_MSG;
        }

        // If there is still one more part, they are the parameters.
        if (multipart_msg.size() == 1)
        {
            // Get the message and the size.
            zmq::message_t message_params = multipart_msg.pop();
            size_t params_size_bytes = message_params.size();

            std::cout<<multipart_msg.str()<<std::endl;
            std::cout<<params_size_bytes<<std::endl;

            // Check the parameters.
            if(params_size_bytes > 0)
            {
                // Get and store the parameters data.
                std::unique_ptr<std::uint8_t> params =
                    std::unique_ptr<std::uint8_t>(new std::uint8_t[params_size_bytes]);
                auto *params_pointer = static_cast<std::uint8_t*>(message_params.data());
                std::copy(params_pointer, params_pointer + params_size_bytes, params.get());
                request.params = std::move(params);
                request.params_size = params_size_bytes;
            }
            else
                return BaseServerResult::EMPTY_PARAMS;
        }
    }
    else
        return BaseServerResult::INVALID_PARTS;

    // Return the result.
    return result;
}

void CommandServerBase::prepareCommandResult(BaseServerResult result, std::unique_ptr<std::uint8_t>& data_out)
{
    data_out = std::unique_ptr<std::uint8_t>(new std::uint8_t[sizeof(BaseServerResult)]);
    utils::binarySerializeDeserialize(&result, sizeof(CmdReplyRes), data_out.get());
}

bool CommandServerBase::validateCommand(int raw_command)
{
    // Auxiliar variables.
    bool result = false;
    int reserved_cmd = static_cast<int>(common::BaseServerCommand::RESERVED_COMMANDS);
    int end_base_cmd = static_cast<int>(common::BaseServerCommand::END_BASE_COMMANDS);
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
    if (BaseServerCommand::REQ_CONNECT == request.command)
    {
        reply.result = this->execReqConnect(request);
    }
    else if(this->connected_clients_.find(request.client.id) == this->connected_clients_.end())
    {
        reply.result = BaseServerResult::CLIENT_NOT_CONNECTED;
    }
    else if (BaseServerCommand::REQ_DISCONNECT == request.command)
    {
        reply.result = this->execReqDisconnect(request);
    }
    else if (BaseServerCommand::REQ_ALIVE == request.command)
    {
        reply.result = BaseServerResult::COMMAND_OK;
    }
    else
    {
        // Custom command, so call the internal callback.
        this->onCustomCommandReceived(request, reply);

        // Chek for an invalid msg.
        if(reply.result == BaseServerResult::INVALID_MSG)
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

    // Disable the timeout if no clients remains or set the socket timeout to the
    // minimum remaining time to the timeout among all clients.
    if(this->connected_clients_.empty())
    {
        this->main_socket_->set(zmq::sockopt::rcvtimeo, -1);
    }
    else
    {
        this->main_socket_->set(zmq::sockopt::rcvtimeo, static_cast<int>(min_remaining_time.count()));
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
        this->main_socket_->set(zmq::sockopt::rcvtimeo, std::max(0, static_cast<int>(remain_time)));
    }
    else
    {
        this->main_socket_->set(zmq::sockopt::rcvtimeo, -1);
    }
}

void CommandServerBase::resetSocket()
{
    // Auxiliar variables.
    int res = 0;
    const zmq::error_t* last_error;
    unsigned reconnect_count = common::kReconnectTimes;

    // Delete the previous socket.
    if (this->main_socket_)
    {
        delete this->main_socket_;
        this->main_socket_ = nullptr;
    }
    // Try creating a new socket.
    do
    {
        try
        {
            // Create the ZMQ rep socket.
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            this->main_socket_ = new zmq::socket_t(*this->context_, zmq::socket_type::rep);
            this->main_socket_->bind(this->server_endpoint_);
            this->main_socket_->set(zmq::sockopt::linger, 0);
        }
        catch (const zmq::error_t& error)
        {
            // Delete the socket and store the last error.
            delete this->main_socket_;
            this->main_socket_ = nullptr;
            last_error = &error;
        }
        reconnect_count--;
    } while (res == EADDRINUSE && reconnect_count > 0);

    if (!this->main_socket_ )
    {
        // Update the working flag and calls to the callback.
        this->server_working_ = false;
        this->onServerError(*last_error, "Error during socket creation.");
    }
    else
    {
        // Call to the internal callback.
        this->onServerStart();
    }
}

void CommandServerBase::onCustomCommandReceived(const CommandRequest&, CommandReply& rep)
{
    rep.result = BaseServerResult::NOT_IMPLEMENTED;
}

} // END NAMESPACES.
// =====================================================================================================================
