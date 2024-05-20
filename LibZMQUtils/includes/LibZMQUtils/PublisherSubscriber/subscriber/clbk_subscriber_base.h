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

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include "LibZMQUtils/PublisherSubscriber/subscriber/subscriber_base.h"
#include "LibZMQUtils/Utilities/callback_handler.h"
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace pubsub{
// =====================================================================================================================

/**
 * @brief The ClbkSubscriberBase class implements a Subscriber that includes callback handling for each topic.
 */
class LIBZMQUTILS_EXPORT ClbkSubscriberBase : public SubscriberBase,
                                              private utils::CallbackHandler
{

public:

    /**
     * @brief ClbkSubscriberBase default constructor.
     */
    ClbkSubscriberBase(const std::string& subscriber_name = "",
                       const std::string& subscriber_version = "",
                       const std::string& subscriber_info = "");

    /**
     * @brief Function for setting an error callback.
     * @param callback The error callback method that will be called.
     */
    void setErrorCallback(std::function<void(const PublishedMessage&, OperationResult)> callback);

    /**
     * @brief Template function for registering a callback. This callback will be registered for a specific topic.
     * @tparam ClassT, the class of the object that contains the callback.
     * @tparam RetT, the return type of the callback function.
     * @tparam Args, variadic template parameters passed to the callback function.
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
     * @brief Template function for setting an error callback. This callback will called whenever a subscriber error
     *        is issued.
     *
     * @param object A object whose method will be called.
     * @param callback The error callback method that will be called.
     */
    template<typename ClassT>
    void setErrorCallback(ClassT* object, void(ClassT::*error_callback)(const PublishedMessage &, OperationResult))
    {
        this->error_callback_ = [object, error_callback](const PublishedMessage &msg, OperationResult res)
        {(object->*error_callback)(msg, res);};
    }

    /**
     * @brief Registers a callback and an associated request processing function for a specific topic.
     *
     * This function not only registers a callback for a specific topic but also sets up an automated
     * process function to handle message with that topic. The process function automatically
     * invokes the registered callback with appropriate parameters extracted from the message.
     *
     * This approach simplifies the setup process by automatically linking the message processing logic
     * with the appropriate callback, thereby reducing manual boilerplate code and potential errors.
     *
     * @tparam CallbackType The type of the callback handler, usually determining how the callback will
     *         be invoked and with what parameters.
     * @tparam ClassT The class type on which the member function callback is defined.
     * @tparam RetT The return type of the callback function. Defaults to void.
     * @tparam Args Variadic template parameters representing the types of the arguments that the
     *         callback function accepts.
     *
     * @param topic The identifier for the topic this callback and process function are associated with.
     * @param object Pointer to the instance of the object on which the callback method will be called.
     * @param callback Member function pointer to the callback method that will be invoked to process
     *        the message.
     */
    template<typename CallbackType, typename ClassT, typename RetT = void, typename... Args>
    void registerCbAndReqProcFunc(const TopicType& topic, ClassT* object, RetT(ClassT::*callback)(Args...))
    {
        // Register the callback.
        this->registerCallback(topic, object, callback);

        // Process function lambda.
        auto lambdaProcFunc = [this](const PublishedMessage& msg)
        {
            this->processClbkRequest<CallbackType, RetT, Args...>(msg);
        };

        // Automatic command process function registration.
        this->registerRequestProcFunc(topic, lambdaProcFunc);
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
     * @brief Override onInvalidMsgReceived to call error callback.
     */
    void onInvalidMsgReceived(const PublishedMessage&, OperationResult) override;

    /**
     * @brief Override onMsgReceived to call error callback if necessary.
     * @return the subscriber result associated with the message received.
     */
    void onMsgReceived(const PublishedMessage&, OperationResult) override;

    /**
     * @brief Invokes error callback, if defined.
     * @param msg, the message that caused the error.
     * @param res, the subscriber result with the error.
     */
    void invokeErrorCallback(const PublishedMessage &msg, OperationResult res);

    /**
     * @brief Parametric method for invoking a registered callback. If no callback is registered, the function will
     *        try to invoke error callback.
     * @tparam CallbackType The type of the callback function to be invoked.
     * @tparam RetT The return type of the callback function.
     * @tparam Args Variadic template parameters passed to the callback function.
     * @param msg, the received message.
     * @param res, the subscriber result associated with callback invocation
     * @param args, the args passed to the callback.
     * @return the result of the callback inovocation or nothing, if callback was not invoked or has void return.
     */
    template <typename CallbackType, typename RetT = void,  typename... Args>
    std::conditional_t<std::is_void_v<RetT>, void, std::optional<RetT>>
    invokeCallback(const PublishedMessage& msg, OperationResult& res, Args&&... args)
    {
        // Get the command.
        TopicType topic = msg.topic;

        // Check the callback.
        if(!this->hasCallback(topic))
        {
            // If there is no callback, try to execute error callback.
            res = OperationResult::EMPTY_EXT_CALLBACK;
            this->invokeErrorCallback(msg, res);
            return RetT();
        }

        //Invoke the callback.
        try
        {
            if constexpr (std::is_void_v<RetT>)
            {
                CallbackHandler::invokeCallback<CallbackType, RetT>(
                    std::hash<TopicType>{}(topic), std::forward<Args>(args)...);
                return;
            }

            else
            {
                return CallbackHandler::invokeCallback<CallbackType, RetT>(
                    std::hash<TopicType>{}(topic), std::forward<Args>(args)...);
            }
        }
        catch(...)
        {
            // If an error rises, try to execute error callback.
            res = OperationResult::INVALID_EXT_CALLBACK;
            this->invokeErrorCallback(msg, res);
            return RetT();
        }
    }

    /**
     * @brief Processes a callback request based on the topic and the data contained in the message received.
     *
     * This function processes different types of callback requests by handling input parameters.
     *
     * The function deserializes input data from the request and invokes the appropriate callback based on the
     * template parameters.
     *
     * @tparam CallbackType The type of the callback function to be invoked.
     * @tparam RetT The return type of the callback function.
     * @tparam Args Variadic template parameters passed to the callback function.
     * @param msg A reference to the PublishedMessage object containing the received message.
     */
    template<typename CallbackType, typename RetT, typename ...Args>
    void processClbkRequest(const PublishedMessage& msg)
    {

        OperationResult res;
        // Input callback case.
        if constexpr (sizeof...(Args) > 0)
        {
            // If there are no parameters, but they are required, execute error callback
            if (0 == msg.data.size)
            {
                this->invokeErrorCallback(msg, OperationResult::EMPTY_PARAMS);
                return;
            }


            std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...> args;

            // Deserialize the inputs.
            try
            {
                zmqutils::serializer::BinarySerializer::fastDeserialization(msg.data.bytes.get(),
                                                                            msg.data.size,
                                                                            args);
            }
            catch(...)
            {
                this->invokeErrorCallback(msg, OperationResult::BAD_PARAMETERS);
                return;
            }


            // Invoke the callback with unpacked parameters
            std::apply([this, &msg, &res](auto&&... args)
            {
                this->invokeCallback<CallbackType, RetT>(msg, res, std::forward<decltype(args)>(args)...);
            }, args);

        }
        else
        {
            // Invoke the callback that do not require input parameters
            this->invokeCallback<CallbackType, RetT>(msg, res);
        }
    }

private:

    // Hide the base functions.
    using CallbackHandler::invokeCallback;
    using CallbackHandler::removeCallback;
    using CallbackHandler::hasCallback;

    // Error callback functor.
    std::function<void(const PublishedMessage&, OperationResult)> error_callback_;
};

}} // END NAMESPACES.
// =====================================================================================================================
