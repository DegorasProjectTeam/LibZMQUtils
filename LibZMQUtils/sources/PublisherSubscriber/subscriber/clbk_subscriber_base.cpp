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
 * @file clbk_subscriber_base.cpp
 * @brief This file contains the implementation of the ClbkSubscriberBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/PublisherSubscriber/subscriber/clbk_subscriber_base.h"
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

ClbkSubscriberBase::ClbkSubscriberBase(const std::string &subscriber_name,
                                       const std::string &subscriber_version,
                                       const std::string &subscriber_info) :
    SubscriberBase(subscriber_name, subscriber_version, subscriber_info)
{}

void ClbkSubscriberBase::setErrorCallback(std::function<void (const PublishedMessage &, OperationResult)> callback)
{
    this->error_callback_ = callback;
}

void ClbkSubscriberBase::removeCallback(const TopicType &topic)
{
    CallbackHandler::removeCallback(std::hash<TopicType>{}(topic));
}

bool ClbkSubscriberBase::hasCallback(const TopicType &topic)
{
    return CallbackHandler::hasCallback(std::hash<TopicType>{}(topic));
}

ClbkSubscriberBase::~ClbkSubscriberBase()
{}

void ClbkSubscriberBase::onInvalidMsgReceived(const PublishedMessage& msg, OperationResult res)
{
    this->invokeErrorCallback(msg, res);
}

void ClbkSubscriberBase::onMsgReceived(const PublishedMessage &msg, OperationResult res)
{
    if (OperationResult::OPERATION_OK != res)
        this->invokeErrorCallback(msg, res);
}

void ClbkSubscriberBase::invokeErrorCallback(const PublishedMessage &msg, OperationResult res)
{
    if (this->error_callback_)
        this->error_callback_(msg, res);
}

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
