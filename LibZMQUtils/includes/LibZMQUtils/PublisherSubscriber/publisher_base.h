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
 * @file publisher_base.h
 * @brief This file contains the declaration of the PublisherBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
#include <atomic>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/Global/zmq_context_handler.h"
#include "LibZMQUtils/PublisherSubscriber/common.h"
#include "LibZMQUtils/InternalHelpers/network_helpers.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

// =====================================================================================================================
using common::SubscriberResult;
using common::ClientResult;
// =====================================================================================================================

/**
 * @brief The PublisherBase class represents a base publisher that binds to and endpoint and can send PubSubMsg messages.
 *
 * This base class can be used without specialization or it can be inherited to provide callbacks for start, stop,
 * message sending and error handling.
 */
class PublisherBase : public ZMQContextHandler
{

public:
    
    /**
     * @brief PublisherBase constructor.
     * @param endpoint, the endpoint to bind the publisher to.
     * @param name, optional name of the publisher. It will be sent on messages.
     */
    LIBZMQUTILS_EXPORT PublisherBase(std::string endpoint,
                                     std::string name = "");
    
    /**
     * @brief Start the publisher so it can send messages. It must be started before sending messages.
     * @return true if it was started successfully. False otherwise.
     */
    LIBZMQUTILS_EXPORT bool startPublisher();

    /**
     * @brief Stops the publisher and cleans the socket. Messages cannot be sent until publisher is started again.
     */
    LIBZMQUTILS_EXPORT void stopPublisher();

    /**
     * @brief Restarts the publisher.
     * @return true if reset was successful, false otherwise.
     */
    LIBZMQUTILS_EXPORT bool resetPublisher();

    /**
     * @brief Get the endpoint that this publisher is bound to.
     * @return the URL of the endpoint that this publisher is bound to.
     */
    LIBZMQUTILS_EXPORT const std::string& getEndpoint() const;

    /**
     * @brief Get the name of this publisher.
     * @return the name of the publisher.
     */
    LIBZMQUTILS_EXPORT const std::string& getName() const;

    /**
     * @brief Check if the publisher is working, i.e., it was successfully started.
     * @return true if publisher is working, false otherwise.
     */
    LIBZMQUTILS_EXPORT bool isWorking() const;

    /**
     * @brief Sends a PubSubMsg.
     * @param data, the data that will be sent in the msg.
     * @return the result of sending operation.
     */
    LIBZMQUTILS_EXPORT ClientResult sendMsg(const common::PubSubData &data);

    /**
     * @brief Get the network adapter information of interfaces that this publisher is bound to.
     *
     * This function returns a const reference to a vector of NetworkAdapterInfo objects. Each `NetworkAdapterInfo`
     * object contains information about a network adapter used by the publisher for communication.
     *
     * @return A const reference to a vector of NetworkAdapterInfo objects.
     */
    LIBZMQUTILS_EXPORT const std::vector<internal_helpers::network::NetworkAdapterInfo> &getBoundInterfaces() const;

    /**
     * @brief Virtual destructor to ensure proper cleanup when the derived class is destroyed.
     * @warning The publisher will stop if is running but in this case the `onPublisherStop` callback can't be executed.
     */
    LIBZMQUTILS_EXPORT virtual ~PublisherBase() override;

protected:

    /**
     * @brief Base publisher start callback. Subclasses can override this function.
     */
    LIBZMQUTILS_EXPORT virtual void onPublisherStart() {}

    /**
     * @brief Base publisher stop callback. Subclasses can override this function.
     */
    LIBZMQUTILS_EXPORT virtual void onPublisherStop() {}

    /**
     * @brief Base publisher sending message callback. Subclasses can override this function.
     */
    LIBZMQUTILS_EXPORT virtual void onSendingMsg(const common::PubSubData&) {}

    /**
     * @brief Base publisher error callback. Subclasses can override this function.
     */
    LIBZMQUTILS_EXPORT virtual void onPublisherError(const zmq::error_t&, const std::string&) {}

private:

    void deleteSockets();

    void internalStopPublisher();

    bool internalResetPublisher();

    zmq::multipart_t prepareMessage(const common::PubSubData &data);

    // Internal publisher identification.
    common::PublisherInfo pub_info_;

    // ZMQ sockets and endpoint.
    std::string endpoint_;        ///< Publisher endpoint.
    zmq::socket_t *socket_;       ///< ZMQ publisher socket.


    // Mutex.
    mutable std::mutex mtx_;      ///< Safety mutex.

    // Bound interfaces
    std::vector<internal_helpers::network::NetworkAdapterInfo> bound_ifaces_;  ///< Interfaces bound by publisher.


    // Usefull flags.
    std::atomic_bool flag_working_;           ///< Flag for check the working status.
};

} // END NAMESPACES.
// =====================================================================================================================
