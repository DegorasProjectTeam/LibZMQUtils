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
 * @file subscriber_base.h
 * @brief This file contains the declaration of the SubscriberBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <future>
#include <string>
#include <map>
#include <shared_mutex>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Global/zmq_context_handler.h"
#include "LibZMQUtils/PublisherSubscriber/data/publisher_subscriber_data.h"
#include "LibZMQUtils/PublisherSubscriber/data/publisher_subscriber_info.h"
#include "LibZMQUtils/Utilities/uuid_generator.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

// SUBSCRIBER CONSTANTS
// =====================================================================================================================
constexpr std::string_view kReservedExitTopic = "quit";
// =====================================================================================================================

/**
 * @class SubscriberBase
 *
 * @brief This class provides the base structure for a ZeroMQ based subscriber. The subscriber can subscribe to multiple
 * publishers and use topic filtering. There is only one topic word reserved for closing the subscriber. This topic
 * cannot be used by the user, since it is issued internally by the class.
 *
 */
class LIBZMQUTILS_EXPORT SubscriberBase : public ZMQContextHandler
{

public:

    /**
     * @brief Base constructor for SubscriberBase
     */
    SubscriberBase(const std::string& subscriber_name = "",
                   const std::string& subscriber_version = "",
                   const std::string& subscriber_info = "");

    /**
     * @brief Get the topic filters that are allowed in this subscriber at a specific time. If there is no topic, then
     * no message will be received by the subscriber.
     * @return the set of topic filters applied.
     */
    const std::set<TopicType>& getTopicFilters() const;

    /**
     * @brief Get the future associated with the worker thread.
     *
     * This function returns a const reference to a std::future<void> object representing the asynchronous
     * worker thread that is running the subscriber event loop. This object can be used to check the status of
     * the worker thread or wait for it to complete.
     *
     * @return A const reference to the worker thread future.
     */
    const std::future<void>& getWorkerFuture() const;

    /**
     * @brief Get a const reference to the map of subscribed publishers.
     *
     * @return A const reference to the map of subscribed publishers.
     */
    const std::map<utils::UUID, PublisherInfo>& getSubscribedPublishers() const;


    /**
     * @brief Get all the subscriber information.
     * @return A const reference to the SubscriberInfo struct that contains all the subscriber information.
     */
    const SubscriberInfo &getSubscriberInfo() const;

    /**
     * @brief Check if the worker thread is currently active.
     *
     * This function returns a boolean value indicating whether the worker thread is
     * currently active. If the worker is active, it means that it is processing incoming messages.
     *
     * @return True if the worker is active, false otherwise.
     */
    bool isWorking() const;

    /**
     * @brief Starts the subscriber worker thread.
     *
     * If the worker is already active, this function does nothing. Otherwise, it creates the ZMQ
     * context if it doesn't exist and launches the worker in a separate thread.
     *
     * @return True if the worker was successfully started, false otherwise.
     */
    bool startSubscriber();

    /**
     * @brief Stops the subscriber worker thread.
     *
     * If the worker is already stopped, this function does nothing. Otherwise, it
     * deletes the ZMQ context and cleans up the subscribed publishers and topic filters.
     */
    void stopSubscriber();

    /**
     * @brief Subscribe to a publisher defined by its endpoint.
     * @param pub_endpoint, the endpoint URL of the publisher to subscribe.
     */
    void subscribe(const std::string &pub_endpoint);

    /**
     * @brief Unsubscribe to a publisher defined by its endpoint.
     * @param pub_endpoint, the endpoint URL of the publisher to unsubscribe.
     */
    void unsubscribe(const std::string &pub_endpoint);

    /**
     * @brief Adds a topic filter for incoming messages. Empty topic means everything, while no topic means nothing.
     * Reserved exit topic cannot be issued to this function. It will be discarded.
     * @param filter, the topic filter to add.
     */
    void addTopicFilter(const TopicType &filter);

    /**
     * @brief Removes a topic filter for incoming messages.
     * Reserved exit topic cannot be issued to this function. It will be discarded.
     * @param filter, the filter to remove.
     */
    void removeTopicFilter(const TopicType &filter);

    /**
     * @brief Removes every topic filter for incoming messages. This way, no message will be allowed.
     */
    void cleanTopicFilters();

    static std::string operationResultToString(OperationResult result);

    static std::string operationResultToString(ResultType result);

    /**
     * @brief Virtual destructor.
     *
     * This destructor is virtual to ensure proper cleanup when the derived class is destroyed.
     */
    virtual ~SubscriberBase() override;

protected:

    // -----------------------------------------------------------------------------------------------------------------
    using ProcessFunction = std::function<void(const PublishedMessage&)>;        ///< Process function alias.
    using ProcessFunctionsMap = std::unordered_map<TopicType, ProcessFunction>;  ///< Process function map alias.
    // -----------------------------------------------------------------------------------------------------------------

    /**
     * @brief Register a function to process a `PubSubMsg` object.
     *
     * @tparam ClassT The class type of the object that contains the member function to be called.
     *
     * @param topic The topic that the function will process replies for.
     * @param obj A pointer to the object that contains the member function to be called.
     * @param func The member function to call when the msg is received.
     *
     */
    template <typename ClassT>
    void registerRequestProcFunc(const TopicType &topic, ClassT* obj,
                                 void(ClassT::*func)(const PublishedMessage&))
    {
        this->process_fnc_map_[topic] = [obj, func](const PublishedMessage& msg)
        {
            return (obj->*func)(msg);
        };
    }

    /**
     * @brief Register a function to process a `PubSubMsg` object.
     *
     * @param topic The topic that the function will process replies for.
     * @param func The member function to call when the msg is received.
     *
     */
    void registerRequestProcFunc(const TopicType &topic, std::function<void(const PublishedMessage&)> func)
    {
        this->process_fnc_map_[topic] = func;
    }

    /**
     * @brief Base subscriber start callback. Subclasses must override this function.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          subscriber's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the subscriber's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onSubscriberStart() = 0;

    /**
     * @brief Base subscriber stop callback. Subclasses must override this function.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          subscriber's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the subscriber's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onSubscriberStop() = 0;

    /**
     * @brief Base invalid message received callback. Subclasses must override this function.
     *
     * @param The PubSubMsg object representing the invalid message received.
     *
     * @warning The overridden callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          subscriber's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the subscriber's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onInvalidMsgReceived(const PublishedMessage&, OperationResult res) = 0;

    /**
     * @brief Base message received callback. Subclasses may override this function.
     *
     * @param The PubSubMsg object representing the msg received.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          subscriber's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the subscriber's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onMsgReceived(const PublishedMessage&, OperationResult res);

    /**
     * @brief Base subscriber error callback. Subclasses must override this function.
     *
     * @param The `zmq::error_t` object representing the error that occurred.
     *
     * @param Optional additional information or context related to the error. It is an empty string by default.
     *
     * @note The `zmq::error_t` class provides information about ZeroMQ errors. You can access the error code,
     *       description, and other details using the methods provided by `zmq::error_t`.
     *
     * @warning If this function is not overridden in subclasses, it will not handle subscriber errors, and errors may not
     *          be handled properly.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          subscriber's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the subscriber's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    virtual void onSubscriberError(const zmq::error_t &error, const std::string& ext_info = "") = 0;

private:

    // Internal helper for stopping the subscriber.
    void internalStopSubscriber();

    // Subscriber worker (will be executed asynchronously).
    void subscriberWorker();

    // Function for receiving data from the socket.
    OperationResult recvFromSocket(PublishedMessage&);

    // Function for resetting the socket.
    void resetSocket();

    // -----------------------------------------------------

    // ZMQ socket.
    zmq::socket_t* socket_;              ///< ZMQ subscriber socket.
    zmq::socket_t* socket_pub_close_;    ///< ZMQ close publisher socket.
    utils::UUID socket_close_uuid_;      ///< UUID for close publisher.

    // Subscriber info
    SubscriberInfo sub_info_;

    // Mutex.
    mutable std::shared_mutex sub_mtx_;  ///< Safety mutex.
    mutable std::mutex depl_mtx_;        ///< Worker deploy mutex.

    // Future and condition variable for the worker.
    std::future<void> fut_worker_;            ///< Future that stores the worker status.
    std::condition_variable cv_worker_depl_;  ///< Condition variable to notify the deployment status of the worker.

    // Clients container.
    std::map<utils::UUID, PublisherInfo> subscribed_publishers_;   ///< Dictionary with the connected clients.

    std::set<TopicType> topic_filters_; ///< Set of topics allowed on this publisher.

    // Process functions container.
    ProcessFunctionsMap process_fnc_map_;        ///< Container with the internal factory process function.

    // Useful flags.
    std::atomic_bool flag_working_;       ///< Flag for check the worker active status.

    /// Specific class scope (for debug purposes).
    inline static const std::string kScope = "[LibZMQUtils,PublisherSubscriber,SubscriberBase]";
};

}} // END NAMESPACES.
// =====================================================================================================================
