
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

using common::AmelasServerCommandStr;



class AmelasClient : public zmqutils::CommandClientBase
{
public:

    AmelasClient(const std::string &server_endpoint);

    // TODO
    //virtual void prepareRequest() = 0;

private:

    void onSendCommand(const RequestData& req, const zmq::multipart_t& msg) override;

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
