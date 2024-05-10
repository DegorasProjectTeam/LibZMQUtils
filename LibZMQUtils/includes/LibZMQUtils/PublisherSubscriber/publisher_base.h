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
#include <string>
#include <atomic>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Global/zmq_context_handler.h"
#include "LibZMQUtils/PublisherSubscriber/common.h"
#include "LibZMQUtils/InternalHelpers/network_helpers.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
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
     * This constructor initializes a ZeroMQ-based publisher, setting the port for listening, the IP address for
     * binding connections, and other metadata such as the publisher name, version, and additional information.
     *
     * @param port              The port number on which the publisher will listen for incoming connections.
     * @param ip_address        The IP address on which the publisher will accept connections. By default, it listens
     *                              on all available interfaces ("*").
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
    PublisherBase(unsigned port, const std::string& ip_address = "*", const std::string& publisher_name = "",
                  const std::string& publisher_version = "", const std::string& publisher_info = "");
    
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
     * @return A const reference to the PublisherInfo struct that contains all the server information.
     */
    const PublisherInfo& getPublisherInfo() const;

    /**
     * @brief Get the UUID of this publisher.
     * @return the UUID of the publisher.
     */
    const utils::UUID& getUUID() const;

    /**
     * @brief Check if the publisher is working, i.e., it was successfully started.
     * @return true if publisher is working, false otherwise.
     */
    bool isWorking() const;

    /**
     * @brief Sends a PubSubMsg.
     * @param data, the data that will be sent in the msg.
     * @return the result of sending operation.
     */
    PublisherResult sendMsg(const PubSubData &data);

    /**
     * @brief Sends a PubSubMsg serializing the arguments.
     * @param topic, the topic of the msg.
     * @param args, the arguments to serialize into the message.
     * @return the result of the sending operation.
     */
    template <typename Topic, typename... Args>
    PublisherResult sendMsg(const Topic &topic, const Args&... args)
    {
        PubSubData data;
        data.topic = static_cast<TopicType>(topic);

        if constexpr (sizeof...(args) > 0)
            data.data_size = zmqutils::serializer::BinarySerializer::fastSerialization(
                data.data, std::forward<const Args&>(args)...);

        return this->sendMsg(data);
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
    virtual void onSendingMsg(const PubSubData&);

    /**
     * @brief Base publisher error callback. Subclasses can override this function.
     */
    virtual void onPublisherError(const zmq::error_t&, const std::string&);

private:

    // Helper aliases.
    using NetworkAdapterInfoV = std::vector<internal_helpers::network::NetworkAdapterInfo>;

    void deleteSockets();

    void internalStopPublisher();

    bool internalResetPublisher();

    zmq::multipart_t prepareMessage(const PubSubData &data);

    // Endpoint data and publisher info.
    PublisherInfo pub_info_;
    NetworkAdapterInfoV server_adapters_;  ///< Interfaces bound by publisher.

    // ZMQ sockets and endpoint.
    zmq::socket_t *socket_;       ///< ZMQ publisher socket.

    // Mutex.
    mutable std::mutex mtx_;      ///< Safety mutex.

    // Usefull flags.
    std::atomic_bool flag_working_;  ///< Flag for check the working status.
};

}} // END NAMESPACES.
// =====================================================================================================================
