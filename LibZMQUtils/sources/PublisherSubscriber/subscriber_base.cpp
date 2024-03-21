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
 * @file subscriber_base.cpp
 * @brief This file contains the implementation of the SubscriberBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <stdio.h>
#include <thread>
#include <chrono>
// =====================================================================================================================

// ZMQ INCLUDES
// =====================================================================================================================
#include <zmq/zmq_addon.hpp>
#include <zmq/zmq.h>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/PublisherSubscriber/subscriber_base.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

SubscriberBase::SubscriberBase() :
    socket_(nullptr),
    socket_pub_close_(nullptr),
    sub_uuid_(utils::UUIDGenerator::getInstance().generateUUIDv4()),
    flag_working_(false)

{

}

const std::future<void> &SubscriberBase::getWorkerFuture() const {return this->fut_worker_;}

const std::map<UUID, common::PublisherInfo> &SubscriberBase::getSubscribedPublishers() const
{return this->subscribed_publishers_;}

bool SubscriberBase::isWorking() const{return this->flag_working_;}

bool SubscriberBase::startSubscriber()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // If server is already started, do nothing
    if (this->flag_working_)
        return true;

    // Launch server worker in other thread.
    this->fut_worker_ = std::async(std::launch::async, &SubscriberBase::serverWorker, this);

    // Wait for the server deployment.
    std::unique_lock<std::mutex> depl_lock(this->depl_mtx_);
    this->cv_server_depl_.wait(depl_lock);

    // Return the server status.
    return this->flag_working_;
}

void SubscriberBase::stopSubscriber()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Call to the internal stop.
    this->internalStopSubscriber();
}

void SubscriberBase::internalStopSubscriber()
{
    // If server is already stopped, do nothing.
    if (!this->flag_working_)
        return;

    // Set the shared working flag to false (is atomic).
    this->flag_working_ = false;

    // If the server is working.
    if(this->fut_worker_.valid() &&
        this->fut_worker_.wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
    {

        // Message for closing.
        zmq::multipart_t msg;
        msg.addstr("quit");
        msg.send(*this->socket_pub_close_);

        // Wait the future.
        this->fut_worker_.wait();
    }

    // Delete the sockets.
    if(this->socket_)
    {
        delete this->socket_;
        this->socket_ = nullptr;
    }

    if (this->socket_pub_close_)
    {
        delete this->socket_pub_close_;
        this->socket_pub_close_ = nullptr;
    }

    // Safe sleep.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // Clean the clients.
    this->subscribed_publishers_.clear();
}

SubscriberBase::~SubscriberBase()
{
    // Stop the server.
    this->internalStopSubscriber();
}


void SubscriberBase::serverWorker()
{
    // Containers.
    ServerResult result;

    // Start server socket
    this->resetSocket();

    // Server worker loop.
    // If there is no client connected wait for a client to connect or for an exit message. If there
    // is a client connected set timeout, so if no command comes in time, check the last time connection
    // for each client. The loop can be stopped (in a safe way) if using the stopServer() function.
    while(this->socket_ && this->flag_working_)
    {
        // Call to the internal waiting command callback (check first the last request).
        if (request.command != ServerCommand::REQ_ALIVE || this->flag_alive_callbacks_)
            this->onWaitingCommand();

        // Clean the containers.
        request = CommandRequest();
        reply = CommandReply();

        // Receive the data.
        result = this->recvFromSocket(request);

        // Process the data.
        if(result == ServerResult::COMMAND_OK && !this->flag_working_)
        {
            // In this case, we will close the server. Call to the internal callback.
            this->onServerStop();
        }
        else if(result == ServerResult::TIMEOUT_REACHED && this->flag_check_clients_alive_)
        {
            this->checkClientsAliveStatus();
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
                this->socket_->send(buffer_res, zmq::send_flags::none);
            }
            catch (const zmq::error_t &error)
            {
                // Check if we want to close the server.
                // The error code is for ZMQ EFSM error.
                if(!(error.num() == common::kZmqEFSMError && !this->flag_working_))
                    this->onServerError(error, "SubscriberBase: Error while sending a response.");
            }
        }
        else if (result == ServerResult::COMMAND_OK)
        {
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
                multipart_msg.send(*this->socket_);
            }
            catch (const zmq::error_t &error)
            {
                // Check if we want to close the server.
                // The error code is for ZMQ EFSM error.
                if(!(error.num() == common::kZmqEFSMError && !this->flag_working_))
                    this->onServerError(error, "SubscriberBase: Error while sending a response.");
            }
        }
    }
    // Finish the worker.
}

ServerResult SubscriberBase::recvFromSocket(CommandRequest& request)
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
        recv_result = multipart_msg.recv(*(this->socket_));
    }
    catch(zmq::error_t& error)
    {
        // Check if we want to close the server.
        // The error code is for ZMQ EFSM error.
        if(error.num() == common::kZmqEFSMError && !this->flag_working_)
            return ServerResult::COMMAND_OK;

        // Else, call to error callback.
        this->onServerError(error, "SubscriberBase: Error while receiving a request.");
        return ServerResult::INTERNAL_ZMQ_ERROR;
    }

    // Check if we want to close the server.
    if(recv_result && multipart_msg.size() == 1 && multipart_msg.begin()->empty() && !this->flag_working_)
        return ServerResult::COMMAND_OK;

    // Check for empty msg or timeout reached.
    if (multipart_msg.empty() && !recv_result)
        return ServerResult::TIMEOUT_REACHED;
    else if (multipart_msg.empty())
        return ServerResult::EMPTY_MSG;

    // Check the multipart msg size.
    if (multipart_msg.size() == 2 || multipart_msg.size() == 3)
    {
        // Get the multipart data.
        zmq::message_t msg_uuid = multipart_msg.pop();
        zmq::message_t msg_command = multipart_msg.pop();

        // First get the uuid data.
        if (msg_uuid.size() == UUID::kUUIDSize + sizeof(utils::BinarySerializer::SizeUnit)*2)
        {
            std::array<std::byte, 16> uuid_bytes;
            utils::BinarySerializer::fastDeserialization(msg_uuid.data(), msg_uuid.size(), uuid_bytes);
            request.client_uuid = UUID(uuid_bytes);
        }
        else
            return ServerResult::INVALID_CLIENT_UUID;

        // Update the last connection if the client is connected.
        this->updateClientLastConnection(request.client_uuid);

        // Get the command.
        if (msg_command.size() == sizeof(utils::BinarySerializer::SizeUnit) + sizeof(CommandType))
        {
            // Auxiliar command container.
            std::int32_t raw_command;

            // Deserialize.
            utils::BinarySerializer::fastDeserialization(msg_command.data(), msg_command.size(), raw_command);

            // Validate the base command or the external command.
            if(SubscriberBase::validateCommand(raw_command))
            {
                request.command = static_cast<ServerCommand>(raw_command);
            }
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
                request.params_size = serializer.moveUnique(request.params);
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



void SubscriberBase::resetSocket()
{
    // Auxiliar variables.
    const zmq::error_t* last_error;

    // Stop the socket.
    this->internalStopSubscriber();

    // Try creating a new socket.
    try
    {
        // Create the ZMQ sub socket.
        auto close_endpoint = "inproc://" + this->sub_uuid_.toRFC4122String();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        this->socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::sub);
        this->socket_->set(zmq::sockopt::linger, 0);
        this->socket_->connect(close_endpoint);
        this->socket_->set(zmq::sockopt::subscribe, "quit");

        this->socket_pub_close_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::pub);
        this->socket_pub_close_->bind(close_endpoint);
        this->socket_pub_close_->set(zmq::sockopt::linger, 0);

        // Update the working flag and calls to the callback.
        this->onSubscriberStart();
        this->flag_working_ = true;
        this->cv_server_depl_.notify_all();
    }
    catch (const zmq::error_t& error)
    {
        // Delete the sockets and store the last error.
        if (this->socket_)
        {
            delete this->socket_;
            this->socket_ = nullptr;
        }

        if (this->socket_pub_close_)
        {
            delete this->socket_pub_close_;
            this->socket_pub_close_ = nullptr;
        }
        last_error = &error;

        // Update the working flag and calls to the callback.
        this->onSubscriberError(*last_error, "Error during socket creation.");
        this->flag_working_ = false;
        this->cv_server_depl_.notify_all();
        return;

    }

}


} // END NAMESPACES.
// =====================================================================================================================
