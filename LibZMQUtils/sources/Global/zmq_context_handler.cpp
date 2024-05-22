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
 * @file zmq_context_handler.cpp
 * @brief This file contains the implementation of the global ZMQContextHandler class.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <vector>
#include <functional>
#include <mutex>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/zmq_context_handler.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{

ZMQContextHandler::ZMQContextHandler()
{
    // For the first instance, create the context.
    if (ZMQContextHandler::instances_.empty())
        ZMQContextHandler::context_ = std::make_unique<zmq::context_t>(1);

    // Register this instance.
    ZMQContextHandler::instances_.push_back(std::ref(*this));
}

ZMQContextHandler &ZMQContextHandler::getInstance()
{
    static ZMQContextHandler instance;
    return instance;
}

ZMQContextHandler::~ZMQContextHandler()
{

    // Safety mutex.
    std::lock_guard<std::mutex> lock(ZMQContextHandler::mtx_);

    // Unregister this instance.
    ZMQContextHandler::instances_.erase(
        std::remove_if(ZMQContextHandler::instances_.begin(), ZMQContextHandler::instances_.end(),
                       [this](const ContextHandlerReference& ref)
                       { return &ref.get() == this; }),
        ZMQContextHandler::instances_.end());

    // Destroy the context if no instances left.
    if (ZMQContextHandler::instances_.empty())
        ZMQContextHandler::context_.reset();
}

const std::unique_ptr<zmq::context_t>& ZMQContextHandler::getContext()
{
    return ZMQContextHandler::context_;
}

// =====================================================================================================================

} // END NAMESPACES.
// =====================================================================================================================
