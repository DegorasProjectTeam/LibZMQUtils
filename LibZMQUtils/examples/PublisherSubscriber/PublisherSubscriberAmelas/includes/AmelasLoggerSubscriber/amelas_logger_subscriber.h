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
#include <string>
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/CallbackSubscriber>
#include <LibZMQUtils/Modules/Utils>
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
class AmelasLoggerSubscriber : public zmqutils::pubsub::ClbkSubscriberBase
{
public:

    AmelasLoggerSubscriber();

    template<typename ClassT, typename RetT = void, typename... Args>
    void registerCallback(const controller::AmelasLogLevel &log_level, ClassT* object, RetT(ClassT::*callback)(Args...))
    {
        zmqutils::pubsub::ClbkSubscriberBase::registerCallback(
            AmelasLoggerTopic[static_cast<size_t>(log_level)], object, callback);
    }

    void addTopicFilter(const controller::AmelasLogLevel& log_level);

    using LogMsgCallback = std::function<zmqutils::pubsub::SubscriberResult(const controller::AmelasLog&)>;

private:

    // -----------------------------------------------------------------------------------------------------------------
    using SubscriberBase::registerRequestProcFunc;
    using CallbackHandler::registerCallback;
    using zmqutils::pubsub::ClbkSubscriberBase::registerCallback;
    using zmqutils::pubsub::SubscriberBase::addTopicFilter;
    // -----------------------------------------------------------------------------------------------------------------

    zmqutils::pubsub::SubscriberResult processLogMsg(const zmqutils::pubsub::PubSubMsg&);

    // Internal overrided start callback.
    virtual void onSubscriberStart() override final;

    // Internal overrided close callback.
    virtual void onSubscriberStop() override final;

    // Internal overrided command received callback.
    virtual zmqutils::pubsub::SubscriberResult onMsgReceived(const zmqutils::pubsub::PubSubMsg&) override final;

    // Internal overrided bad command received callback.
    virtual void onInvalidMsgReceived(const zmqutils::pubsub::PubSubMsg&,
                                      zmqutils::pubsub::SubscriberResult) override final;

    // Internal overrided server error callback.
    virtual void onSubscriberError(const zmq::error_t&, const std::string& ext_info) override final;
};

}} // END NAMESPACES.
// =====================================================================================================================
