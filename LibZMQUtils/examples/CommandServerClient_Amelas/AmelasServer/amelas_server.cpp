


#include "amelas_server.h"

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace cltsrv{
// =====================================================================================================================

using zmqutils::common::ServerCommand;
using zmqutils::common::ServerResult;
using zmqutils::common::ResultType;
using zmqutils::utils::BinarySerializer;


AmelasServer::AmelasServer(unsigned int port, const std::string &local_addr) :
    CommandServerBase(port, local_addr),
    CallbackHandler()
{
    // Register each process function.

    this->registerProcessFunction(AmelasServerCommand::REQ_SET_HOME_POSITION,
                                  &AmelasServer::processSetHomePosition);

    this->registerProcessFunction(AmelasServerCommand::REQ_GET_HOME_POSITION,
                                  &AmelasServer::processGetHomePosition);


}


void AmelasServer::processSetHomePosition(const CommandRequest& request, CommandReply& reply)
{
    // Auxiliar variables and containers.
    controller::ControllerError ctrl_err;
    double az, el;

    // Check the request parameters size.
    if (request.params_size == 0 || !request.params)
    {
        reply.result = ServerResult::EMPTY_PARAMS;
        return;
    }

    // Try to read the parameters data.
    try
    {
        BinarySerializer::fastDeserialization(request.params.get(), request.params_size, az, el);
    }
    catch(...)
    {
        reply.result = ServerResult::BAD_PARAMETERS;
        return;
    }

    // Position struct.
    controller::AltAzPos pos = {az, el};

    // Now we will process the command in the controller.
    ctrl_err = this->invokeCallback<controller::SetHomePositionCallback>(request, reply, pos);

    // Serialize parameters if all ok.
    if(reply.result == ServerResult::COMMAND_OK)
        reply.params_size = BinarySerializer::fastSerialization(reply.params, ctrl_err);
}

void AmelasServer::processGetHomePosition(const CommandRequest& request, CommandReply &reply)
{
    // Auxiliar variables and containers.
    controller::ControllerError ctrl_err;
    controller::AltAzPos pos;

    // Now we will process the command in the controller.
    ctrl_err = this->invokeCallback<controller::GetHomePositionCallback>(request, reply, pos);

    // Serialize parameters if all ok.
    if(reply.result == ServerResult::COMMAND_OK)
        reply.params_size = BinarySerializer::fastSerialization(reply.params, ctrl_err, pos.az, pos.el);
}

void AmelasServer::processAmelasCommand(const CommandRequest& request, CommandReply& reply)
{
    AmelasServerCommand command = static_cast<AmelasServerCommand>(request.command);

    auto iter = process_fnc_map_.find(command);
    if(iter != process_fnc_map_.end())
    {
        // Invoke the function.
        iter->second(request, reply);
    }
    else
    {
        // Command not found in the map.
        reply.result = ServerResult::NOT_IMPLEMENTED;
    }
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

void AmelasServer::onCustomCommandReceived(const CommandRequest& request, CommandReply& reply)
{
    // Get the command.
    AmelasServerCommand command = static_cast<AmelasServerCommand>(request.command);

    // Get the command string.
    std::string cmd_str;
    std::uint32_t cmd_uint = static_cast<std::uint32_t>(request.command);
    cmd_str = (cmd_uint < AmelasServerCommandStr.size()) ? AmelasServerCommandStr[cmd_uint] : "Unknown command";

    // Log the command.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"ON CUSTOM COMMAND RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client Id: "<<request.client.id<<std::endl;
    std::cout<<"Command: "<<cmd_uint<<" ("<<cmd_str<<")"<<std::endl;
    std::cout << std::string(100, '-') << std::endl;

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
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SERVER START: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Addresses: "<<ips<<std::endl;
    std::cout<<"Port: "<<this->getServerPort()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onServerStop()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SERVER CLOSE: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onWaitingCommand()
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON WAITING COMMAND: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onDeadClient(const HostClientInfo& client)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON DEAD CLIENT: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Current Clients: "<<this->getConnectedClients().size()<<std::endl;
    std::cout<<"Client Id: "<<client.id<<std::endl;
    std::cout<<"Client Ip: "<<client.ip<<std::endl;
    std::cout<<"Client Host: "<<client.hostname<<std::endl;
    std::cout<<"Client Process: "<<client.pid<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onConnected(const HostClientInfo& client)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON CONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Current Clients: "<<this->getConnectedClients().size()<<std::endl;
    std::cout<<"Client Id: "<<client.id<<std::endl;
    std::cout<<"Client Ip: "<<client.ip<<std::endl;
    std::cout<<"Client Host: "<<client.hostname<<std::endl;
    std::cout<<"Client Process: "<<client.pid<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onDisconnected(const HostClientInfo& client)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON DISCONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Current Clients: "<<this->getConnectedClients().size()<<std::endl;
    std::cout<<"Client Id: "<<client.id<<std::endl;
    std::cout<<"Client Ip: "<<client.ip<<std::endl;
    std::cout<<"Client Host: "<<client.hostname<<std::endl;
    std::cout<<"Client Process: "<<client.pid<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onServerError(const zmq::error_t &error, const std::string &ext_info)
{
    // Log.
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SERVER ERROR: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Code: "<<error.num()<<std::endl;
    std::cout<<"Error: "<<error.what()<<std::endl;
    std::cout<<"Info: "<<ext_info<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onCommandReceived(const CommandRequest &request)
{
    // Get the command string.
    std::string cmd_str;
    std::uint32_t command = static_cast<std::uint32_t>(request.command);
    cmd_str = (command < AmelasServerCommandStr.size()) ? AmelasServerCommandStr[command] : "Unknown command";
    // Log.
    BinarySerializer serializer(request.params.get(), request.params_size);
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON COMMAND RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client Id: "<<request.client.id<<std::endl;
    std::cout<<"Command: "<<command<<" ("<<cmd_str<<")"<<std::endl;
    std::cout<<"Params Size: "<<request.params_size<<std::endl;
    std::cout<<"Params Hex: "<<serializer.getDataHexString()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onInvalidMsgReceived(const CommandRequest &request)
{
    // Log.
    BinarySerializer serializer(request.params.get(), request.params_size);
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON BAD COMMAND RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client Id: "<<request.client.id<<std::endl;
    std::cout<<"Client Ip: "<<request.client.ip<<std::endl;
    std::cout<<"Client Host: "<<request.client.hostname<<std::endl;
    std::cout<<"Client Process: "<<request.client.pid<<std::endl;
    std::cout<<"Command: "<<static_cast<int>(request.command)<<std::endl;
    std::cout<<"Params Size: "<<request.params_size<<std::endl;
    std::cout<<"Params Hex: "<<serializer.getDataHexString()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

void AmelasServer::onSendingResponse(const CommandReply &reply)
{
    // Log.
    BinarySerializer serializer(reply.params.get(), reply.params_size);
    int result = static_cast<int>(reply.result);
    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SENDING RESPONSE: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Result: "<<result<<" ("<<AmelasServerResultStr[result]<<")"<<std::endl;
    std::cout<<"Params Size: "<<reply.params_size<<std::endl;
    std::cout<<"Params Hex: "<<serializer.getDataHexString()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;
}

}} // END NAMESPACES.
// =====================================================================================================================

