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
 * @file debug_publisher_base.h
 * @brief This file contains the declaration of the DebugPublisherBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/PublisherSubscriber/publisher/publisher_base.h"
#include "LibZMQUtils/PublisherSubscriber/data/publisher_subscriber_data.h"
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================


class LIBZMQUTILS_EXPORT DebugPublisherBase : public PublisherBase
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
    DebugPublisherBase(unsigned port, const std::string& ip_address = "*", const std::string& publisher_name = "",
                       const std::string& publisher_version = "", const std::string& publisher_info = "");


protected:

    /**
     * @brief Base publisher start callback. Subclasses can override this function.
     */
    virtual void onPublisherStart() override;

    /**
     * @brief Base publisher stop callback. Subclasses can override this function.
     */
    virtual void onPublisherStop() override;

    /**
     * @brief Base publisher sending message callback. Subclasses can override this function.
     */
    virtual void onSendingMsg(const PublishedMessage &) override;

    /**
     * @brief Base publisher error callback. Subclasses can override this function.
     */
    virtual void onPublisherError(const zmq::error_t&, const std::string&) override;

private:

    std::string generateStringHeader(const std::string& clbk_name, const std::vector<std::string>& data);

};

}} // END NAMESPACES.
// =====================================================================================================================
