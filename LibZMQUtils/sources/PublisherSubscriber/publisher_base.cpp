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
 * @file publisher_base.cpp
 * @brief This file contains the implementation of the PublisherBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <string>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <chrono>
// =====================================================================================================================

// ZMQ INCLUDES
// =====================================================================================================================
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/PublisherSubscriber/publisher_base.h"
#include "LibZMQUtils/InternalHelpers/network_helpers.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
// =====================================================================================================================

// =====================================================================================================================
using zmqutils::internal_helpers::network::NetworkAdapterInfo;
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils
{
// =====================================================================================================================

PublisherBase::PublisherBase(std::string endpoint,
                             std::string name) :
    endpoint_(std::move(endpoint)),
    socket_(nullptr),
    flag_working_(false)
{
    // Generate a unique UUID (v4) for the publisher.
    utils::UUID uuid = utils::UUIDGenerator::getInstance().generateUUIDv4();

    this->pub_info_.name = std::move(name);
    this->pub_info_.uuid = std::move(uuid);


    // Get the addr of interface for binding. If * symbol is issued, bind to every interface.
    std::string local_addr;
    std::regex addr_regexp(R"(://(.*):)");
    std::smatch match;
    std::regex_search(this->endpoint_, match, addr_regexp);

    if (2 == match.size())
        local_addr = match[1];

    // If addr was found, find the selected interface(s)
    if (!local_addr.empty())
    {
        std::vector<NetworkAdapterInfo> ifaces = internal_helpers::network::getHostIPsWithInterfaces();
        // Store the adapters.
        if(local_addr == "*")
            this->bound_ifaces_ = ifaces;
        else
        {
            for(const auto& iface : ifaces)
            {
                if(iface.ip == local_addr)
                    this->bound_ifaces_.push_back(iface);
            }
        }
    }

    // Check for valid configuration.
    if(this->bound_ifaces_.empty())
        throw std::invalid_argument("PublisherBase: No interfaces found for address <" + local_addr + ">.");
}

PublisherBase::~PublisherBase()
{
    // Stop the publisher.
    // Warning: In this case the onPublisher callback can't be executed.
    this->internalStopPublisher();
}

bool PublisherBase::startPublisher()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // If publisher is already started, do nothing
    if (this->socket_)
        return false;

    // Start the publisher.
    return this->internalResetPublisher() ? (this->onPublisherStart(), true) : false;
}

void PublisherBase::stopPublisher()
{    
    // Atomic.
    // If publisher is already stopped, do nothing.
    if (!this->flag_working_)
        return;

    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Call to the internal stop.
    this->internalStopPublisher();

    // Call to the internal callback.
    this->onPublisherStop();
}

bool PublisherBase::resetPublisher()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Call to the internal method.
    return this->internalResetPublisher();
}

const std::string &PublisherBase::getEndpoint() const
{
    // NOTE: Mutex is not neccesary here.
    return this->endpoint_;
}

const std::string &PublisherBase::getName() const
{
    // NOTE: Mutex is not neccesary here.
    return this->pub_info_.name;
}

bool PublisherBase::isWorking() const
{
    return this->flag_working_;
}

ClientResult PublisherBase::sendMsg(const common::PubSubData& request)
{
    // Result.
    ClientResult result = ClientResult::COMMAND_OK;

    // Check if we started the publisher.
    if (!this->socket_)
        return ClientResult::CLIENT_STOPPED;

    // Send the msg.
    try
    {
        // Prepare the multipart msg.
        zmq::multipart_t multipart_msg(this->prepareMessage(request));

        // Call to the internal sending command callback.
        this->onSendingMsg(request);

        // Send the msg.
        multipart_msg.send(*this->socket_);
    }
    catch (const zmq::error_t &error)
    {
        // Call to the error callback and stop the publisher for safety.
        this->onPublisherError(error, "PublisherBase: Error while sending a request. Stopping the publisher.");
        this->internalStopPublisher();
        return ClientResult::INTERNAL_ZMQ_ERROR;
    }

    // Return the result.
    return result;
}

const std::vector<NetworkAdapterInfo> &PublisherBase::getBoundInterfaces() const
{
    return this->bound_ifaces_;
}

bool PublisherBase::internalResetPublisher()
{
    // Close the previous sockets to flush.
    this->deleteSockets();

    // Create the ZMQ socket.
    try
    {
        // Zmq publisher socket.
        this->socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::pub);
        this->socket_->connect(this->endpoint_);
        this->socket_->set(zmq::sockopt::linger, 0);

        // Update the working flag.
        this->flag_working_ = true;
    }
    catch (const zmq::error_t &error)
    {
        // Delete the sockets.
        this->deleteSockets();

        // Update the working flag.
        this->flag_working_ = false;

        // Call to the internal callback.
        this->onPublisherError(error, "PublisherBase: Error while creating the publisher.");
        return false;
    }

    // All ok
    return true;
}

void PublisherBase::deleteSockets()
{
    // Delete the pointers.
    if(this->socket_)
    {
        delete this->socket_;
        this->socket_ = nullptr;
    }
}

void PublisherBase::internalStopPublisher()
{
    // If server is already stopped, do nothing.
    if (!this->flag_working_)
        return;

    // Set the shared working flag to false (is atomic).
    this->flag_working_ = false;

    // Delete the sockets.
    this->deleteSockets();

    // Safe sleep.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}


zmq::multipart_t PublisherBase::prepareMessage(const common::PubSubData &data)
{
    // Serializer.
    utils::BinarySerializer serializer;

    // Prepare the topic
    size_t topic_size = serializer.write(data.topic);
    zmq::message_t msg_topic(serializer.release(), topic_size);

    // Prepare the name message.
    size_t name_size = serializer.write(this->pub_info_.name);
    zmq::message_t msg_name(serializer.release(), name_size);

    // Prepare the uuid message.
    size_t uuid_size = serializer.write(this->pub_info_.uuid.getBytes());
    zmq::message_t msg_uuid(serializer.release(), uuid_size);

    // Prepare the multipart msg.
    zmq::multipart_t multipart_msg;
    multipart_msg.add(std::move(msg_topic));
    multipart_msg.add(std::move(msg_name));
    multipart_msg.add(std::move(msg_uuid));


    // Add command parameters if they exist
    if (data.data_size > 0)
    {
        // Prepare the command parameters
        zmq::message_t message_params(data.data.get(), data.data_size);
        multipart_msg.add(std::move(message_params));
    }

    // Return the multipart msg.
    return multipart_msg;
}

} // END NAMESPACES.
// =====================================================================================================================
