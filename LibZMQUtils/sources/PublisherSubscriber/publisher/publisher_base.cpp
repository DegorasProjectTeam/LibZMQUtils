/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
 *   A modern open-source and cross-platform C++ library with high-level utilities based on the well-known ZeroMQ      *
 *   open-source universal messaging library. Includes a suite of modules that encapsulates the ZMQ communication      *
 *   patterns as well as automatic binary serialization capabilities, specially designed for system infraestructure.   *
 *   The library is suited for the quick and easy integration of new and old systems and can be used in different      *
 *   sectors and disciplines seeking robust messaging and serialization solutions.                                     *
 *                                                                                                                     *
 *   Developed as free software within the context of the Degoras Project for the Satellite Laser Ranging Station      *
 *   (SFEL) at the Spanish Navy Observatory (ROA) in San Fernando, Cádiz. The library is open for use by other SLR     *
 *   stations and organizations, so we warmly encourage you to give it a try and feel free to contact us anytime!      *
 *                                                                                                                     *
 *   Copyright (C) 2024 Degoras Project Team                                                                           *
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
#include "LibZMQUtils/Utilities/utils.h"
// =====================================================================================================================

// =====================================================================================================================
using zmqutils::internal_helpers::network::NetworkAdapterInfo;
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

// CONSTANTS
// =====================================================================================================================
constexpr unsigned kDefaultPublisherReconnAttempts = 5;        ///< Default publisher reconnection number of attempts.
// =====================================================================================================================

PublisherBase::PublisherBase(unsigned publisher_port,
                             const std::string& publisher_iface,
                             const std::string& publisher_name,
                             const std::string& publisher_version,
                             const std::string& publisher_info) :
    publisher_socket_(nullptr),
    flag_publisher_working_(false),
    publisher_reconn_attempts_(kDefaultPublisherReconnAttempts),
    stop_queue_worker_(false)
{
    // Auxiliar variables and containers.
    std::string inter_aux = publisher_iface;

    // Generate a unique UUID (v4) for the publisher.
    utils::UUID uuid = utils::UUIDGenerator::getInstance().generateUUIDv4();

    // Get the adapters.
    std::vector<NetworkAdapterInfo> ifaces = internal_helpers::network::getHostIPsWithInterfaces();

    // Check if we have active interfaces.
    if(ifaces.empty())
        throw std::invalid_argument(PublisherBase::kClassScope + " No active network interfaces found.");

    // Check the server interface.
    if(publisher_iface.empty())
        throw std::invalid_argument(PublisherBase::kClassScope + " The server network interface can't be empty.");

    // Update if localhost.
    if(publisher_iface == "localhost")
        inter_aux = "127.0.0.1";

    // Store the adapters.
    if(publisher_iface == "*")
        this->publisher_adapters_ = ifaces;
    else
    {
        for(const auto& intrfc : ifaces)
        {
            if(intrfc.ip == inter_aux)
                this->publisher_adapters_.push_back(intrfc);
            else if(intrfc.name == inter_aux)
                this->publisher_adapters_.push_back(intrfc);
        }
    }

    // Check for valid configuration.
    if(this->publisher_adapters_.empty())
        throw std::invalid_argument(PublisherBase::kClassScope + " No interfaces found for <" + publisher_iface + ">.");

    // Update the publisher information.
    this->pub_info_.uuid = uuid;
    this->pub_info_.port = publisher_port;
    this->pub_info_.endpoint = "tcp://" + publisher_iface + ":" + std::to_string(publisher_port);
    this->pub_info_.hostname = internal_helpers::network::getHostname();
    this->pub_info_.name = publisher_name;
    this->pub_info_.info = publisher_info;
    this->pub_info_.version = publisher_version;
    this->pub_info_.ips = this->getPublisherIps();
}

void PublisherBase::internalEnqueueMsg(PublishedMessage &msg)
{
    std::unique_lock<std::mutex> lock(this->queue_mutex_);

    switch (msg.priority)
    {
    case MessagePriority::CriticalPriority:
        this->queue_prio_critical_.push(std::move(msg));
        break;
    case MessagePriority::HighPriority:
        this->queue_prio_high_.push(std::move(msg));
        break;
    case MessagePriority::NormalPriority:
        this->queue_prio_normal_.push(std::move(msg));
        break;
    case MessagePriority::LowPriority:
        this->queue_prio_low_.push(std::move(msg));
        break;
    case MessagePriority::NoPriority:
        this->queue_prio_no_.push(std::move(msg));
        break;
    }

    // Unlock and signal the worker thread to process messages.
    lock.unlock();
    this->queue_cv_.notify_one();
}

void PublisherBase::messageQueueWorker()
{
    // Worker infinite loop.
    while (!this->stop_queue_worker_)
    {
        // Lock for cv.
        std::unique_lock<std::mutex> lock(this->queue_mutex_);

        // Wait for a new msg in the queue.
        this->queue_cv_.wait(lock, [this]
        {
            return stop_queue_worker_ ||
                   !this->queue_prio_critical_.empty() ||
                   !this->queue_prio_high_.empty() ||
                   !this->queue_prio_normal_.empty() ||
                   !this->queue_prio_low_.empty() ||
                   !this->queue_prio_no_.empty();
        });

        // Stop the worker case.
        if (this->stop_queue_worker_)
            break;

        // Storage for published msg.
        PublishedMessage msg;

        // Move the msg.
        if (!this->queue_prio_critical_.empty())
        {
            msg = std::move(this->queue_prio_critical_.front());
            this->queue_prio_critical_.pop();
        }
        else if (!this->queue_prio_high_.empty())
        {
            msg = std::move(this->queue_prio_high_.front());
            this->queue_prio_high_.pop();
        }
        else if (!this->queue_prio_normal_.empty())
        {
            msg = std::move(this->queue_prio_normal_.front());
            this->queue_prio_normal_.pop();
        }
        else if (!queue_prio_low_.empty())
        {
            msg = std::move(this->queue_prio_low_.front());
            this->queue_prio_low_.pop();
        }
        else if (!this->queue_prio_no_.empty())
        {
            msg = std::move(this->queue_prio_no_.front());
            this->queue_prio_no_.pop();
        }
        else
        {
            continue;
        }

        // Unlock.
        lock.unlock();

        // Send the message via ZMQ
        try
        {
            // Prepare the multipart msg.
            zmq::multipart_t multipart_msg(this->prepareMessage(msg.topic, msg));

            // Call to the internal sending command callback.
            this->onSendingMsg(msg);

            // Send the msg.
            bool res = multipart_msg.send(*this->publisher_socket_);
            if (!res)
            {
                // Custom error for 0 bytes sent.
                this->onPublisherError(zmq::error_t(), this->kClassScope + " No data was sent (0 bytes).");
            }
        }
        catch (const zmq::error_t& error)
        {
            // Call to the error callback and stop the publisher for safety.
            this->onPublisherError(error, this->kClassScope + " Error while sending a request. Stopping the publisher.");
            this->internalStopPublisher();
        }
    }
}

PublisherBase::~PublisherBase()
{   
    // Stop the publisher.
    // Warning: In this case the onPublisherStop callback can't be executed.
    std::unique_lock<std::shared_mutex> lock(this->pub_mtx_);
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
    if (this->flag_publisher_working_)
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
    if (!this->flag_publisher_working_)
        return;

    // Safe mutex lock
    std::unique_lock<std::shared_mutex> lock(this->pub_mtx_);

    // Call to the internal stop.
    this->internalStopPublisher();

    // Call to the internal callback.
    this->onPublisherStop();
}

bool PublisherBase::resetPublisher()
{
    // Safe mutex lock
    std::unique_lock<std::shared_mutex> lock(this->pub_mtx_);

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
    std::shared_lock<std::shared_mutex> lock(this->pub_mtx_);
    std::vector<std::string> ips;
    for(const auto& intrfc : this->internalGetPublisherAddresses())
        ips.push_back(intrfc.ip);
    return ips;
}

std::string PublisherBase::getPublisherIpsStr(const std::string &separator) const
{
    std::shared_lock<std::shared_mutex> lock(this->pub_mtx_);
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
    std::unique_lock<std::shared_mutex> lock(this->pub_mtx_);
    return this->publisher_adapters_;
}

bool PublisherBase::isWorking() const
{
    return this->flag_publisher_working_;
}

OperationResult PublisherBase::enqueueMsg(const TopicType& topic, MessagePriority priority, PublishedData& data)
{
    // Safe mutex lock
    std::unique_lock<std::shared_mutex> lock(this->pub_mtx_);

    // Check if we started the publisher.
    if (!this->publisher_socket_)
        return OperationResult::PUBLISHER_STOPPED;

    // Prepare the message.
    PublishedMessage msg(topic, this->pub_info_, utils::currentISO8601Date(true, false, true),
                         std::move(data), priority);

    // Enqueue the msg.
    this->internalEnqueueMsg(msg);

    // Return the result.
    return OperationResult::OPERATION_OK;
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
    // Auxiliar variables.
    int last_error_code = 0;
    int reconnect_count = static_cast<int>(this->publisher_reconn_attempts_);

    // Lock.
    this->pub_mtx_.lock();

    // Close the previous sockets to flush.
    this->deleteSockets();

    // Stop the worker thread.
    this->stop_queue_worker_ = true;
    this->queue_cv_.notify_all();
    if (this->queue_worker_th_.joinable())
        this->queue_worker_th_.join();
    this->stop_queue_worker_ = false;

    // Try creating a new socket.
    do{
        try
        {
            // Zmq publisher socket.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            this->publisher_socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::pub);
            this->publisher_socket_->bind(this->pub_info_.endpoint);
            this->publisher_socket_->set(zmq::sockopt::linger, 0);

            // Prepare the queues worker thread.
            this->queue_worker_th_ = std::thread(&PublisherBase::messageQueueWorker, this);

            // Update the working flag.
            this->flag_publisher_working_ = true;
        }
        catch (const zmq::error_t &error)
        {
            // Delete the socket and store the last error.
            if (this->publisher_socket_)
            {
                delete this->publisher_socket_;
                this->publisher_socket_ = nullptr;
            }

            // Store the last error.
            this->last_zmq_error_ = error;
            last_error_code = error.num();
            reconnect_count--;

            // Check reconnection case.
            if (reconnect_count <= 0 || last_error_code != EADDRINUSE)
            {
                // Update the working flag.
                this->flag_publisher_working_ = false;

                // Unlock.
                this->pub_mtx_.unlock();

                // Call to the internal callback.
                this->onPublisherError(error, this->kClassScope + " Error while creating the publisher.");
                return false;
            }
        }
    }while(reconnect_count > 0 && !this->flag_publisher_working_);

    // Unlock.
    this->pub_mtx_.unlock();

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
    if(this->publisher_socket_)
    {
        delete this->publisher_socket_;
        this->publisher_socket_ = nullptr;
    }
}

void PublisherBase::internalStopPublisher()
{
    // If server is already stopped, do nothing.
    if (!this->flag_publisher_working_)
        return;

    // Stop the worker thread.
    this->stop_queue_worker_ = true;
    this->queue_cv_.notify_all();
    if (this->queue_worker_th_.joinable())
        this->queue_worker_th_.join();

    // Set the shared working flag to false (is atomic).
    this->flag_publisher_working_ = false;

    // Delete the sockets.
    this->deleteSockets();
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
    //multipart_msg.add(std::move(msg_info));

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
