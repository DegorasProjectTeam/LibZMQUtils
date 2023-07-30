#include "amelas_client.h"

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace cltsrv{
// =====================================================================================================================

using common::AmelasServerCommandStr;
using common::AmelasServerResultStr;
using common::AmelasServerCommand;
using common::AmelasServerResult;
using zmqutils::common::ServerCommand;
using zmqutils::common::ServerResult;
using zmqutils::common::ResultType;
using zmqutils::utils::BinarySerializer;

AmelasClient::AmelasClient(const std::string &server_endpoint) :
    zmqutils::CommandClientBase(server_endpoint)
{}

void AmelasClient::onClientStart()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS CLIENT>"<<std::endl;
    std::cout<<"-> ON CLIENT START: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client Id: "<<this->getClientInfo().id<<std::endl;
    std::cout<<"Endpoint: "<<this->getServerEndpoint()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasClient::onClientStop()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS CLIENT>"<<std::endl;
    std::cout<<"-> ON CLIENT STOP: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasClient::onWaitingReply()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS CLIENT>"<<std::endl;
    std::cout<<"-> ON WAITING REPLY: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasClient::onDeadServer()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS CLIENT>"<<std::endl;
    std::cout<<"-> ON DEAD SERVER: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasClient::onConnected()
{
    // TODO In base get server info when connected.
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS CLIENT>"<<std::endl;
    std::cout<<"-> ON CONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Endpoint: "<<this->getServerEndpoint()<<std::endl;
    std::cout<<"Server Name: "<<std::endl;
    std::cout<<"Server Version: "<<std::endl;
    std::cout<<"Server Info: "<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasClient::onDisconnected()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON DISCONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasClient::onSendingCommand(const RequestData &req)
{
    // Get the command string.
    std::string cmd_str;
    std::uint32_t command = static_cast<std::uint32_t>(req.command);
    cmd_str = (command < AmelasServerCommandStr.size()) ? AmelasServerCommandStr[command] : "Unknown command";
    // Log.
    BinarySerializer serializer(req.params.get(), req.params_size);
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS CLIENT>"<<std::endl;
    std::cout<<"-> ON SEND COMMAND: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Command: "<<static_cast<int>(req.command)<<std::endl;
    std::cout<<"Params size: "<<req.params_size<<std::endl;
    std::cout<<"Params Hex: "<<serializer.getDataHexString()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasClient::onClientError(const zmq::error_t& error, const std::string& ext_info)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS CLIENT>"<<std::endl;
    std::cout<<"-> ON CLIENT ERROR: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Code: "<<error.num()<<std::endl;
    std::cout<<"Error: "<<error.what()<<std::endl;
    std::cout<<"Info: "<<ext_info<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}



}} // END NAMESPACES.
// =====================================================================================================================

