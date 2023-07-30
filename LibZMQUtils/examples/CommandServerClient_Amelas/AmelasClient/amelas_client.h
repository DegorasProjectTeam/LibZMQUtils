
// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/CommandClient>
#include <LibZMQUtils/Utils>
// =====================================================================================================================

// AMELAS INCLUDES
// =====================================================================================================================
#include <AmelasServer/amelas_server.h>
#include <AmelasServer/common.h>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace cltsrv{
// =====================================================================================================================

using namespace amelas::cltsrv::common;
using zmqutils::common::RequestData;


class AmelasClient : public zmqutils::CommandClientBase
{
public:

    AmelasClient(const std::string &server_endpoint);

    // TODO
    //virtual void prepareRequest() = 0;

    virtual ~AmelasClient(){}

private:

    virtual void onClientStart() final;

    virtual void onClientStop() final;

    virtual void onWaitingReply() final;

    virtual void onDeadServer() final;

    virtual void onConnected() final;

    virtual void onDisconnected() final;

    virtual void onInvalidMsgReceived(const CommandReply&) final {};

    virtual void onReplyReceived(const CommandReply& reply) final;

    virtual void onSendingCommand(const RequestData&) final;

    virtual void onClientError(const zmq::error_t&, const std::string& ext_info) final;


};

}} // END NAMESPACES.
// =====================================================================================================================
