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
 * @file clbk_subscriber_base.h
 * @brief This file contains the declaration of the ClbkSubscriberBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/PublisherSubscriber/common.h"
#include "LibZMQUtils/PublisherSubscriber/subscriber_base.h"
#include "LibZMQUtils/Utilities/callback_handler.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

class ClbkSubscriberBase : public SubscriberBase, public utils::CallbackHandler
{

public:

    LIBZMQUTILS_EXPORT ClbkSubscriberBase();

    template<typename ClassT, typename RetT = void, typename... Args>
    void registerCallback(const common::TopicType &topic, ClassT* object, RetT(ClassT::*callback)(Args...))
    {
        CallbackHandler::registerCallback(std::hash<common::TopicType>{}(topic), object, callback);
    }

    LIBZMQUTILS_EXPORT void removeCallback(const common::TopicType &topic);

    LIBZMQUTILS_EXPORT bool hasCallback(const common::TopicType &topic);

    LIBZMQUTILS_EXPORT virtual ~ClbkSubscriberBase() override;

protected:

    template <typename CallbackType, typename RetT = void,  typename... Args>
    RetT invokeCallback(const common::PubSubMsg& msg, Args&&... args)
    {
        // Get the command.
        common::TopicType topic = msg.data.topic;

        // Check the callback.
        if(!this->hasCallback(topic))
        {
            return common::SubscriberResult::EMPTY_EXT_CALLBACK;
        }

        //Invoke the callback.
        try
        {
            return CallbackHandler::invokeCallback<CallbackType, RetT>(
                std::hash<common::TopicType>{}(topic), std::forward<Args>(args)...);
        }
        catch(...)
        {
            return common::SubscriberResult::INVALID_EXT_CALLBACK;
        }
    }

private:

    // Hide the base functions.
    using CallbackHandler::invokeCallback;
    using CallbackHandler::removeCallback;
    using CallbackHandler::hasCallback;
};

} // END NAMESPACES.
// =====================================================================================================================
