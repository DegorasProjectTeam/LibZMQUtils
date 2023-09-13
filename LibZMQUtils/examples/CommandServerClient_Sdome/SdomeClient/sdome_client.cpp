#include "sdome_client.h"

// SDOME NAMESPACES
// =====================================================================================================================
namespace sdome{
namespace communication{
// =====================================================================================================================

using common::SdomeServerCommandStr;
using common::SdomeServerResultStr;
using common::SdomeServerCommand;
using common::SdomeServerResult;
using zmqutils::common::ServerCommand;
using zmqutils::common::ServerResult;
using zmqutils::common::ResultType;
using zmqutils::common::CommandType;
using zmqutils::utils::BinarySerializer;

SdomeClient::SdomeClient(const std::string& server_endpoint,
                           const std::string& client_name,
                           const std::string interf_name) :
    zmqutils::CommandClientBase(server_endpoint, client_name, interf_name)
{}

void SdomeClient::onClientStart()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON CLIENT START: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Endpoint: "<<this->getServerEndpoint()<<std::endl;
    std::cout<<"Name: "<<this->getClientInfo().name<<std::endl;
    std::cout<<"UUID: "<<this->getClientInfo().uuid.toRFC4122String()<<std::endl;
    std::cout<<"Ip: "<<this->getClientInfo().ip<<std::endl;
    std::cout<<"Pid: "<<this->getClientInfo().pid<<std::endl;
    std::cout<<"Hostname: "<<this->getClientInfo().hostname<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void SdomeClient::onClientStop()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON CLIENT STOP: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void SdomeClient::onWaitingReply()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON WAITING REPLY: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void SdomeClient::onDeadServer()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON DEAD SERVER: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void SdomeClient::onConnected()
{
    // TODO In base get server info when connected.
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON CONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Endpoint: "<<this->getServerEndpoint()<<std::endl;
    std::cout<<"Server Name: "<<std::endl;
    std::cout<<"Server Version: "<<std::endl;
    std::cout<<"Server Info: "<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void SdomeClient::onDisconnected()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<SDOME SERVER>"<<std::endl;
    std::cout<<"-> ON DISCONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void SdomeClient::onReplyReceived(const CommandReply &reply)
{
    // Auxiliar.
    BinarySerializer serializer(reply.params.get(), reply.params_size);
    ResultType result = static_cast<ResultType>(reply.result);
    std::string res_str = zmqutils::utils::getEnumString(reply.result, SdomeServerResultStr);
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON REPLY RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Result: "<<result<<" ("<<res_str<<")"<<std::endl;
    std::cout<<"Params Size: "<<reply.params_size<<std::endl;
    std::cout<<"Params Hex: "<<serializer.getDataHexString()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void SdomeClient::onSendingCommand(const RequestData &req)
{
    BinarySerializer serializer(req.params.get(), req.params_size);
    CommandType command = static_cast<CommandType>(req.command);
    std::string cmd_str = zmqutils::utils::getEnumString(req.command, SdomeServerCommandStr);
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON SEND COMMAND: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Command: "<<command<<" ("<<cmd_str<<")"<<std::endl;
    std::cout<<"Params size: "<<req.params_size<<std::endl;
    std::cout<<"Params Hex: "<<serializer.getDataHexString()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void SdomeClient::onClientError(const zmq::error_t& error, const std::string& ext_info)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<"<<this->getClientName()<<">"<<std::endl;
    std::cout<<"-> ON CLIENT ERROR: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Code: "<<error.num()<<std::endl;
    std::cout<<"Error: "<<error.what()<<std::endl;
    std::cout<<"Info: "<<ext_info<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}



}} // END NAMESPACES.
// =====================================================================================================================

