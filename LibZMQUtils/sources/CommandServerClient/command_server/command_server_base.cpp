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
 * @file command_server_base.cpp
 * @brief This file contains the implementation of the CommandServerBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <stdio.h>
#include <thread>
#include <chrono>
// =====================================================================================================================

// ZMQ INCLUDES
// =====================================================================================================================
#include <zmq_addon.hpp>
#include <zmq.h>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/CommandServerClient/command_server/command_server_base.h"
#include "LibZMQUtils/Global/constants.h"
#include "LibZMQUtils/Utilities/utils.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
#include "LibZMQUtils/InternalHelpers/network_helpers.h"
// =====================================================================================================================

// =====================================================================================================================
using zmqutils::utils::UUID;
using zmqutils::internal_helpers::network::NetworkAdapterInfo;
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace reqrep{
// =====================================================================================================================

CommandServerBase::CommandServerBase(unsigned port,
                                     const std::string& ip_address,
                                     const std::string &server_name,
                                     const std::string& server_version,
                                     const std::string& server_info) :
    server_socket_(nullptr),
    flag_server_working_(false),
    flag_check_clients_alive_(true),
    flag_alive_callbacks_(true),
    client_alive_timeout_(kDefaultClientAliveTimeoutMsec),
    server_reconn_attempts_(kDefaultServerReconnAttempts),
    max_connected_clients_(kDefaultMaxNumberOfClients)
{
    // Generate a unique UUID (v4) for the server.
    utils::UUID uuid = utils::UUIDGenerator::getInstance().generateUUIDv4();

    // Get the adapters.
    std::vector<NetworkAdapterInfo> interfcs = internal_helpers::network::getHostIPsWithInterfaces();

    // Store the adapters.
    if(ip_address == "*")
        this->server_adapters_ = interfcs;
    else
    {
        for(const auto& intrfc : interfcs)
        {
            if(intrfc.ip == ip_address)
                this->server_adapters_.push_back(intrfc);
        }
    }

    // Check for valid configuration.
    if(this->server_adapters_.empty())
    {
        std::string module = "[LibZMQUtils,CommandServerClient,CommandServerBase] ";
        throw std::invalid_argument(module + "No interfaces found for address <" + ip_address + ">.");
    }

    // Update the server information.
    this->server_info_.uuid = uuid;
    this->server_info_.name = server_name;
    this->server_info_.port = port;
    this->server_info_.version = server_version;
    this->server_info_.info = server_info;
    this->server_info_.endpoint = "tcp://" + ip_address + ":" + std::to_string(port);
    this->server_info_.ips = this->getServerIps();
    this->server_info_.hostname = internal_helpers::network::getHostname();
}

const std::future<void> &CommandServerBase::getServerWorkerFuture() const
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    return this->fut_server_worker_;
}

const std::map<UUID, CommandClientInfo> &CommandServerBase::getConnectedClients() const
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    return this->connected_clients_;
}

bool CommandServerBase::isWorking() const
{
    return this->flag_server_working_;
}

void CommandServerBase::setClientAliveTimeout(const std::chrono::milliseconds& timeout)
{
    this->client_alive_timeout_ = static_cast<unsigned>(timeout.count());
    if(client_alive_timeout_ == 0)
        this->setClientStatusCheck(false);
}

void CommandServerBase::setReconectionAttempts(unsigned attempts)
{
    this->server_reconn_attempts_ = attempts;
}

void CommandServerBase::setMaxNumberOfClients(unsigned clients)
{
    if(!this->isWorking())
        this->max_connected_clients_ = clients;
}

void CommandServerBase::setClientStatusCheck(bool enable)
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);
    // Disable the client alive checking.
    this->flag_check_clients_alive_ = enable;
    if(this->server_socket_)
    {
        if(!enable)
            this->server_socket_->set(zmq::sockopt::rcvtimeo, -1);
        else
            this->server_socket_->set(zmq::sockopt::rcvtimeo, static_cast<int>(this->client_alive_timeout_));
    }
}

void CommandServerBase::setAliveCallbacksEnabled(bool flag)
{
    this->flag_alive_callbacks_ = flag;
}

const CommandServerInfo &CommandServerBase::getServerInfo() const
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    return this->server_info_;
}

const std::vector<NetworkAdapterInfo>& CommandServerBase::getServerAddresses() const
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    return this->server_adapters_;
}

std::string CommandServerBase::getServerIpsStr(const std::string &separator) const
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    std::string ips;
    for(const auto& intrfc : this->internalGetServerAddresses())
    {
        ips.append(intrfc.ip);
        ips.append(separator);
    }
    if (!ips.empty() && separator.length() > 0)
        ips.erase(ips.size() - separator.size(), separator.size());
    return ips;
}

std::vector<std::string> CommandServerBase::getServerIps() const
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    std::vector<std::string> ips;
    for(const auto& intrfc : this->internalGetServerAddresses())
        ips.push_back(intrfc.ip);
    return ips;
}

bool CommandServerBase::startServer()
{
    // If server is already started, do nothing
    if (this->flag_server_working_)
        return true;

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
    // If server is already stopped, do nothing
    if (!this->flag_server_working_)
        return ;

    // Mutex lock zone.
    {
        // Lock.
        std::unique_lock<std::mutex> lock(this->mtx_);

        // Stop the server.
        this->internalStopServer();
    }

    // Call to the internal callback.
    this->onServerStop();
}

std::string CommandServerBase::serverCommandToString(ServerCommand command) const
{
    // Mutex.
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Containers.
    std::int32_t enum_val = static_cast<std::int32_t>(command);
    std::size_t idx = static_cast<std::size_t>(command);
    std::string cmd_str = "UNKNOWN_COMMAND";

    // Call to the internal registered function.
    if(this->command_to_string_function_)
        cmd_str = this->command_to_string_function_(command);
    else if (enum_val < 0)
        cmd_str = "INVALID_COMMAND";
    else if (idx < std::size(ServerCommandStr))
        cmd_str = ServerCommandStr[idx];
    return cmd_str;
}

std::string CommandServerBase::serverCommandToString(CommandType command) const
{
    return this->serverCommandToString(static_cast<ServerCommand>(command));
}

std::string CommandServerBase::operationResultToString(OperationResult result) const
{
    // Mutex.
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Containers.
    std::int32_t enum_val = static_cast<std::int32_t>(result);
    std::size_t idx = static_cast<std::size_t>(result);
    std::string op_str = "UNKNOWN_OPERATION_RESULT";

    // Check the index.
    if (enum_val < 0)
        op_str = "INVALID_OPERATION_RESULT";
    else if (idx < std::size(OperationResultStr))
        op_str = OperationResultStr[idx];
    return op_str;
}

std::string CommandServerBase::operationResultToString(ResultType result) const
{
    return this->operationResultToString(static_cast<OperationResult>(result));
}

void CommandServerBase::internalStopServer()
{
    // If server is already stopped, do nothing.
    if (!this->flag_server_working_)
        return;

    // Set the shared working flag to false (is atomic).
    this->flag_server_working_ = false;

    // If the server is working.
    if(this->fut_server_worker_.valid() &&
        this->fut_server_worker_.wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
    {
        // Auxiliar endpoint.
        std::string endpoint = this->server_info_.endpoint;
        std::size_t found = endpoint.find("*");
        if (found != std::string::npos)
            endpoint.replace(found, 1, "127.0.0.1");

        // Auxiliar socket for closing.
        zmq::socket_t* sock = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::req);
        sock->connect(endpoint);
        sock->set(zmq::sockopt::linger, 0);

        // Message for closing.
        sock->send(zmq::message_t(), zmq::send_flags::none);

        // Wait the future.
        this->fut_server_worker_.wait();

        // Delete the auxiliar socket.
        delete sock;
    }

    // Delete the socket.
    if(this->server_socket_)
    {
        delete this->server_socket_;
        this->server_socket_ = nullptr;
    }

    // Safe sleep.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // Clean the clients.
    this->connected_clients_.clear();
}

CommandServerBase::~CommandServerBase()
{
    // Stop the server.
    this->internalStopServer();
}

OperationResult CommandServerBase::execReqConnect(CommandRequest& cmd_req, CommandReply& reply)
{
    // Auxiliar containers.
    CommandClientInfo client_info;

    // Prepare the serializer.
    serializer::BinarySerializer serializer(std::move(cmd_req.data.bytes), cmd_req.data.size);

    // Check the parameters.
    if(cmd_req.data.size == 0)
        return OperationResult::EMPTY_PARAMS;

    // Get the parameters.
    try
    {
        // Deserialize the client data.
        serializer.read(client_info);

        // Check the parameters.
        if(!internal_helpers::network::isValidIP(client_info.ip))
            return OperationResult::INVALID_CLIENT_IP;
    }
    catch (...)
    {
        return OperationResult::BAD_PARAMETERS;
    }

    // Lock zone.
    {
        std::unique_lock<std::mutex> lock(this->mtx_);

        // Check if the client is already connected.
        auto it = this->connected_clients_.find(cmd_req.client_uuid);
        if(it != this->connected_clients_.end())
            return OperationResult::ALREADY_CONNECTED;

        // Check the maximum number of connections.
        if(this->connected_clients_.size() >= this->max_connected_clients_)
            return OperationResult::MAX_CLIENTS_REACH;

        // Store the client last seen time.
        client_info.last_seen = std::chrono::high_resolution_clock::now();
        client_info.last_seen_steady = std::chrono::steady_clock::now();

        // Add the new client.
        this->connected_clients_[cmd_req.client_uuid] = client_info;

        // Update the timeout of the main socket.
        if(this->flag_check_clients_alive_)
            this->updateServerTimeout();

        // Prepare the reply.
        reply.data.size = serializer::BinarySerializer::fastSerialization(reply.data.bytes,
            this->server_info_.hostname, this->server_info_.name, this->server_info_.info, this->server_info_.version);
    }

    // Call to the internal callback.
    this->onConnected(client_info);

    // All ok.
    return OperationResult::COMMAND_OK;
}

OperationResult CommandServerBase::execReqDisconnect(const CommandRequest& cmd_req)
{
    // Temporal container.
    CommandClientInfo tmp_host;

    // Lock zone.
    {
        std::unique_lock<std::mutex> lock(this->mtx_);

        // Get the client.
        auto it = this->connected_clients_.find(cmd_req.client_uuid);

        // Temporal client.
        tmp_host = it->second;

        // Remove the client from the map of connected clients.
        this->connected_clients_.erase(it);

        // Update the timeout of the main socket.
        if(this->flag_check_clients_alive_)
            this->updateServerTimeout();

    }

    // Call to the internal callback.
    this->onDisconnected(tmp_host);

    // All ok.
    return OperationResult::COMMAND_OK;
}

OperationResult CommandServerBase::execReqGetServerTime(CommandReply& reply)
{
    // Get the UTC time.
    const std::string date = utils::currentISO8601Date(true, false, true);

    if(date.empty())
        return OperationResult::COMMAND_FAILED;

    // Serialize the date.
    reply.data.size = serializer::BinarySerializer::fastSerialization(reply.data.bytes, date);

    // All ok.
    return OperationResult::COMMAND_OK;
}

void CommandServerBase::serverWorker()
{
    // Containers.
    CommandRequest request;
    CommandReply reply;
    OperationResult op_res;

    // Start server socket inside a lock zone.
    {
        std::unique_lock<std::mutex> lock(this->mtx_);
        this->resetSocket();
    }

    // Check the socket status and call to the internal callbacks.
    if(this->flag_server_working_)
    {
        this->onServerStart();
    }
    else
    {
        std::string module = "[LibZMQUtils,CommandServerClient,CommandServerBase] ";
        this->onServerError(this->last_zmq_error_, module + "Error during socket creation.");
    }

    // Notify all the deployment.
    this->cv_server_depl_.notify_all();

    // Server worker loop.
    // If there is no client connected wait for a client to connect or for an exit message. If there
    // is a client connected set timeout, so if no command comes in time, check the last time connection
    // for each client. The loop can be stopped (in a safe way) if using the stopServer() function.
    while(this->server_socket_ && this->flag_server_working_)
    {
        // Resets all the containers.
        request.clear();
        reply.clear();

        // Call to the internal waiting command callback (check first the last request).
        if (request.command != ServerCommand::REQ_ALIVE || this->flag_alive_callbacks_)
            this->onWaitingCommand();

        // Clean the containers.
        request = CommandRequest();
        reply = CommandReply();

        // Receive the data.
        op_res = this->recvFromSocket(request);

        // Check all the clients status.
        if(op_res == OperationResult::COMMAND_OK && this->flag_server_working_ && this->flag_check_clients_alive_ )
            this->checkClientsAliveStatus();

        // Process the data.
        if(op_res == OperationResult::COMMAND_OK && !this->flag_server_working_)
        {
            // Nothing to do, we want close the server.
        }
        else if(op_res == OperationResult::TIMEOUT_REACHED && this->flag_check_clients_alive_)
        {
            this->checkClientsAliveStatus();
        }
        else if (op_res != OperationResult::COMMAND_OK)
        {
            // Store the result
            reply.result = op_res;
            reply.command = request.command;

            // Internal callback.
            this->onInvalidMsgReceived(request);

            // Send response callback.
            this->onSendingResponse(reply);

            // Prepare the message.
            serializer::BinarySerializer serializer;
            size_t size_res = serializer.write(reply.result);
            zmq::const_buffer buffer_res(serializer.release(), size_res);

            // Send the response.
            try
            {
                this->server_socket_->send(buffer_res, zmq::send_flags::none);
            }
            catch (const zmq::error_t &error)
            {
                // Store the last error.
                this->last_zmq_error_ = error;

                // Check if we want to close the server.
                // The error code is for ZMQ EFSM error.
                if(!(error.num() == kZmqEFSMError && !this->flag_server_working_))
                {
                    std::string module = "[LibZMQUtils,CommandServerClient,CommandServerBase] ";
                    this->onServerError(this->last_zmq_error_, module + "Error while sending a response.");
                }
            }
        }
        else if (op_res == OperationResult::COMMAND_OK)
        {
            // Store the command.
            reply.command = request.command;
            reply.result = op_res;

            // Execute the command.
            this->processCommand(request, reply);

            // Binary serializer.
            serializer::BinarySerializer serializer;

            // Prepare the multipart msg.
            zmq::multipart_t multipart_msg;

            // Prepare the command result.
            size_t size = serializer.write(reply.command, reply.result);
            multipart_msg.addmem(serializer.release(), size);

            // Specific data.
            if(reply.result == OperationResult::COMMAND_OK && reply.data.size != 0)
            {
                // Prepare the custom response.
                zmq::message_t message_rep_custom(reply.data.bytes.get(), reply.data.size);
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
                if(!(error.num() == kZmqEFSMError && !this->flag_server_working_))
                {
                    // Store the last error.
                    this->last_zmq_error_ = error;
                    // Call to the internal callback.
                    std::string module = "[LibZMQUtils,CommandServerClient,CommandServerBase] ";
                    this->onServerError(this->last_zmq_error_, module + "Error while sending a response.");
                }
            }
        }
    }
    // Finish the worker.
}

OperationResult CommandServerBase::recvFromSocket(CommandRequest& request)
{
    // Result variable.
    OperationResult result = OperationResult::COMMAND_OK;

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
        if(error.num() == kZmqEFSMError && !this->flag_server_working_)
            return OperationResult::COMMAND_OK;

        // Store the last error.
        this->last_zmq_error_ = error;

        // Call to the internal callback.
        std::string module = "[LibZMQUtils,CommandServerClient,CommandServerBase] ";

        this->onServerError(this->last_zmq_error_, module + "Error while receiving a request.");
        return OperationResult::INTERNAL_ZMQ_ERROR;
    }

    // Check if we want to close the server.
    if(recv_result && multipart_msg.size() == 1 && multipart_msg.begin()->empty() && !this->flag_server_working_)
        return OperationResult::COMMAND_OK;

    // Check for empty msg or timeout reached.
    if (multipart_msg.empty() && !recv_result)
        return OperationResult::TIMEOUT_REACHED;
    else if (multipart_msg.empty())
        return OperationResult::EMPTY_MSG;

    // Check the multipart msg size.
    if (multipart_msg.size() == 2 || multipart_msg.size() == 3)
    {
        // Get the multipart data.
        zmq::message_t msg_uuid = multipart_msg.pop();
        zmq::message_t msg_command = multipart_msg.pop();

        // First get the uuid data.
        if (msg_uuid.size() == UUID::kUUIDSize + sizeof(serializer::SizeUnit)*2)
        {
            std::array<std::byte, 16> uuid_bytes;
            serializer::BinarySerializer::fastDeserialization(std::move(msg_uuid.data()), msg_uuid.size(), uuid_bytes);
            request.client_uuid = UUID(uuid_bytes);
        }
        else
            return OperationResult::INVALID_CLIENT_UUID;

        // Update the last connection if the client is connected.
        this->updateClientLastConnection(request.client_uuid);

        // Get the command.
        if (msg_command.size() == sizeof(serializer::SizeUnit) + sizeof(CommandType))
        {
            // Auxiliar command container.
            std::int32_t raw_command;

            // Deserialize.
            serializer::BinarySerializer::fastDeserialization(msg_command.data(), msg_command.size(), raw_command);

            // Validate the base command or the external command.
            if(CommandServerBase::validateCommand(raw_command))
            {
                request.command = static_cast<ServerCommand>(raw_command);
            }
            else
            {
                request.command = ServerCommand::INVALID_COMMAND;
                return OperationResult::INVALID_MSG;
            }
        }
        else
        {
            request.command = ServerCommand::INVALID_COMMAND;
            return OperationResult::INVALID_MSG;
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
                serializer::BinarySerializer serializer(message_params.data(), message_params.size());
                request.data.size = serializer.moveUnique(request.data.bytes);
            }
            else
                return OperationResult::EMPTY_PARAMS;
        }
    }
    else
        return OperationResult::INVALID_PARTS;

    // Return the result.
    return result;
}

bool CommandServerBase::validateCommand(int raw_command)
{
    // Auxiliar variables.
    bool result = false;
    int reserved_cmd = static_cast<int>(ServerCommand::END_IMPL_COMMANDS);
    int end_base_cmd = static_cast<int>(ServerCommand::END_BASE_COMMANDS);
    // Check if the command is valid.
    if (raw_command >= kMinBaseCmdId && raw_command < reserved_cmd)
        result = true;
    else if(raw_command > end_base_cmd)
        result = true;
    return result;
}

const CommandServerBase::NetworkAdapterInfoV &CommandServerBase::internalGetServerAddresses() const
{
    return this->server_adapters_;
}

void CommandServerBase::processCommand(CommandRequest& request, CommandReply& reply)
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
        reply.result = this->execReqConnect(request, reply);
    }
    else if(this->connected_clients_.find(request.client_uuid) == this->connected_clients_.end())
    {
        reply.result = OperationResult::CLIENT_NOT_CONNECTED;
    }
    else if (ServerCommand::REQ_DISCONNECT == request.command)
    {
        reply.result = this->execReqDisconnect(request);
    }
    else if (ServerCommand::REQ_ALIVE == request.command)
    {
        reply.result = OperationResult::COMMAND_OK;
    }
    else if(ServerCommand::REQ_GET_SERVER_TIME == request.command)
    {
        reply.result = this->execReqGetServerTime(reply);
    }
    else if(this->validateCustomRequest(request))
    {
        // Call the internal callback.
        this->onCustomCommandReceived(request);

        // Custom command, so call to the custom process.
        this->processCustomCommand(request, reply);
    }
    else
    {
        reply.result = OperationResult::UNKNOWN_COMMAND;
    }
}

void CommandServerBase::processCustomCommand(CommandRequest& request, CommandReply &reply)
{
    auto iter = process_fnc_map_.find(request.command);
    if(iter != process_fnc_map_.end())
    {
        // Invoke the function.
        iter->second(request, reply);
    }
    else
    {
        // Command not found in the map.
        reply.result = OperationResult::NOT_IMPLEMENTED;
    }
}

void CommandServerBase::checkClientsAliveStatus()
{
    // Auxiliar containers.
    std::vector<UUID> dead_clients;
    std::chrono::milliseconds timeout(this->client_alive_timeout_);
    std::chrono::milliseconds min_remaining_time = timeout;
    std::vector<CommandClientInfo> deleted_clients;

    // Get the current time.
    utils::SCTimePointStd now = std::chrono::steady_clock::now();

    // Lock zone.
    {
        std::unique_lock<std::mutex> lock(this->mtx_);

        // Check each connection.
        for(const auto& client : this->connected_clients_)
        {
            // Get the last connection time.
            const auto& last_conn = client.second.last_seen_steady;
            // Check if the client reaches the timeout checking the last connection time.
            auto since_last_conn = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_conn);
            if(since_last_conn >= timeout)
            {
                // If dead, call the onDead callback and quit the client from the map.
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
        for(const auto& client_uuid : dead_clients)
        {
            const auto tmp_client = this->connected_clients_.at(client_uuid);
            this->connected_clients_.erase(client_uuid);
            deleted_clients.push_back(tmp_client);
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

    // Call to the internal callback.
    for(const auto& client : deleted_clients)
        this->onDeadClient(client);
}

void CommandServerBase::updateClientLastConnection(const UUID& uuid)
{
    // Update the client last connection.
    auto client_itr = this->connected_clients_.find(uuid);
    if(client_itr != this->connected_clients_.end())
    {
        client_itr->second.last_seen = std::chrono::high_resolution_clock::now();
        client_itr->second.last_seen_steady = std::chrono::steady_clock::now();
    }
}

void CommandServerBase::updateServerTimeout()
{
    // Calculate the minor timeout to set it into the socket.
    auto min_timeout = std::min_element(this->connected_clients_.begin(), this->connected_clients_.end(),
        [](const auto& a, const auto& b)
        {
        auto diff_a = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - a.second.last_seen_steady);
        auto diff_b = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - b.second.last_seen_steady);
        return diff_a.count() < diff_b.count();
        });

    if (min_timeout != this->connected_clients_.end())
    {
        auto remain_time = this->client_alive_timeout_ -
                           std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::steady_clock::now() - min_timeout->second.last_seen_steady).count();
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
    int reconnect_count = static_cast<int>(this->server_reconn_attempts_);

    // Stop the socket.
    this->internalStopServer();

    // Try creating a new socket.
    do
    {
        try
        {
            // Create the ZMQ rep socket.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            this->server_socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::rep);
            this->server_socket_->bind(this->server_info_.endpoint);
            this->server_socket_->set(zmq::sockopt::linger, 0);

            // Update the working flag.
            this->flag_server_working_ = true;
        }
        catch (const zmq::error_t& error)
        {
            // Delete the socket and store the last error.
            if (this->server_socket_)
            {
                delete this->server_socket_;
                this->server_socket_ = nullptr;
            }

            // Store the last error.
            this->last_zmq_error_ = error;

            last_error_code = error.num();
            reconnect_count--;

            if (reconnect_count <= 0 || last_error_code != EADDRINUSE)
            {
                // Update the working flag and calls to the callback.
                this->flag_server_working_ = false;
                return;
            }
        }
    } while (reconnect_count > 0 && !this->flag_server_working_);
}

}} // END NAMESPACES.
// =====================================================================================================================
