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
 * @brief The DebugClbkSubscriberBase class implements a ClbkSubscriberBase that includes internal callbacks that
 * prints all the input and output data in each internal callback call to facilitate debugging and development. At any
 * time you can toggle inheritance between DebugClbkSubscriberBase and the original ClbkSubscriberBase one to monitor
 * what is happening on the screen. This class is for support and does not imply that a robust logging system should
 * not be used in the override implementation of the system being developed.
 */
class LIBZMQUTILS_EXPORT DebugClbkSubscriberBase : public ClbkSubscriberBase
{

public:

    DebugClbkSubscriberBase(const std::string& subscriber_name = "",
                            const std::string& subscriber_version = "",
                            const std::string& subscriber_info = "",
                            bool log_internal_callbacks = true);

protected:

    /// Internal overrided onSubscriberStart callback that logs when executing.
    void onSubscriberStart() override;

    /// Internal overrided onSubscriberStop callback that logs when executing.
    void onSubscriberStop() override;

    /// Internal overrided onSubscriberError callback that logs when executing.
    void onSubscriberError(const zmq::error_t &error, const std::string &ext_info) override;

    /// Internal overrided onInvalidMsgReceived callback that logs when executing.
    void onInvalidMsgReceived(const PublishedMessage&, OperationResult) override;

    /// Internal overrided onMsgReceived callback that logs when executing.
    void onMsgReceived(const PublishedMessage&, OperationResult) override;

private:

    // Auxiliar flags.
    std::atomic_bool log_internal_callbacks_;

    // Auxiliar function to generate the log string.
    std::string generateStringHeader(const std::string& clbk_name, const std::vector<std::string>& data);
};

}} // END NAMESPACES.
// =====================================================================================================================
