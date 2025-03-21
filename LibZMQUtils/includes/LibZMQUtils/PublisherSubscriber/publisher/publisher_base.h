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
 * @file publisher_base.h
 * @brief This file contains the declaration of the PublisherBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <condition_variable>
#include <queue>
#include <string>
#include <atomic>
#include <shared_mutex>
#include <thread>
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Global/zmq_context_handler.h"
#include "LibZMQUtils/PublisherSubscriber/data/publisher_subscriber_data.h"
#include "LibZMQUtils/PublisherSubscriber/data/publisher_subscriber_info.h"
#include "LibZMQUtils/InternalHelpers/network_helpers.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================


class LIBZMQUTILS_EXPORT PublisherBase : public ZMQContextHandler
{

public:
    
    /**
     * @brief Constructs a ZeroMQ-based publisher with specific parameters.
     *
     * This constructor initializes a ZeroMQ-based publisher, setting the publisher_port for listening, the IP address for
     * binding connections, and other metadata such as the publisher name, version, and additional information.
     *
     * @param publisher_port    The publisher_port number on which the publisher will listen for incoming connections.
     * @param publisher_iface   The interface address on which the publisher will accept connections. By default, it listens
     *                          on all available interfaces ("*").
     * @param publisher_name    Optional parameter to specify the publisher name. By default is empty.
     * @param publisher_version Optional parameter to specify the publisher version (like "1.1.1"). By default is empty.
     * @param publisher_info    Optional parameter to specify the publisher information. By default is empty.
     *
     * @throws std::invalid_argument If no network interfaces matching the specified IP address are found.
     *
     * @note The publisher requires at least one valid IP address to function properly. If "ip_address" is set to "*",
     * it will listen on all available local interfaces. Otherwise, the publisher will only bind to the specified IP
     * address if it matches a valid interface.
     *
     * @warning When specifying the `ip_address`, ensure it is a valid IP address present on the system. Incorrect or
     * unavailable addresses may result in connection failures.
     */
    PublisherBase(unsigned publisher_port,
                  const std::string& publisher_iface = "*",
                  const std::string& publisher_name = "",
                  const std::string& publisher_version = "",
                  const std::string& publisher_info = "");
    
    /**
     * @brief Start the publisher so it can send messages. It must be started before sending messages.
     * @return true if it was started successfully. False otherwise.
     */
    bool startPublisher();

    /**
     * @brief Stops the publisher and cleans the socket. Messages cannot be sent until publisher is started again.
     */
    void stopPublisher();

    /**
     * @brief Restarts the publisher.
     * @return true if reset was successful, false otherwise.
     */
    bool resetPublisher();

    /**
     * @brief Get the endpoint that this publisher is bound to.
     * @return the URL of the endpoint that this publisher is bound to.
     */
    const std::string& getEndpoint() const;

    /**
     * @brief Get all the publisher information.
     * @return A const reference to the PublisherInfo struct that contains all the publisher information.
     */
    const PublisherInfo& getPublisherInfo() const;

    /**
     * @brief Get the UUID of this publisher.
     * @return the UUID of the publisher.
     */
    const utils::UUID& getUUID() const;

    /**
     * @brief Get the IPs of the interfaces this publisher is bound to.
     * @return A vector with the IPs this publisher is bound to.
     */
    void extracted() const;
    std::vector<std::string> getPublisherIps() const;

    /**
     * @brief Get the IPs of the interfaces this publisher is bound to as a single string.
     * @param separator The separator used to separate the different interfaces.
     * @return A string with the IPs this publisher is bound to separated by separator sequence.
     */
    std::string getPublisherIpsStr(const std::string &separator) const;

    /**
     * @brief Get the network adapter addresses used by the publisher.
     *
     * This function returns a const reference to a vector of NetworkAdapterInfo objects. Each object contains
     * information about a network adapter used by the publisher for send messages.
     *
     * @return A const reference to a vector of NetworkAdapterInfo objects.
     */
    const std::vector<internal_helpers::network::NetworkAdapterInfo> getPublisherAddresses() const;

    /**
     * @brief Check if the publisher is working, i.e., it was successfully started.
     * @return true if publisher is working, false otherwise.
     */
    bool isWorking() const;

    /**
     * @brief Enqueues a messgae to be sent by the publisher worker.
     * @param topic, the topic associated to the message that will be sent.
     * @param priority, the message priority.
     * @param data, the data that will be sent in the message.
     * @return The result of sending operation as an OperationResult enum.
     * @note This method is thread-safe, since it is protected by a mutex.
     * @todo Queued functionality.
     */
    OperationResult enqueueMsg(const TopicType& topic, MessagePriority priority, PublishedData &&data);

    /**
     * @brief Sends a PubSubMsg serializing the arguments.
     * @param topic, the topic of the msg.
     * @param priority, the message priority.
     * @param args, the arguments to serialize into the message.
     * @return the result of the sending operation.
     */
    template <typename Topic, typename... Args>
    OperationResult enqueueMsg(const Topic &topic, MessagePriority priority, const Args&... args)
    {
        PublishedData data;

        if constexpr (sizeof...(args) > 0)
            data.size = zmqutils::serializer::BinarySerializer::fastSerialization(
                data.bytes, std::forward<const Args&>(args)...);

        return this->enqueueMsg(static_cast<TopicType>(topic), priority, std::move(data));
    }

    template <typename Topic, typename... Args>
    OperationResult enqueueMsg(const Topic &topic, MessagePriority priority, Args&... args)
    {
        PublishedData data;

        if constexpr (sizeof...(args) > 0)
            data.size = zmqutils::serializer::BinarySerializer::fastSerialization(
                data.bytes, std::forward<const Args&>(args)...);

        return this->enqueueMsg(static_cast<TopicType>(topic), priority, std::move(data));
    }

    /**
     * @brief Get the network adapter information of interfaces that this publisher is bound to.
     *
     * This function returns a const reference to a vector of NetworkAdapterInfo objects. Each `NetworkAdapterInfo`
     * object contains information about a network adapter used by the publisher for communication.
     *
     * @return A const reference to a vector of NetworkAdapterInfo objects.
     */
    const std::vector<internal_helpers::network::NetworkAdapterInfo> &getBoundInterfaces() const;

    static std::string operationResultToString(OperationResult result);

    static std::string operationResultToString(ResultType result);
    /**
     * @brief Virtual destructor to ensure proper cleanup when the derived class is destroyed.
     * @warning The publisher will stop if is running but in this case the `onPublisherStop` callback can't be executed.
     */
    virtual ~PublisherBase() override;

protected:

    /**
     * @brief Base publisher start callback. Subclasses can override this function.
     */
    virtual void onPublisherStart();

    /**
     * @brief Base publisher stop callback. Subclasses can override this function.
     */
    virtual void onPublisherStop();

    /**
     * @brief Base publisher sending message callback. Subclasses can override this function.
     */
    virtual void onSendingMsg(const PublishedMessage &);

    /**
     * @brief Base publisher error callback. Subclasses can override this function.
     */
    virtual void onPublisherError(const zmq::error_t&, const std::string&);

private:

    // Helper aliases.
    using NetworkAdapterInfoV = std::vector<internal_helpers::network::NetworkAdapterInfo>;

    /// Internal method for the queue worker thread.
    void messageQueueWorker();

    /// Internal method for enqueue messages.
    bool internalEnqueueMsg(PublishedMessage &&msg);

    /// Internal helper to delete the ZMQ sockets.
    void deleteSockets();

    /// Internal helper to stop the publisher.
    void internalStopPublisher();

    /// Internal helper to reset the server.
    bool internalResetPublisher();

    /// Intermal helper to get the publisher addresses.
    const NetworkAdapterInfoV& internalGetPublisherAddresses() const;

    /// Internal function to prepare the data.
    /// Be careful with this function, since it takes the ownership of the data.
    zmq::multipart_t prepareMessage(PublishedMessage &publication);

    // Endpoint data and publisher info.
    NetworkAdapterInfoV publisher_adapters_;  ///< Interfaces bound by publisher.
    PublisherInfo pub_info_;                  ///< Publisher information.

    // ZMQ sockets and endpoint.
    zmq::socket_t *publisher_socket_;       ///< ZMQ publisher socket.
    zmq::error_t last_zmq_error_;    ///< Last ZMQ error.

    // Mutex.
    mutable std::shared_mutex pub_mtx_;  ///< Safety mutex.

    // Usefull flags and parameters.
    std::atomic_bool flag_publisher_working_;               ///< Flag for check the working status.
    std::atomic_uint publisher_reconn_attempts_;  ///< Publisher reconnection number of attempts.

    // Queues for each priority level
    std::queue<PublishedMessage> queue_prio_critical_;  ///< Queue for critical priority messages.
    std::queue<PublishedMessage> queue_prio_high_;      ///< Queue for high priority messages.
    std::queue<PublishedMessage> queue_prio_normal_;    ///< Queue for normal priority messages.
    std::queue<PublishedMessage> queue_prio_low_;       ///< Queue for low priority messages.
    std::queue<PublishedMessage> queue_prio_no_;        ///< Queue for no priority messages.

    // Queues related members.
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic_bool stop_queue_worker_;
    std::thread queue_worker_th_;

    // Specific class scope (for debug purposes).
    inline static const std::string kClassScope = "[LibZMQUtils,PublisherSubscriber,PublisherBase]";
};

}} // END NAMESPACES.
// =====================================================================================================================
