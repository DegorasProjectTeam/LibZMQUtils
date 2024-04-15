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
 * @file logger_subscriber.h
 * @brief EXAMPLE FILE - This file contains the declaration of the LoggerSubscriber example class.
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
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/CallbackSubscriber>
#include <LibZMQUtils/Modules/Utils>
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace logger {
// =====================================================================================================================

class AmelasLog;

// Example of creating a command server from the base.
class LoggerSubscriber : public zmqutils::pubsub::ClbkSubscriberBase
{
public:

    LoggerSubscriber();

    using zmqutils::pubsub::ClbkSubscriberBase::registerCallback;

    using LogMsgCallback = std::function<zmqutils::pubsub::SubscriberResult(const AmelasLog&)>;

private:

    // -----------------------------------------------------------------------------------------------------------------
    using SubscriberBase::registerRequestProcFunc;
    using CallbackHandler::registerCallback;
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

} // END NAMESPACES.
// =====================================================================================================================
