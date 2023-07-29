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

AmelasClient::AmelasClient(const std::string &server_endpoint) :
    zmqutils::CommandClientBase(server_endpoint)
{}

void AmelasClient::onSendingCommand(const RequestData &req, const zmq::multipart_t &msg)
{
    // Get the command string.
    std::string cmd_str;
    cmd_str = (req.command <AmelasServerCommandStr.size()) ? AmelasServerCommandStr[req.command] : "Unknown command";
    // Log.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS CLIENT>"<<std::endl;
    std::cout<<"-> ON SEND COMMAND: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Command: "<<static_cast<int>(req.command)<<std::endl;
    std::cout<<"Params size: "<<req.params_size<<std::endl;
    std::cout<<"Msg parts: "<<msg.size()<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}



}} // END NAMESPACES.
// =====================================================================================================================

