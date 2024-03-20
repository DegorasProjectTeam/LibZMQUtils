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
#include <condition_variable>
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
using common::ServerResult;
using common::ClientResult;
// =====================================================================================================================

class PublisherBase : public ZMQContextHandler
{

public:
    
    LIBZMQUTILS_EXPORT PublisherBase(std::string endpoint,
                                     std::string name = "");
    
    LIBZMQUTILS_EXPORT bool startPublisher();

    LIBZMQUTILS_EXPORT void stopPublisher();

    LIBZMQUTILS_EXPORT bool resetPublisher();

    LIBZMQUTILS_EXPORT const std::string& getEndpoint() const;

    LIBZMQUTILS_EXPORT const std::string& getName() const;

    LIBZMQUTILS_EXPORT bool isWorking() const;

    LIBZMQUTILS_EXPORT ClientResult sendMsg(const common::PubSubData &);

    /**
     * @brief Get the network adapter addresses used by the server.
     *
     * This function returns a const reference to a vector of NetworkAdapterInfo objects. Each `NetworkAdapterInfo`
     * object contains information about a network adapter used by the server for communication.
     *
     * @return A const reference to a vector of NetworkAdapterInfo objects.
     */
    LIBZMQUTILS_EXPORT const std::vector<internal_helpers::network::NetworkAdapterInfo> &getServerAddresses() const;

    /**
     * @brief Virtual destructor to ensure proper cleanup when the derived class is destroyed.
     * @warning The client will stop if is running but in this case the `onClientStop` callback can't be executed.
     */
    LIBZMQUTILS_EXPORT virtual ~PublisherBase() override;

protected:

    LIBZMQUTILS_EXPORT virtual void onPublisherStart() = 0;

    LIBZMQUTILS_EXPORT virtual void onPublisherStop() = 0;


    LIBZMQUTILS_EXPORT virtual void onSendingMsg(const common::PubSubData&) = 0;

    LIBZMQUTILS_EXPORT virtual void onPublisherError(const zmq::error_t&, const std::string& ext_info) = 0;

private:

    void deleteSockets();

    void internalStopPublisher();

    bool internalResetPublisher();

    zmq::multipart_t prepareMessage(const common::PubSubData &data);

    // Internal client identification.
    common::PublisherInfo pub_info_;

    // ZMQ sockets and endpoint.
    std::string endpoint_;        ///< Server endpoint.
    zmq::socket_t *socket_;       ///< ZMQ client socket.
    zmq::socket_t *recv_close_socket_;   ///< ZMQ auxiliar socket for requesting to close.
    zmq::socket_t *req_close_socket_;    ///< ZMQ auxiliar socket for receiving the close request.

    // Condition variables with associated flags.
    std::condition_variable stopped_done_cv_;
    std::atomic_bool flag_closed_;

    // Mutex.
    mutable std::mutex mtx_;                    ///< Safety mutex.
    mutable std::mutex client_close_mtx_;       ///< Safety mutex for closing client.

    // Bound interfaces
    std::vector<internal_helpers::network::NetworkAdapterInfo> bound_ifaces_;


    // Usefull flags.
    std::atomic_bool flag_working_;           ///< Flag for check the working status.
};

} // END NAMESPACES.
// =====================================================================================================================
