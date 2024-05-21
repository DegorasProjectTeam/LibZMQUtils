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
 * @file publisher_base.cpp
 * @brief This file contains the implementation of the PublisherBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
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
#include <zmq.hpp>
#include <zmq_addon.hpp>
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/PublisherSubscriber/publisher/publisher_base.h"
#include "LibZMQUtils/InternalHelpers/network_helpers.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
// =====================================================================================================================

// =====================================================================================================================
using zmqutils::internal_helpers::network::NetworkAdapterInfo;
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

PublisherBase::PublisherBase(unsigned publisher_port,
                             const std::string& publisher_iface,
                             const std::string& publisher_name,
                             const std::string& publisher_version,
                             const std::string& publisher_info) :
    socket_(nullptr),
    flag_working_(false)
{
    // Generate a unique UUID (v4) for the publisher.
    utils::UUID uuid = utils::UUIDGenerator::getInstance().generateUUIDv4();

    // Get the adapters.
    std::vector<NetworkAdapterInfo> interfcs = internal_helpers::network::getHostIPsWithInterfaces();

    // Auxiliar ip.
    std::string inter_aux = publisher_iface;

    // Update if localhost.
    if(inter_aux == "localhost")
        inter_aux = "127.0.0.1";

    // Store the adapters.
    if(inter_aux == "*")
        this->publisher_adapters_ = interfcs;
    else
    {
        for(const auto& intrfc : interfcs)
        {
            if(intrfc.ip == inter_aux)
                this->publisher_adapters_.push_back(intrfc);
            else if(intrfc.name == inter_aux)
                this->publisher_adapters_.push_back(intrfc);
        }
    }

    // Check for valid configuration.
    if(this->publisher_adapters_.empty())
    {
        std::string module = "[LibZMQUtils,PublisherSubscriber,PublisherBase] ";
        throw std::invalid_argument(module + "No interfaces found for address <" + inter_aux + ">.");
    }

    // Update the publisher information.
    this->pub_info_.uuid = uuid;
    this->pub_info_.port = publisher_port;
    this->pub_info_.endpoint = "tcp://" + inter_aux + ":" + std::to_string(publisher_port);
    this->pub_info_.hostname = internal_helpers::network::getHostname();
    this->pub_info_.name = publisher_name;
    this->pub_info_.info = publisher_info;
    this->pub_info_.version = publisher_version;
    this->pub_info_.ips = this->getPublisherIps();
}

PublisherBase::~PublisherBase()
{
    // Stop the publisher.
    // Warning: In this case the onPublisher callback can't be executed.
    this->internalStopPublisher();
}

void PublisherBase::onPublisherError(const zmq::error_t &, const std::string &)
{}

void PublisherBase::onPublisherStart()
{}

void PublisherBase::onPublisherStop()
{}

void PublisherBase::onSendingMsg(const PublishedMessage&)
{}

bool PublisherBase::startPublisher()
{
    // If publisher is already started, do nothing
    if (this->flag_working_)
        return false;

    // Start the publisher.
    bool res = this->internalResetPublisher();

    // Start the publisher.
    return res ? (this->onPublisherStart(), true) : false;
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
    return this->pub_info_.endpoint;
}

const PublisherInfo& PublisherBase::getPublisherInfo() const
{
    // NOTE: Mutex is not neccesary here.
    return this->pub_info_;
}

const utils::UUID &PublisherBase::getUUID() const
{
    return this->pub_info_.uuid;
}

std::vector<std::string> PublisherBase::getPublisherIps() const
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    std::vector<std::string> ips;
    for(const auto& intrfc : this->internalGetPublisherAddresses())
        ips.push_back(intrfc.ip);
    return ips;
}

std::string PublisherBase::getPublisherIpsStr(const std::string &separator) const
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    std::string ips;
    for(const auto& intrfc : this->internalGetPublisherAddresses())
    {
        ips.append(intrfc.ip);
        ips.append(separator);
    }
    if (!ips.empty() && separator.length() > 0)
        ips.erase(ips.size() - separator.size(), separator.size());
    return ips;
}

const PublisherBase::NetworkAdapterInfoV PublisherBase::getPublisherAddresses() const
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    return this->publisher_adapters_;
}

bool PublisherBase::isWorking() const
{
    return this->flag_working_;
}

OperationResult PublisherBase::sendMsg(const TopicType& topic, PublishedData& data)
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Result.
    OperationResult result = OperationResult::MSG_OK;

    // Check if we started the publisher.
    if (!this->socket_)
        return OperationResult::PUBLISHER_STOPPED;

    // Send the msg.
    try
    {
        // Prepare the message.
        PublishedMessage msg(topic, this->pub_info_, std::move(data), utils::currentISO8601Date(true, false, true));

        // Prepare the multipart msg.
        zmq::multipart_t multipart_msg(this->prepareMessage(topic, msg));

        // Call to the internal sending command callback.
        this->onSendingMsg(msg);

        // Send the msg.
        bool res = multipart_msg.send(*this->socket_);
        if (!res)
        {
            return OperationResult::INTERNAL_ZMQ_ERROR;
        }

    }
    catch (const zmq::error_t &error)
    {
        // Call to the error callback and stop the publisher for safety.
        std::string module = "[LibZMQUtils,PublisherSubscriber,PublisherBase] ";
        this->onPublisherError(error, module + "Error while sending a request. Stopping the publisher.");
        this->internalStopPublisher();
        return OperationResult::INTERNAL_ZMQ_ERROR;
    }

    // Return the result.
    return result;
}

const std::vector<NetworkAdapterInfo> &PublisherBase::getBoundInterfaces() const
{
    return this->publisher_adapters_;
}

std::string PublisherBase::operationResultToString(OperationResult result)
{
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

std::string PublisherBase::operationResultToString(ResultType result)
{
    return PublisherBase::operationResultToString(static_cast<OperationResult>(result));
}

bool PublisherBase::internalResetPublisher()
{
    // Lock.
    this->mtx_.lock();

    // Close the previous sockets to flush.
    this->deleteSockets();

    // Create the ZMQ socket.
    try
    {
        // Zmq publisher socket.
        this->socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::pub);
        this->socket_->bind(this->pub_info_.endpoint);
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

        // Unlock.
        this->mtx_.unlock();

        // Call to the internal callback.
        this->onPublisherError(error, "PublisherBase: Error while creating the publisher.");
        return false;
    }

    // Unlock.
    this->mtx_.unlock();

    // All ok
    return true;
}

const PublisherBase::NetworkAdapterInfoV &PublisherBase::internalGetPublisherAddresses() const
{
    return this->publisher_adapters_;
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

zmq::multipart_t PublisherBase::prepareMessage(const TopicType& topic, PublishedMessage& msg)
{
    // Serializer.
    serializer::BinarySerializer serializer;

    // Prepare the topic. This must come plain, since it is used by ZMQ topic filtering.
    zmq::message_t msg_topic(topic);

    // Prepare the uuid.
    size_t uuid_size = serializer.write(msg.pub_info.uuid.getBytes());
    zmq::message_t msg_uuid(serializer.release(), uuid_size);

    // Prepare the timestamp.
    size_t tp_size = serializer.write(msg.timestamp);
    zmq::message_t msg_tp(serializer.release(), tp_size);

    // Prepare the information.
    // TODO SEND THE IPS
    size_t info_size = serializer.write(msg.pub_info.endpoint, msg.pub_info.hostname, msg.pub_info.name,
                                        msg.pub_info.info, msg.pub_info.version);
    zmq::message_t msg_info(serializer.release(), info_size);

    // Prepare the multipart msg.
    zmq::multipart_t multipart_msg;
    multipart_msg.add(std::move(msg_topic));
    multipart_msg.add(std::move(msg_uuid));
    multipart_msg.add(std::move(msg_tp));
    multipart_msg.add(std::move(msg_info));

    // Add command parameters if they exist
    if (msg.data.size > 0)
    {
        // Prepare the command parameters
        zmq::message_t message_params(msg.data.bytes.get(), msg.data.size);
        multipart_msg.add(std::move(message_params));
    }

    // Return the multipart msg.
    return multipart_msg;
}

}} // END NAMESPACES.
// =====================================================================================================================
