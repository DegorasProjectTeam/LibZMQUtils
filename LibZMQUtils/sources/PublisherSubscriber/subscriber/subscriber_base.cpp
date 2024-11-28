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
 * @file subscriber_base.cpp
 * @brief This file contains the implementation of the SubscriberBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <shared_mutex>
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
#include "LibZMQUtils/PublisherSubscriber/subscriber/subscriber_base.h"
#include "LibZMQUtils/Global/constants.h"
#include "LibZMQUtils/InternalHelpers/network_helpers.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
#include "LibZMQUtils/Utilities/utils.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

SubscriberBase::SubscriberBase(const std::string& subscriber_name,
                               const std::string& subscriber_version ,
                               const std::string& subscriber_info) :
    socket_(nullptr),
    socket_pub_close_(nullptr),
    flag_working_(false)
{
    // Get the client interfaces.
    std::vector<internal_helpers::network::NetworkAdapterInfo> interfcs =
        internal_helpers::network::getHostIPsWithInterfaces();

    // Check if we have active interfaces.
    if(interfcs.empty())
        throw std::invalid_argument(this->kScope + " No active network interfaces found.");

    // Update the subscriber info.
    this->sub_info_.uuid = utils::UUIDGenerator::getInstance().generateUUIDv4();
    this->sub_info_.hostname = internal_helpers::network::getHostname();
    this->sub_info_.name = subscriber_name;
    this->sub_info_.version = subscriber_version;
    this->sub_info_.info = subscriber_info;
}

const std::set<TopicType> &SubscriberBase::getTopicFilters() const
{
    return this->topic_filters_;
}

const std::future<void> &SubscriberBase::getWorkerFuture() const
{
    return this->fut_worker_;
}

const std::map<utils::UUID, PublisherInfo> &SubscriberBase::getSubscribedPublishers() const
{
    return this->subscribed_publishers_;
}

const SubscriberInfo &SubscriberBase::getSubscriberInfo() const
{
    return this->sub_info_;
}

bool SubscriberBase::isWorking() const
{
    return this->flag_working_;
}

bool SubscriberBase::startSubscriber()
{
    // Safe mutex lock
    std::unique_lock<std::shared_mutex> lock(this->sub_mtx_);

    // If worker is already started, do nothing
    if (this->flag_working_)
        return true;

    // Launch worker in other thread.
    this->fut_worker_ = std::async(std::launch::async, &SubscriberBase::subscriberWorker, this);

    // Wait for the worker deployment.
    std::unique_lock<std::mutex> depl_lock(this->depl_mtx_);
    this->cv_worker_depl_.wait(depl_lock);

    // Return the worker status.
    return this->flag_working_;
}

void SubscriberBase::stopSubscriber()
{
    // Safe mutex lock
    std::unique_lock<std::shared_mutex> lock(this->sub_mtx_);

    // Call to the internal stop.
    this->internalStopSubscriber();

    // Clean the subscribers.
    this->subscribed_publishers_.clear();

    // Clean the topics.
    this->topic_filters_.clear();

    // Call to the internal callback.
    this->onSubscriberStop();
}

void SubscriberBase::subscribe(const std::string& pub_endpoint)
{
    // Check the publisher endpoint.
    if(pub_endpoint.empty())
        throw std::invalid_argument(this->kScope + " The publisher endpoint can't be empty.");

    // Check if endpoint is already subscribed
    auto it = std::find_if(this->subscribed_publishers_.begin(), this->subscribed_publishers_.end(),
                          [&pub_endpoint](const auto& pair)
    {
        return pair.second.endpoint == pub_endpoint;
    });

    // If endpoint is not subscribed, then store information.
    if (it == this->subscribed_publishers_.end())
    {
        // TODO ¿De que sirve aqui usar un uuid inventado? pub-sub abstrae de esa parte.
        // Tal vez seria interesante actualizar la información a partir de los mensajes recibidos.

        unsigned port = static_cast<unsigned>(std::stoi(pub_endpoint.substr(pub_endpoint.rfind(':') + 1)));
        PublisherInfo pub_info(utils::UUID(), port, pub_endpoint);
        this->subscribed_publishers_.insert({pub_info.uuid, pub_info});

        // If socket is started, then reset to apply the change.
        if (this->flag_working_)
            this->resetSocket();
    }
}

void SubscriberBase::unsubscribe(const std::string &pub_endpoint)
{
    // Check the publisher endpoint.
    if(pub_endpoint.empty())
        return;

    // Check if endpoint is subscribed
    auto it = std::find_if(this->subscribed_publishers_.begin(), this->subscribed_publishers_.end(),
    [&pub_endpoint](const auto& pair)
    {
        return pair.second.endpoint == pub_endpoint;
    });

    if (it != this->subscribed_publishers_.end())
    {
        // If endpoint is subscribed, erase it.
        this->subscribed_publishers_.erase(it);
        // If socket is started, then reset to apply the change.
        if (this->flag_working_)
            this->resetSocket();
    }
}

void SubscriberBase::addTopicFilter(const TopicType &filter)
{
    // Avoid reserved topic
    if (filter != kReservedTopicExit)
    {
        auto res = this->topic_filters_.insert(filter);
        // If filter was applied, reset socket to apply the change if it is working.
        if (res.second && this->flag_working_)
            this->resetSocket();
    }
}

void SubscriberBase::removeTopicFilter(const TopicType &filter)
{
    // Avoid reserved topic
    if (filter != kReservedTopicExit)
    {
        auto res = this->topic_filters_.erase(filter);
        // If filter was applied, reset socket to apply the change if it is working.
        if (res > 0 && this->flag_working_)
            this->resetSocket();
    }

}

std::string SubscriberBase::operationResultToString(OperationResult result)
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

std::string SubscriberBase::operationResultToString(ResultType result)
{
    return SubscriberBase::operationResultToString(static_cast<OperationResult>(result));
}

void SubscriberBase::internalStopSubscriber()
{
    // If worker is already stopped, do nothing.
    if (!this->flag_working_)
        return;

    // Set the shared working flag to false (is atomic).
    this->flag_working_ = false;

    // If the worker is active.
    if(this->fut_worker_.valid() &&
        this->fut_worker_.wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
    {
        // Message for closing.

        // Serializer.
        serializer::BinarySerializer serializer;

        // Prepare the topic. This must come plain, since it is used by ZMQ topic filtering.
        zmq::message_t msg_topic{TopicType(kReservedTopicExit)};

        // Prepare the close socket uuid.
        size_t uuid_size = serializer.write(this->socket_close_uuid_.getBytes());
        zmq::message_t msg_uuid(serializer.release(), uuid_size);

        // Prepare the timestamp.
        size_t ts_size = serializer.write(utils::currentISO8601Date(true, false, true));
        zmq::message_t msg_ts(serializer.release(), ts_size);

        // Information is empty.
        zmq::message_t msg_info;

        // Prepare the multipart msg.
        zmq::multipart_t multipart_msg;
        multipart_msg.add(std::move(msg_topic));
        multipart_msg.add(std::move(msg_uuid));
        multipart_msg.add(std::move(msg_ts));
        multipart_msg.add(std::move(msg_info));

        // Send the message.
        multipart_msg.send(*this->socket_pub_close_);

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
    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

SubscriberBase::~SubscriberBase()
{
    // Stop the publisher.
    // Warning: In this case the onSubscriberStop callback can't be executed.
    std::unique_lock<std::shared_mutex> lock(this->sub_mtx_);
    this->internalStopSubscriber();
    this->subscribed_publishers_.clear();
    this->topic_filters_.clear();
}

void SubscriberBase::subscriberWorker()
{
    // Containers.
    OperationResult result;
    PublishedMessage msg;

    // Start subscriber socket
    this->resetSocket();

    // Worker loop.
    while(this->flag_working_ && this->socket_)
    {
        // Receive the data.
        result = this->recvFromSocket(msg);

        // Process the data.
        if(result == OperationResult::OPERATION_OK && !this->flag_working_)
        {
            // In this case, we will close the subscriber.
        }
        else if (result != OperationResult::OPERATION_OK)
        {
            // Internal callback.
            this->onInvalidMsgReceived(msg, result);
        }
        else if (result == OperationResult::OPERATION_OK)
        {
            // Fin the functor.
            auto iter = process_fnc_map_.find(msg.topic);

            // Update the result value.
            result = (iter == process_fnc_map_.end()) ? OperationResult::NOT_IMPLEMENTED : OperationResult::OPERATION_OK;

            // Call callback for msg received.
            this->onMsgReceived(msg, result);

            // Invoke the function if implemented.
            if(iter != process_fnc_map_.end())
                iter->second(msg);
        }
    } // Finish the worker.
}

OperationResult SubscriberBase::recvFromSocket(PublishedMessage& msg)
{
    // Result variable.
    OperationResult result = OperationResult::OPERATION_OK;

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
        // Check if we want to close the subscriber.
        // The error code is for ZMQ EFSM error.
        if(error.num() == kZmqEFSMError && !this->flag_working_)
            return OperationResult::OPERATION_OK;

        // Else, call to error callback.
        this->onSubscriberError(error, this->kScope + " Error while receiving a request.");
        return OperationResult::INTERNAL_ZMQ_ERROR;
    }

    // Check for empty msg or timeout reached.
    if (multipart_msg.empty())
        return OperationResult::EMPTY_MSG;

    // Check the multipart msg size.
    if (recv_result && (multipart_msg.size() == 4 || multipart_msg.size() == 5))
    {
        // Get the multipart data.
        zmq::message_t msg_topic = multipart_msg.pop();
        zmq::message_t msg_uuid = multipart_msg.pop();
        zmq::message_t msg_time = multipart_msg.pop();

        // Get the topic. Topic is not serialized using BinarySerializer, since it must come plain.
        msg.topic = msg_topic.to_string();

        // Get the publisher uuid data.
        if (msg_uuid.size() == utils::UUID::kUUIDSize + sizeof(serializer::SizeUnit)*2)
        {
            std::array<std::byte, 16> uuid_bytes;
            serializer::BinarySerializer::fastDeserialization(msg_uuid.data(), msg_uuid.size(), uuid_bytes);
            msg.publisher_uuid = utils::UUID(uuid_bytes);
        }
        else
            return OperationResult::INVALID_PUB_UUID;

        // If exit topic was issued, check if uuid matches the close publisher and return.
        // No more info is necessary.
        if (kReservedTopicExit == msg.topic)
        {
            if (this->socket_close_uuid_ == msg.publisher_uuid)
                return OperationResult::OPERATION_OK;
            else
                return OperationResult::INVALID_PARTS;
        }

        // Get the timestamp.
        serializer::BinarySerializer::fastDeserialization(msg_time.data(), msg_time.size(), msg.timestamp);

        // TODO WARNING: WE CANT UPDATE THE STORED INFO BECAUSE IN ZMQ YOU CANT KNOW WHAT PUBLISHER SENDS THE MSG. IN
        // THIS CASE MAYBE YOU CAN PUBLISH A PUBLISHER INFORMATION TOPIC FOR ASSOCIATE THE UUID WITH SPECIFIC
        // PUBLISHER INFORMATION IN THE PUBLISHERS MAP.

        // If there is still one more part, it is the message data.
        if (multipart_msg.size() == 1)
        {
            // Get the message and the size.
            zmq::message_t message_data = multipart_msg.pop();

            // Check the parameters.
            if(message_data.size() > 0)
            {
                // Get and store the parameters data.
                serializer::BinarySerializer serializer(message_data.data(), message_data.size());
                msg.data.size = serializer.moveUnique(msg.data.bytes);
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
        auto close_endpoint = "inproc://" + this->sub_info_.uuid.toRFC4122String();
        this->socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::sub);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        this->socket_->set(zmq::sockopt::linger, 0);
        this->socket_->connect(close_endpoint);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        this->socket_->set(zmq::sockopt::subscribe, kReservedTopicExit);

        // Connect to subscribed publishers
        for (const auto& publishers : this->subscribed_publishers_)
        {
            this->socket_->connect(publishers.second.endpoint);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Set all topic filters
        for (const auto& topic: this->topic_filters_)
        {
            this->socket_->set(zmq::sockopt::subscribe, topic);
        }

        this->socket_pub_close_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::pub);
        this->socket_pub_close_->bind(close_endpoint);
        this->socket_pub_close_->set(zmq::sockopt::linger, 0);

        this->socket_close_uuid_ = utils::UUIDGenerator::getInstance().generateUUIDv4();

        // Update the working flag and calls to the callback.
        this->onSubscriberStart();
        this->flag_working_ = true;
        this->cv_worker_depl_.notify_all();
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
        this->onSubscriberError(*last_error, this->kScope + " Error during socket creation.");
        this->flag_working_ = false;
        this->cv_worker_depl_.notify_all();
        return;
    }
}

void SubscriberBase::onMsgReceived(const PublishedMessage&, OperationResult)
{}

}} // END NAMESPACES.
// =====================================================================================================================
