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
 * @file clbk_subscriber_base.h
 * @brief This file contains the declaration of the ClbkSubscriberBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/PublisherSubscriber/subscriber_base.h"
#include "LibZMQUtils/Utilities/callback_handler.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

/**
 * @brief The ClbkSubscriberBase class implements a Subscriber that includes callback handling for each topic.
 */
class LIBZMQUTILS_EXPORT ClbkSubscriberBase : public SubscriberBase, public utils::CallbackHandler
{

public:

    /**
     * @brief ClbkSubscriberBase default constructor.
     */
    ClbkSubscriberBase();

    /**
     * @brief Template function for registering a callback. This callback will be registered for a specific topic.
     * @param topic, the topic the callback is applied to.
     * @param object, a parametric object whose method will be called.
     * @param callback, the callback method that will be called.
     */
    template<typename ClassT, typename RetT = void, typename... Args>
    void registerCallback(const TopicType &topic, ClassT* object, RetT(ClassT::*callback)(Args...))
    {
        CallbackHandler::registerCallback(std::hash<TopicType>{}(topic), object, callback);
    }

    /**
     * @brief Remove the registered callback for a specific topic.
     * @param topic, the topic whose callback will be erased.
     */
    void removeCallback(const TopicType &topic);

    /**
     * @brief Check if there is a registered callback for a specific topic.
     * @param topic, the topic whose callback existence will be checked.
     * @return
     */
    bool hasCallback(const TopicType &topic);

    /**
     * @brief Virtual destructor.
     */
    virtual ~ClbkSubscriberBase() override;

protected:

    /**
     * @brief Parametric method for invoking a registed callback. If no callback is registered, an error is returned.
     * @param msg, the received message.
     * @param args, the args passed to the callback.
     * @return the result of the callback inovocation.
     */
    template <typename CallbackType, typename RetT = void,  typename... Args>
    RetT invokeCallback(const PubSubMsg& msg, Args&&... args)
    {
        // Get the command.
        TopicType topic = msg.data.topic;

        // Check the callback.
        if(!this->hasCallback(topic))
        {
            return SubscriberResult::EMPTY_EXT_CALLBACK;
        }

        //Invoke the callback.
        try
        {
            return CallbackHandler::invokeCallback<CallbackType, RetT>(
                std::hash<TopicType>{}(topic), std::forward<Args>(args)...);
        }
        catch(...)
        {
            return SubscriberResult::INVALID_EXT_CALLBACK;
        }
    }

private:

    // Hide the base functions.
    using CallbackHandler::invokeCallback;
    using CallbackHandler::removeCallback;
    using CallbackHandler::hasCallback;
};

}} // END NAMESPACES.
// =====================================================================================================================
