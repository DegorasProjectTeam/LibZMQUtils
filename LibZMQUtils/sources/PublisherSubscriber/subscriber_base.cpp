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
#include "LibZMQUtils/Global/constants.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

SubscriberBase::SubscriberBase() :
    socket_(nullptr),
    socket_pub_close_(nullptr),
    sub_uuid_(utils::UUIDGenerator::getInstance().generateUUIDv4()),
    flag_working_(false)

{

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

bool SubscriberBase::isWorking() const
{
    return this->flag_working_;
}

bool SubscriberBase::startSubscriber()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // If worker is already started, do nothing
    if (this->flag_working_)
        return true;

    // Launch worker in other thread.
    this->fut_worker_ = std::async(std::launch::async, &SubscriberBase::startWorker, this);

    // Wait for the worker deployment.
    std::unique_lock<std::mutex> depl_lock(this->depl_mtx_);
    this->cv_worker_depl_.wait(depl_lock);

    // Return the worker status.
    return this->flag_working_;
}

void SubscriberBase::stopSubscriber()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Call to the internal stop.
    this->internalStopSubscriber();

    // Clean the subscribers.
    this->subscribed_publishers_.clear();

    // Clean the topics.
    this->topic_filters_.clear();
}

void SubscriberBase::subscribe(const std::string &pub_endpoint)
{
    // Check if endpoint is already subscribed
    auto it = std::find_if(this->subscribed_publishers_.begin(), this->subscribed_publishers_.end(),
                          [&pub_endpoint](const auto& pair)
    {
        return pair.second.endpoint == pub_endpoint;
    });
    if (it == this->subscribed_publishers_.end())
    {
        // If endpoint is not subscribed, then store information.
        PublisherInfo pub_info(utils::UUIDGenerator::getInstance().generateUUIDv4(), pub_endpoint);
        this->subscribed_publishers_.insert({pub_info.uuid, pub_info});
        // If socket is started, then reset to apply the change.
        if (this->flag_working_)
            this->resetSocket();
    }

}

void SubscriberBase::unsubscribe(const std::string &pub_endpoint)
{
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
    if (filter != kReservedExitTopic)
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
    if (filter != kReservedExitTopic)
    {
        auto res = this->topic_filters_.erase(filter);
        // If filter was applied, reset socket to apply the change if it is working.
        if (res > 0 && this->flag_working_)
            this->resetSocket();
    }

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
        zmq::multipart_t msg;
        msg.addstr(TopicType(kReservedExitTopic));
        msg.addstr("close_pub");
        msg.addstr(this->socket_close_uuid_.toRFC4122String());
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

}

SubscriberBase::~SubscriberBase()
{
    this->internalStopSubscriber();
}

SubscriberResult SubscriberBase::onMsgReceived(const PubSubMsg &msg)
{
    auto iter = process_fnc_map_.find(msg.data.topic);
    if(iter != process_fnc_map_.end())
    {
        // Invoke the function.
        return iter->second(msg);
    }
    else
    {
        // Command not found in the map.
        return SubscriberResult::NOT_IMPLEMENTED;
    }
}


void SubscriberBase::startWorker()
{
    // Containers.
    SubscriberResult result;
    PubSubMsg msg;

    // Start subscriber socket
    this->resetSocket();

    // Worker loop.
    while(this->socket_ && this->flag_working_)
    {
        // Receive the data.
        result = this->recvFromSocket(msg);

        // Process the data.
        if(result == SubscriberResult::MSG_OK && !this->flag_working_)
        {
            // In this case, we will close the subscriber. Call to the internal callback.
            this->onSubscriberStop();
        }
        else if (result != SubscriberResult::MSG_OK)
        {
            // Internal callback.
            this->onInvalidMsgReceived(msg, result);
        }
        else if (result == SubscriberResult::MSG_OK)
        {
            // Call callback for msg received.
            result = this->onMsgReceived(msg);

            // If processing was not succesful, call invalid msg callback.
            if (result != SubscriberResult::MSG_OK)
                this->onInvalidMsgReceived(msg, result);
        }
    }
    // Finish the worker.
}

SubscriberResult SubscriberBase::recvFromSocket(PubSubMsg& msg)
{
    // Result variable.
    SubscriberResult result = SubscriberResult::MSG_OK;

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
            return SubscriberResult::MSG_OK;

        // Else, call to error callback.
        this->onSubscriberError(error, "SubscriberBase: Error while receiving a request.");
        return SubscriberResult::INTERNAL_ZMQ_ERROR;
    }

    // Check for empty msg or timeout reached.
    if (multipart_msg.empty())
        return SubscriberResult::EMPTY_MSG;

    // Check the multipart msg size.
    if (recv_result && (multipart_msg.size() == 3 || multipart_msg.size() == 4))
    {
        // Get the multipart data.
        zmq::message_t msg_topic = multipart_msg.pop();
        zmq::message_t msg_pub_name = multipart_msg.pop();
        zmq::message_t msg_uuid = multipart_msg.pop();

        // Get the topic. Topic is not serialized using BinarySerializer, since it must come plain.
        msg.data.topic = msg_topic.to_string();

        // Get the publisher name.
        utils::BinarySerializer::fastDeserialization(
            msg_pub_name.data(), msg_pub_name.size(), msg.pub_info.name);

        // Get the publisher uuid data.
        if (msg_uuid.size() == utils::UUID::kUUIDSize + sizeof(utils::SizeUnit)*2)
        {
            std::array<std::byte, 16> uuid_bytes;
            utils::BinarySerializer::fastDeserialization(msg_uuid.data(), msg_uuid.size(), uuid_bytes);
            msg.pub_info.uuid = utils::UUID(uuid_bytes);
        }
        else
            return SubscriberResult::INVALID_PUB_UUID;

        // If exit topic was issued, check if uuid matches the close publisher.
        if (kReservedExitTopic == msg.data.topic)
        {
            if (this->socket_close_uuid_ == msg.pub_info.uuid)
                return SubscriberResult::MSG_OK;
            else
                return SubscriberResult::INVALID_PARTS;
        }


        // If there is still one more part, it is the message data.
        if (multipart_msg.size() == 1)
        {
            // Get the message and the size.
            zmq::message_t message_data = multipart_msg.pop();

            // Check the parameters.
            if(message_data.size() > 0)
            {
                // Get and store the parameters data.
                utils::BinarySerializer serializer(message_data.data(), message_data.size());
                msg.data.data_size = serializer.moveUnique(msg.data.data);
            }
            else
                return SubscriberResult::EMPTY_PARAMS;
        }
    }
    else
        return SubscriberResult::INVALID_PARTS;

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
        this->socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::sub);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        this->socket_->set(zmq::sockopt::linger, 0);
        this->socket_->connect(close_endpoint);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        this->socket_->set(zmq::sockopt::subscribe, kReservedExitTopic);
        // Connect to subscribed publishers
        for (const auto& publishers : this->subscribed_publishers_)
        {
            this->socket_->connect(publishers.second.endpoint);
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
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
        this->onSubscriberError(*last_error, "Error during socket creation.");
        this->flag_working_ = false;
        this->cv_worker_depl_.notify_all();
        return;

    }
}

}} // END NAMESPACES.
// =====================================================================================================================
