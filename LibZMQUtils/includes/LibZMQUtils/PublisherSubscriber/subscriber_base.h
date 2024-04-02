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
 * @file subscriber_base.h
 * @brief This file contains the declaration of the SubscriberBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <future>
#include <string>
#include <map>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Global/zmq_context_handler.h"
#include "LibZMQUtils/PublisherSubscriber/common.h"
#include "LibZMQUtils/InternalHelpers/network_helpers.h"
#include "LibZMQUtils/Utilities/uuid_generator.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

// SUBSCRIBER CONSTANTS
// =====================================================================================================================
constexpr std::string_view kReservedExitTopic = "quit";
// =====================================================================================================================

// =====================================================================================================================
using common::ServerResultStr;
using common::SubscriberResult;
using internal_helpers::network::NetworkAdapterInfo;
using utils::UUID;
// =====================================================================================================================

/**
 * @class SubscriberBase
 *
 * @brief This class provides the base structure for a ZeroMQ based subscriber. The subscriber can subscribe to multiple
 * publishers and use topic filtering. There is only one topic word reserved for closing the subscriber. This topic
 * cannot be used by the user, since it is issued internally by the class.
 *
 */
class SubscriberBase : public ZMQContextHandler
{

public:

    /**
     * @brief Base constructor for SubscriberBase
     */
    LIBZMQUTILS_EXPORT SubscriberBase();


    /**
     * @brief Get the topic filters that are allowed in this subscriber at a specific time. If there is no topic, then
     * no message will be received by the subscriber.
     * @return the set of topic filters applied.
     */
    LIBZMQUTILS_EXPORT const std::set<common::TopicType>& getTopicFilters() const;


    /**
     * @brief Get the future associated with the worker thread.
     *
     * This function returns a const reference to a std::future<void> object representing the asynchronous
     * worker thread that is running the subscriber event loop. This object can be used to check the status of
     * the worker thread or wait for it to complete.
     *
     * @return A const reference to the worker thread future.
     */
    LIBZMQUTILS_EXPORT const std::future<void>& getWorkerFuture() const;

    /**
     * @brief Get a const reference to the map of subscribed publishers.
     *
     * @return A const reference to the map of subscribed publishers.
     */
    LIBZMQUTILS_EXPORT const std::map<UUID, common::PublisherInfo>& getSubscribedPublishers() const;

    /**
     * @brief Check if the worker thread is currently active.
     *
     * This function returns a boolean value indicating whether the worker thread is
     * currently active. If the worker is active, it means that it is processing incoming messages.
     *
     * @return True if the worker is active, false otherwise.
     */
    LIBZMQUTILS_EXPORT bool isWorking() const;

    /**
     * @brief Starts the subscriber worker thread.
     *
     * If the worker is already active, this function does nothing. Otherwise, it creates the ZMQ
     * context if it doesn't exist and launches the worker in a separate thread.
     *
     * @return True if the worker was successfully started, false otherwise.
     */
    LIBZMQUTILS_EXPORT bool startSubscriber();

    /**
     * @brief Stops the subscriber worker thread.
     *
     * If the worker is already stopped, this function does nothing. Otherwise, it
     * deletes the ZMQ context and cleans up the subscribed publishers and topic filters.
     */
    LIBZMQUTILS_EXPORT void stopSubscriber();

    /**
     * @brief Subscribe to a publisher defined by its endpoint.
     * @param pub_endpoint, the endpoint URL of the publisher to subscribe.
     */
    LIBZMQUTILS_EXPORT void subscribe(const std::string &pub_endpoint);

    /**
     * @brief Unsubscribe to a publisher defined by its endpoint.
     * @param pub_endpoint, the endpoint URL of the publisher to unsubscribe.
     */
    LIBZMQUTILS_EXPORT void unsubscribe(const std::string &pub_endpoint);

    /**
     * @brief Adds a topic filter for incoming messages. Empty topic means everything, while no topic means nothing.
     * Reserved exit topic cannot be issued to this function. It will be discarded.
     * @param filter, the topic filter to add.
     */
    LIBZMQUTILS_EXPORT void addTopicFilter(const common::TopicType &filter);

    /**
     * @brief Removes a topic filter for incoming messages.
     * Reserved exit topic cannot be issued to this function. It will be discarded.
     * @param filter, the filter to remove.
     */
    LIBZMQUTILS_EXPORT void removeTopicFilter(const common::TopicType &filter);

    /**
     * @brief Removes every topic filter for incoming messages. This way, no message will be allowed.
     */
    LIBZMQUTILS_EXPORT void cleanTopicFilters();

    /**
     * @brief Virtual destructor.
     *
     * This destructor is virtual to ensure proper cleanup when the derived class is destroyed.
     */
    LIBZMQUTILS_EXPORT virtual ~SubscriberBase() override;

protected:

    // -----------------------------------------------------------------------------------------------------------------
    using ProcessFunction = std::function<void(const common::PubSubMsg&)>;         ///< Process function alias.
    using ProcessFunctionsMap = std::unordered_map<common::TopicType, ProcessFunction>;  ///< Process function map alias.
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
    void registerRequestProcFunc(const common::TopicType &topic, ClassT* obj,
                                 void(ClassT::*func)(const common::PubSubMsg&))
    {
        this->process_fnc_map_[topic] = [obj, func](const common::PubSubMsg& msg)
        {
            (obj->*func)(msg);
        };
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
    LIBZMQUTILS_EXPORT virtual void onSubscriberStart() = 0;

    /**
     * @brief Base subscriber stop callback. Subclasses must override this function.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          subscriber's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the subscriber's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    LIBZMQUTILS_EXPORT virtual void onSubscriberStop() = 0;

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
    LIBZMQUTILS_EXPORT virtual void onInvalidMsgReceived(const common::PubSubMsg&) = 0;

    /**
     * @brief Base message received callback. Subclasses must override this function.
     *
     * @param The PubSubMsg object representing the msg received.
     *
     * @warning The overrided callback must be non-blocking and have minimal computation time. Blocking or
     *          computationally intensive operations within internal callbacks can significantly affect the
     *          subscriber's performance and responsiveness. If complex tasks are required, it is recommended to
     *          perform them asynchronously to avoid blocking the subscriber's main thread. Consider using separate
     *          threads or asynchronous mechanisms to handle time-consuming tasks.
     */
    LIBZMQUTILS_EXPORT virtual void onMsgReceived(const common::PubSubMsg&) = 0;

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
    LIBZMQUTILS_EXPORT virtual void onSubscriberError(const zmq::error_t &error, const std::string& ext_info = "") = 0;

private:

    // Internal helper for stopping the subscriber.
    void internalStopSubscriber();

    // Subscriber worker (will be executed asynchronously).
    void startWorker();

    // Function for receiving data from the socket.
    SubscriberResult recvFromSocket(common::PubSubMsg&);

    // Function for resetting the socket.
    void resetSocket();


    // -----------------------------------------------------

    // ZMQ socket.
    zmq::socket_t* socket_;              ///< ZMQ subscriber socket.
    zmq::socket_t* socket_pub_close_;    ///< ZMQ close publisher socket.
    UUID socket_close_uuid_;             ///< UUID for close publisher.

    // Subscruiber uuid
    UUID sub_uuid_;

    // Mutex.
    mutable std::mutex mtx_;        ///< Safety mutex.
    mutable std::mutex depl_mtx_;   ///< Worker deploy mutex.

    // Future and condition variable for the worker.
    std::future<void> fut_worker_;     ///< Future that stores the worker status.
    std::condition_variable cv_worker_depl_;  ///< Condition variable to notify the deployment status of the worker.

    // Clients container.
    std::map<UUID, common::PublisherInfo> subscribed_publishers_;   ///< Dictionary with the connected clients.

    std::set<common::TopicType> topic_filters_; ///< Set of topics allowed on this publisher.

    // Process functions container.
    ProcessFunctionsMap process_fnc_map_;        ///< Container with the internal factory process function.

    // Useful flags.
    std::atomic_bool flag_working_;       ///< Flag for check the worker active status.

};

} // END NAMESPACES.
// =====================================================================================================================
