
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

    virtual void onClientStart() final{};

    virtual void onClientStop() final{};

    virtual void onWaitingReply() final{};

    virtual void onDeadServer() final{};

    virtual void onConnected() final{};

    virtual void onDisconnected() final{};

    virtual void onReplyReceived(const CommandReply&) final {};

    virtual void onSendingCommand(const RequestData&, const zmq::multipart_t&) final;

    virtual void onClientError(const zmq::error_t&, const std::string& ext_info) final{};

    /* TODO
    void onConnected(const HostClient& client) override
    {
        // Log.
        std::cout << std::string(80, '-') << std::endl;
        std::cout<<"<AMELAS SERVER>"<<std::endl;
        std::cout<<"-> ON CONNECTED: "<<std::endl;
        std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout<<"Current Clients: "<<this->getConnectedClients().size()<<std::endl;
        std::cout<<"Client Id: "<<client.id<<std::endl;
        std::cout<<"Client Ip: "<<client.ip<<std::endl;
        std::cout<<"Client Host: "<<client.hostname<<std::endl;
        std::cout<<"Client Process: "<<client.pid<<std::endl;
        std::cout << std::string(80, '-') << std::endl;
    }
    */
};

}} // END NAMESPACES.
// =====================================================================================================================
