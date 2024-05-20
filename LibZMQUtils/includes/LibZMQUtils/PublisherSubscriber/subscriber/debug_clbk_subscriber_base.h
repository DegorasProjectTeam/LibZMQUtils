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
 * @file debug_clbk_subscriber_base.h
 * @brief This file contains the declaration of the DebugClbkSubscriberBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/PublisherSubscriber/subscriber/clbk_subscriber_base.h"
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

/**
 * @brief The DebugClbkSubscriberBase class implements a ClbkSubscriberBase that override status callback methods
 * logging the information at standard output.
 */
class LIBZMQUTILS_EXPORT DebugClbkSubscriberBase : public ClbkSubscriberBase
{

public:

    /**
     * @brief DebugClbkSubscriberBase default constructor.
     */
    DebugClbkSubscriberBase(const std::string& subscriber_name = "",
                            const std::string& subscriber_version = "",
                            const std::string& subscriber_info = "");


protected:

    void onSubscriberStart() override;
    void onSubscriberStop() override;
    void onSubscriberError(const zmq::error_t &error, const std::string &ext_info) override;

    /**
     * @brief Override onInvalidMsgReceived to call error callback.
     */
    void onInvalidMsgReceived(const PublishedMessage&, OperationResult) override;

    /**
     * @brief Override onMsgReceived to call error callback if necessary.
     * @return the subscriber result associated with the message received.
     */
    void onMsgReceived(const PublishedMessage&, OperationResult) override;

private:

    std::string generateStringHeader(const std::string& clbk_name, const std::vector<std::string>& data);


};

}} // END NAMESPACES.
// =====================================================================================================================
