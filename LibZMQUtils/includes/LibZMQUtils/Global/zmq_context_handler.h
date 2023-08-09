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
 * @file zmq_context_handler.h
 * @brief This file contains the declaration of the global ZMQContextHandler class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2308.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
#include <vector>
#include <functional>
#include <mutex>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

class LIBZMQUTILS_EXPORT ZMQContextHandler
{

public:

    // Aliases.
    using ContextHandlerReference = std::reference_wrapper<ZMQContextHandler>;

    static ZMQContextHandler& getInstance();

    virtual ~ZMQContextHandler();

protected:

    ZMQContextHandler();

    ZMQContextHandler(const ZMQContextHandler&) = delete;

    ZMQContextHandler& operator=(const ZMQContextHandler&) = delete;

    const std::unique_ptr<zmq::context_t>& getContext();

private:

    // Internal variables and containers.
    inline static std::mutex mtx_;                                   ///< Safety mutex.
    inline static std::unique_ptr<zmq::context_t> context_;          ///< ZMQ global context.
    inline static std::vector<ContextHandlerReference> instances_;   ///< Instances of the ContextHandler.
};

} // END NAMESPACES.
// =====================================================================================================================
