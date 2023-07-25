#include "amelas_example_server.h"

AmelasExampleServer::AmelasExampleServer(unsigned int port, const std::string &local_addr) :
    CommandServerBase(port, local_addr)
{}



void AmelasExampleServer::processSetHomePosition(const CommandRequest& request, CommandReply& reply)
{
    AmelasExampleController::AmelasError amelas_err;

    // Auxilar variables.
    double az, el;
    unsigned double_sz = sizeof(double);
    bool result;

    // Check the request parameters size.
    if (request.params_size == 0)
    {
        reply.result = common::BaseServerResult::EMPTY_PARAMS;
        return;
    }
    else if (request.params_size != double_sz*2)
    {
        reply.result = common::BaseServerResult::BAD_PARAMETERS;
        return;
    }

    // Deserialize the parameters.
    zmqutils::utils::binarySerializeDeserialize(request.params.get(), double_sz, &az);
    zmqutils::utils::binarySerializeDeserialize(request.params.get() + double_sz, double_sz, &el);

    // Process the command.
    amelas_err =
        this->invoke<AmelasExampleController::SetHomePositionCallback>(AmelasServerCommand::REQ_SET_HOME_POSITION,
                                                                           az, el);
    // Store the amelas error.
    reply.params = std::unique_ptr<std::uint8_t>(new std::uint8_t[sizeof(common::ResultType)]);
    common::ResultType amelas_res = static_cast<common::ResultType>(amelas_res);
    zmqutils::utils::binarySerializeDeserialize(&amelas_res, sizeof(common::ResultType), reply.params.get());
    reply.params_size = sizeof(common::ResultType);

    // Store the server result.
    reply.result = common::BaseServerResult::COMMAND_OK;
}

void AmelasExampleServer::processAmelasCommand(const CommandRequest& request, CommandReply& reply)
{
    AmelasServerCommand command = static_cast<AmelasServerCommand>(request.command);

    if(command == AmelasServerCommand::REQ_SET_HOME_POSITION)
    {
        this->processSetHomePosition(request, reply);
    }
    else
    {
        reply.result = common::BaseServerResult::NOT_IMPLEMENTED;
    }
}

void AmelasExampleServer::onCustomCommandReceived(const CommandRequest& request, CommandReply& reply)
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
        reply.result = common::BaseServerResult::INVALID_MSG;
    }
    else if(AmelasExampleServer::validateAmelasCommand(command))
    {
        this->processAmelasCommand(request, reply);
    }
    else
    {
        // Call to the base function.
        CommandServerBase::onCustomCommandReceived(request, reply);
    }
}

void AmelasExampleServer::onServerStart()
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

void AmelasExampleServer::onServerStop()
{
    // Log.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON SERVER CLOSE: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

void AmelasExampleServer::onWaitingCommand()
{
    // Log.
    std::cout << std::string(80, '-') << std::endl;
    std::cout<<"<AMELAS SERVER>"<<std::endl;
    std::cout<<"-> ON WAITING COMMAND: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

void AmelasExampleServer::onDeadClient(const HostClient& client)
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

void AmelasExampleServer::onConnected(const HostClient& client)
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

void AmelasExampleServer::onDisconnected(const HostClient& client)
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

void AmelasExampleServer::onServerError(const zmq::error_t &error, const std::string &ext_info)
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

void AmelasExampleServer::onCommandReceived(const CommandRequest &cmd_req)
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

void AmelasExampleServer::onInvalidMsgReceived(const CommandRequest &cmd_req)
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

void AmelasExampleServer::onSendingResponse(const CommandReply &cmd_rep)
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

bool AmelasExampleServer::validateAmelasCommand(AmelasServerCommand command)
{
    // Auxiliar variables.
    bool result = false;
    common::CommandType cmd = static_cast<common::CommandType>(command);
    // Check if the command is within the range of implemented custom commands.
    if (cmd >= kMinCmdId && cmd <= kMaxCmdId)
        result = true;
    return result;
}
