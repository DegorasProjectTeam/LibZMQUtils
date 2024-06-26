/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
 *   ExamplesLibZMQUtils related project.                                                                              *
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
 * @file amelas_logger_subscriber.h
 * @brief EXAMPLE FILE - This file contains the declaration of the AmelasLoggerSubscriber example class.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/PublisherSubscriber>
#include <LibZMQUtils/Modules/Utilities>
// =====================================================================================================================

// INTERFACE INCLUDES
// =====================================================================================================================
#include <AmelasController/amelas_log.h>
#include <AmelasLoggerPublisher/amelas_logger_publisher.h>
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace communication{
// =====================================================================================================================

// Example of creating a command server from the base.
class AmelasLoggerSubscriber : public zmqutils::pubsub::DebugClbkSubscriberBase
{
public:

    using LogMsgCallback = std::function<void(const controller::AmelasLog&)>;

    using zmqutils::pubsub::DebugClbkSubscriberBase::DebugClbkSubscriberBase;

    /**
     * @brief Adds topic filter based on log level. Upon subscriber creation, no topic is allowed in, so this function
     *        must be called to start receiving logs.
     * @param log_level, the log level to allow in.
     */
    void addTopicFilter(const controller::AmelasLogLevel& log_level);

    /**
     * @brief Removes topic filter based on log level.
     * @param log_level, the log level to NOT allow in.
     */
    void removeTopicFilter(const controller::AmelasLogLevel& log_level);

    /**
     * @brief Register callback function for each log level.
     * @tparam ClassT, the class of the object that contains the callback.
     * @tparam RetT, the return type of the callback.
     * @tparam Variadic template parameters passed to the callback function.
     * @param log_level, the log level to apply the callback.
     * @param object, the object that contains the callback.
     * @param callback, the callback function called when a message with log_level comes.
     */
    template<typename ClassT, typename RetT = void, typename... Args>
    void registerCallback(const controller::AmelasLogLevel &log_level, ClassT* object, RetT(ClassT::*callback)(Args...))
    {
        zmqutils::pubsub::ClbkSubscriberBase::registerCallback(
            AmelasLoggerTopic[static_cast<size_t>(log_level)], object, callback);
    }

    /**
     * @brief Register callback and processing function for each log level.
     * @tparam CallbackType, the signature of the callback.
     * @tparam ClassT, the class of the object that contains the callback.
     * @tparam RetT, the return type of the callback.
     * @tparam Variadic template parameters passed to the callback function.
     * @param log_level, the log level to apply the callback and processing function.
     * @param object, the object that contains the callback.
     * @param callback, the callback function called when a message with log_level comes.
     */
    template<typename CallbackType = LogMsgCallback, typename ClassT, typename RetT = void, typename... Args>
    void registerCallbackAndRequestProcFunc(const controller::AmelasLogLevel &log_level, ClassT* object,
                                            RetT(ClassT::*callback)(Args...))
    {
        zmqutils::pubsub::ClbkSubscriberBase::registerCbAndReqProcFunc<CallbackType, ClassT, RetT, Args...>(
            zmqutils::pubsub::TopicType(AmelasLoggerTopic[static_cast<size_t>(log_level)]), object, callback);
    }

private:

    // -----------------------------------------------------------------------------------------------------------------
    using SubscriberBase::registerRequestProcFunc;
    using zmqutils::pubsub::ClbkSubscriberBase::registerCbAndReqProcFunc;
    using zmqutils::pubsub::ClbkSubscriberBase::registerCallback;
    using zmqutils::pubsub::SubscriberBase::addTopicFilter;
    using zmqutils::pubsub::SubscriberBase::removeTopicFilter;
    // -----------------------------------------------------------------------------------------------------------------
};

}} // END NAMESPACES.
// =====================================================================================================================
