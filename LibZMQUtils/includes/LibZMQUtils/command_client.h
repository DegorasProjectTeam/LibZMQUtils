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
 * @file command_client.h
 * @brief This file contains the declaration of the CommandClientBase class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2307.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <future>
#include <map>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/libzmqutils_global.h"
#include "LibZMQUtils/common.h"
// =====================================================================================================================

// ZMQ DECLARATIONS
// =====================================================================================================================
namespace zmq
{
    class context_t;
    class socket_t;
}
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

// =====================================================================================================================
using common::CmdRequestId;
using common::BaseServerCommand;
// =====================================================================================================================

struct LIBZMQUTILS_EXPORT CommandData
{
    CommandData(BaseServerCommand id) :
        command_id(id),
        params(nullptr),
        params_size(0){}

    BaseServerCommand command_id;
    void* params;
    size_t params_size;
};

class LIBZMQUTILS_EXPORT CommandClientBase
{

public:


    // TODO: maybe this should be configurable
    static const int kClientAliveTimeoutMsec;
    static const int kClientSendAlivePeriodMsec;

    enum class CommandError : std::uint32_t
    {
        NOT_ERROR,
        NO_COMMAND,
        NOT_CONNECTED,
        ALREADY_DISCONNECTED,
        ALREADY_CONNECTED,
        BAD_PARAMETERS,
        COMMAND_FAILED,
        NOT_IMPLEMENTED
    };


    CommandClientBase(const std::string &server_endpoint);

    bool startClient(const std::string& interface_name);
    void stopClient();
    void resetClient();

    void startAutoAlive();
    void stopAutoAlive();

    void setClientHostIP(const std::string& interf);

    void setClientId(const std::string &id){};

    int sendCommand(const CommandData& msg, void* &data_out, size_t &out_bytes);

    // Remove. Only for testing.
    int sendBadCommand1(void* &data_out, size_t &out_bytes);

    static void binarySerializeDeserialize(const void *data, size_t data_size_bytes, void *dest);

private:

    int recvFromSocket(zmq::socket_t *socket, void *&data, size_t &data_size_bytes) const;
    void sendAliveCallback();

    // Internal client identification.
    common::HostClient client_info_;

    // Server endpoint.
    std::string server_endpoint_;

    // ZMQ context and socket.
    zmq::context_t *context_;
    zmq::socket_t *socket_;

    // Mutex.
    std::mutex mtx_;

    std::future<void> auto_alive_future_;
    std::condition_variable auto_alive_cv_;
    std::atomic_bool auto_alive_working_;

};

} // END NAMESPACES.
// =====================================================================================================================
