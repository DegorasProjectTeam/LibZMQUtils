#include "drgg_example_server.h"

DRGGCustomServer::DRGGCustomServer(unsigned int port, const std::string &local_addr) :
    CommandServerBase(port, local_addr){}

void DRGGCustomServer::onCustomCommandReceived(const CommandRequest &cmd_req, CommandReply &cmd_rep)
{
    // Get the command string.
    CommandServerBase::onCustomCommandReceived(cmd_req, cmd_rep);
}

void DRGGCustomServer::onServerStart()
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

    std::cout<<"---------------------------------------------------------"<<std::endl;
    std::cout<<"ON SERVER START: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Addresses: "<<ips<<std::endl;
    std::cout<<"Port: "<<this->getServerPort()<<std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
}

void DRGGCustomServer::onServerStop()
{
    std::cout<<"---------------------------------------------------------"<<std::endl;
    std::cout<<"ON SERVER CLOSE: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
}

void DRGGCustomServer::onWaitingCommand()
{
    std::cout<<"---------------------------------------------------------"<<std::endl;
    std::cout<<"ON WAITING COMMAND: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
}

void DRGGCustomServer::onDeadClient()
{
    std::cout<<"---------------------------------------------------------"<<std::endl;
    std::cout<<"ON DEAD CLIENT: "<<std::endl;
    std::cout<<"Client dead: TODO"<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
}

void DRGGCustomServer::onNewConnection(const CommandRequest &cmd_req)
{
    std::cout<<"---------------------------------------------------------"<<std::endl;
    std::cout<<"ON CONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client Id: "<<cmd_req.client.id<<std::endl;
    std::cout<<"Client Ip: "<<cmd_req.client.ip<<std::endl;
    std::cout<<"Client Host: "<<cmd_req.client.hostname<<std::endl;
    std::cout<<"Client Process: "<<cmd_req.client.pid<<std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
}

void DRGGCustomServer::onDisconnected(const CommandRequest &cmd_req)
{
    std::cout<<"---------------------------------------------------------"<<std::endl;
    std::cout<<"ON DISCONNECTED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client Id: "<<cmd_req.client.id<<std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
}

void DRGGCustomServer::onServerError(const zmq::error_t &error, const std::string &ext_info)
{
    std::cout<<"---------------------------------------------------------"<<std::endl;
    std::cout<<"ON SERVER ERROR: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Code: "<<error.num()<<std::endl;
    std::cout<<"Error: "<<error.what()<<std::endl;
    std::cout<<"Info: "<<ext_info<<std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
}

void DRGGCustomServer::onCommandReceived(const CommandRequest &cmd_req)
{
    // Get the command string.
    std::string cmd_str;
    std::uint32_t command = static_cast<std::uint32_t>(cmd_req.command);

    std::cout<<"Internal"<<std::endl;
    std::cout<<command<<std::endl;
    std::cout<<DRGGServerCommandStr.size()<<std::endl;
    std::cout<<DRGGServerCommandStr[command]<<std::endl;

    std::cout<<DRGGServerCommandStr[4]<<std::endl;

    if(command < DRGGServerCommandStr.size())
        cmd_str = DRGGServerCommandStr[command];
    else
        cmd_str = "Unknown command";

    std::cout<<"---------------------------------------------------------"<<std::endl;
    std::cout<<"ON COMMAND RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client Id: "<<cmd_req.client.id<<std::endl;
    std::cout<<"Command: "<<command<<" ("<<cmd_str<<")"<<std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
}

void DRGGCustomServer::onBadMessageReceived(const CommandRequest &cmd_req)
{
    std::cout<<"---------------------------------------------------------"<<std::endl;
    std::cout<<"ON BAD COMMAND RECEIVED: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Client Id: "<<cmd_req.client.id<<std::endl;
    std::cout<<"Client Ip: "<<cmd_req.client.ip<<std::endl;
    std::cout<<"Client Host: "<<cmd_req.client.hostname<<std::endl;
    std::cout<<"Client Process: "<<cmd_req.client.pid<<std::endl;
    std::cout<<"Command: "<<static_cast<int>(cmd_req.command)<<std::endl;
    std::cout<<"Params Size: "<<cmd_req.params_size<<std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
}

void DRGGCustomServer::onSendingResponse(const CommandReply &cmd_rep)
{
    int result = static_cast<int>(cmd_rep.result);
    std::cout<<"---------------------------------------------------------"<<std::endl;
    std::cout<<"ON SENDING RESPONSE: "<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout<<"Result: "<<result<<" ("<<zmqutils::common::BaseServerResultStr[result]<<")"<<std::endl;
    std::cout<<"---------------------------------------------------------"<<std::endl;
}


