#include "amelas_server.h"

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
// =====================================================================================================================

using common::AmelasServerCommandStr;
using common::AmelasServerResultStr;
using common::ControllerError;
using common::AmelasServerCommand;
using common::AmelasServerResult;
using zmqutils::common::ServerCommand;
using zmqutils::common::ServerResult;
using zmqutils::common::ResultType;


AmelasServer::AmelasServer(unsigned int port, const std::string &local_addr) :
    CommandServerBase(port, local_addr)
{}

const std::map<AmelasServerCommand, common::ControllerCallback> &AmelasServer::getCallbackMap() const
{
    return this->callback_map_;
}

void AmelasServer::removeCallback(common::AmelasServerCommand command)
{
    this->callback_map_.erase(command);
}

bool AmelasServer::isCallbackSet(common::AmelasServerCommand command) const
{
    return this->callback_map_.find(command) != this->callback_map_.end();
}

void AmelasServer::clearCallbacks()
{
    this->callback_map_.clear();
}

void AmelasServer::processSetHomePosition(const CommandRequest& request, CommandReply& reply)
{
    // Command and error.
    AmelasServerCommand cmd = AmelasServerCommand::REQ_SET_HOME_POSITION;
    ControllerError controller_err;

    // Auxilar variables.
    double az, el;
    constexpr std::size_t double_sz = sizeof(double);
    bool result;

    // Check the request parameters size.
    if (request.params_size == 0)
    {
        reply.result = ServerResult::EMPTY_PARAMS;
        return;
    }
    else if (request.params_size != double_sz*2)
    {
        reply.result = ServerResult::BAD_PARAMETERS;
        return;
    }

    // Deserialize the parameters.
    zmqutils::utils::binarySerializeDeserialize(request.params.get(), double_sz, &az);
    zmqutils::utils::binarySerializeDeserialize(request.params.get() + double_sz, double_sz, &el);

    // Generate the struct.
    common::AltAzPos pos = {az, el};

    // Process the command.
    // Check the callback.
    if(!this->isCallbackSet(cmd))
    {
        reply.result = static_cast<ServerResult>(AmelasServerResult::EMPTY_CALLBACK);
        return;
    }

    // Process the command.
    try{controller_err = this->invokeCallback<common::SetHomePositionCallback>(cmd, pos);}
    catch(...)
    {
        reply.result = static_cast<ServerResult>(AmelasServerResult::INVALID_CALLBACK);
        return;
    }

    // Store the amelas error.
    reply.params = std::unique_ptr<std::uint8_t>(new std::uint8_t[sizeof(ResultType)]);
    ResultType amelas_res = static_cast<ResultType>(controller_err);
    zmqutils::utils::binarySerializeDeserialize(&amelas_res, sizeof(ResultType), reply.params.get());
    reply.params_size = sizeof(ResultType);
}

void AmelasServer::processGetHomePosition(const CommandRequest &, CommandReply &reply)
{
    // Command and error.
    AmelasServerCommand cmd = AmelasServerCommand::REQ_GET_HOME_POSITION;
    ControllerError controller_err;

    // Auxilar variables.
    constexpr std::size_t res_sz = sizeof(ResultType);
    constexpr std::size_t double_sz = sizeof(double);
    ControllerError amelas_err = ControllerError::SUCCESS;
    common::AltAzPos pos;

    // Process the command.
    try{controller_err = this->invokeCallback<common::SetHomePositionCallback>(cmd, pos);}
    catch(...)
    {
        reply.result = static_cast<ServerResult>(AmelasServerResult::INVALID_CALLBACK);
        return;
    }

    // Serialize parameters
    reply.params = std::unique_ptr<std::uint8_t>(new std::uint8_t[res_sz + 2*double_sz]);
    reply.params_size = res_sz + 2*double_sz;
    zmqutils::utils::binarySerializeDeserialize(&amelas_err, res_sz, reply.params.get());
    zmqutils::utils::binarySerializeDeserialize(&pos.az, double_sz, reply.params.get() + res_sz);
    zmqutils::utils::binarySerializeDeserialize(&pos.el, double_sz, reply.params.get() + res_sz + double_sz);

    // Store the server result.
    reply.result = ServerResult::COMMAND_OK;
}

void AmelasServer::processAmelasCommand(const CommandRequest& request, CommandReply& reply)
{
    AmelasServerCommand command = static_cast<AmelasServerCommand>(request.command);

    if(command == AmelasServerCommand::REQ_SET_HOME_POSITION)
    {
        this->processSetHomePosition(request, reply);
    }
    else if (command == AmelasServerCommand::REQ_GET_HOME_POSITION)
    {
        this->processGetHomePosition(request, reply);
    }
    else
    {
        reply.result = ServerResult::NOT_IMPLEMENTED;
    }
}

void AmelasServer::onCustomCommandReceived(const CommandRequest& request, CommandReply& reply)
{
    // Get the command.
    AmelasServerCommand command = static_cast<AmelasServerCommand>(request.command);

    // Get the command string.
    std::string cmd_str;
    std::uint32_t cmd_uint = static_cast<std::uint32_t>(request.command);
    cmd_str = (cmd_uint < AmelasServerCommandStr.size()) ? AmelasServerCommandStr[cmd_uint] : "Unknown command";

    // Log the command.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"ON CUSTOM COMMAND RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client Id: "<<request.client.id<<std::endl;
    std::cout<<"Command: "<<cmd_uint<<" ("<<cmd_str<<")"<<std::endl;
    std::cout << std::string(80, '-') << std::endl;

    // Process the command if it is implemented.
    if(command == AmelasServerCommand::END_AMELAS_COMMANDS)
    {
        // Update the result.
        reply.result = ServerResult::INVALID_MSG;
    }
    else if(AmelasServer::validateAmelasCommand(command))
    {
        this->processAmelasCommand(request, reply);
    }
    else
    {
        // Call to the base function.
        CommandServerBase::onCustomCommandReceived(request, reply);
    }
}

void AmelasServer::onServerStart()
{
    // Ips.
    std::string ips;

    // Get listen interfaces ips.
    for(const auto& intrfc : this->getServerAddresses())
    {
        ips.append(intrfc.ip);
        ips.append(" - ");
    }
    ips.pop_back();
    ips.pop_back();

    // Log.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SERVER START: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Addresses: "<<ips<<std::endl;
    std::cout<<"Port: "<<this->getServerPort()<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

void AmelasServer::onServerStop()
{
    // Log.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SERVER CLOSE: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

void AmelasServer::onWaitingCommand()
{
    // Log.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON WAITING COMMAND: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

void AmelasServer::onDeadClient(const HostClient& client)
{
    // Log.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON DEAD CLIENT: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Current Clients: "<<this->getConnectedClients().size()<<std::endl;
    std::cout<<"Client Id: "<<client.id<<std::endl;
    std::cout<<"Client Ip: "<<client.ip<<std::endl;
    std::cout<<"Client Host: "<<client.hostname<<std::endl;
    std::cout<<"Client Process: "<<client.pid<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

void AmelasServer::onConnected(const HostClient& client)
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

void AmelasServer::onDisconnected(const HostClient& client)
{
    // Log.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON DISCONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Current Clients: "<<this->getConnectedClients().size()<<std::endl;
    std::cout<<"Client Id: "<<client.id<<std::endl;
    std::cout<<"Client Ip: "<<client.ip<<std::endl;
    std::cout<<"Client Host: "<<client.hostname<<std::endl;
    std::cout<<"Client Process: "<<client.pid<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

void AmelasServer::onServerError(const zmq::error_t &error, const std::string &ext_info)
{
    // Log.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SERVER ERROR: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Code: "<<error.num()<<std::endl;
    std::cout<<"Error: "<<error.what()<<std::endl;
    std::cout<<"Info: "<<ext_info<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

void AmelasServer::onCommandReceived(const CommandRequest &cmd_req)
{
    // Get the command string.
    std::string cmd_str;
    std::uint32_t command = static_cast<std::uint32_t>(cmd_req.command);
    cmd_str = (command < AmelasServerCommandStr.size()) ? AmelasServerCommandStr[command] : "Unknown command";
    // Log.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON COMMAND RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client Id: "<<cmd_req.client.id<<std::endl;
    std::cout<<"Command: "<<command<<" ("<<cmd_str<<")"<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

void AmelasServer::onInvalidMsgReceived(const CommandRequest &cmd_req)
{
    // Log.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON BAD COMMAND RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Raw Str: "<<cmd_req.raw_msg.str()<<std::endl;
    std::cout<<"Client Id: "<<cmd_req.client.id<<std::endl;
    std::cout<<"Client Ip: "<<cmd_req.client.ip<<std::endl;
    std::cout<<"Client Host: "<<cmd_req.client.hostname<<std::endl;
    std::cout<<"Client Process: "<<cmd_req.client.pid<<std::endl;
    std::cout<<"Command: "<<static_cast<int>(cmd_req.command)<<std::endl;
    std::cout<<"Params Size: "<<cmd_req.params_size<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

void AmelasServer::onSendingResponse(const CommandReply &cmd_rep)
{
    // Log.
    int result = static_cast<int>(cmd_rep.result);
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SENDING RESPONSE: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Result: "<<result<<" ("<<AmelasServerResultStr[result]<<")"<<std::endl;
    std::cout<<"Params Size: "<<cmd_rep.params_size<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

bool AmelasServer::validateAmelasCommand(AmelasServerCommand command)
{
    // Auxiliar variables.
    bool result = false;
    zmqutils::common::CommandType cmd = static_cast<zmqutils::common::CommandType>(command);
    // Check if the command is within the range of implemented custom commands.
    if (cmd >= common::kMinCmdId && cmd <= common::kMaxCmdId)
        result = true;
    return result;
}

} // END NAMESPACES.
// =====================================================================================================================

