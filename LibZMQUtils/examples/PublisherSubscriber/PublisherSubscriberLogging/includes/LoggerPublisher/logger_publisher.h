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
 * @file logger_publisher.h
 * @brief EXAMPLE FILE - This file contains the declaration of the LoggerPublisher example class.
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
#include <LibZMQUtils/Modules/Publisher>
#include <LibZMQUtils/Modules/Utils>
// =====================================================================================================================

#include "includes/LoggerCommon/logger_common.h"

// NAMESPACES
// =====================================================================================================================
namespace logger{
// =====================================================================================================================

class LoggerPublisher : public zmqutils::pubsub::PublisherBase
{
public:

    LoggerPublisher(std::string endpoint,
                    std::string name = "");

    zmqutils::pubsub::PublisherResult sendLog(const AmelasLog &log);

private:

    virtual void onPublisherStart() override final;

    virtual void onPublisherStop() override final;

    virtual void onSendingMsg(const zmqutils::pubsub::PubSubData&) override final;

    virtual void onPublisherError(const zmq::error_t&, const std::string& ext_info) override final;
};

}  // END NAMESPACES.
// =====================================================================================================================
