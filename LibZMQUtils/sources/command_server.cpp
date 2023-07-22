


#include <signal.h>
#include <iostream>
#include <stdio.h>

#include <zmq/zmq_addon.hpp>
#include <zmq/zmq.h>

#include "LibZMQUtils/command_server.h"
#include "LibZMQUtils/utils.h"


// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================


constexpr int kClientAliveTimeoutMsec = 8000;
constexpr unsigned kReconnectTimes = 10;

CommandServerBase::CommandServerBase(unsigned int port, const std::string& local_addr) :
    context_(nullptr),
    main_socket_(nullptr),
    server_endpoint_("tcp://" + local_addr + ":" + std::to_string(port)),
    server_port_(port),
    server_working_(false),
    client_present_(false),
    disconnect_requested_(false)
{
    // Get the adapters.
    std::vector<utils::NetworkAdapterInfo> interfcs = utils::getHostIPsWithInterfaces();
    // Store the adapters.
    if(local_addr == "*")
        this->server_listen_adapters_ = interfcs;
    else
    {
        for(const auto& intrfc : interfcs)
        {
            if(intrfc.ip == local_addr)
                this->server_listen_adapters_.push_back(intrfc);
        }
    }
}

const std::future<void> &CommandServerBase::getServerWorkerFuture() const {return this->server_worker_future_;}

const unsigned& CommandServerBase::getServerPort() const {return this->server_port_;}

const std::vector<utils::NetworkAdapterInfo>& CommandServerBase::getServerAddresses() const
{return this->server_listen_adapters_;}

const std::string& CommandServerBase::getServerEndpoint() const {return this->server_endpoint_;}

void CommandServerBase::startServer()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // If server is already started, do nothing
    if (this->server_working_)
        return;

    // Create the ZMQ context.
    if (!this->context_)
        this->context_ = new zmq::context_t(1);

    // Launch server worker in other thread.
    this->server_worker_future_ = std::async(std::launch::async, &CommandServerBase::serverWorker, this);
}

void CommandServerBase::stopServer()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // If server is already stopped, do nothing.
    if (!this->server_working_)
        return;

    // Set the shared working flag to false (is atomic).
    this->server_working_ = false;

    // Delete the context.
    if (this->context_)
    {
        delete this->context_;
        context_ = nullptr;
    }
}

CommandServerBase::~CommandServerBase()
{
    // Stop the server (this function also deletes the pointers).
    this->stopServer();
}


BaseServerResult CommandServerBase::execConnect(const CommandRequest& cmd_req)
{
    // If client is already connected and connect is issued again, report and do nothing
    if (this->client_present_)
    {
        return BaseServerResult::ALREADY_CONNECTED;
    }
    else
    {
        // Safe mutex lock
        std::unique_lock<std::mutex> lock(this->mtx_);

        // TODO multiclient.

        // Configuration for client present. The socket now has a tiemout to detect dead client.
        this->client_present_ = true;
        this->main_socket_->set(zmq::sockopt::rcvtimeo, kClientAliveTimeoutMsec);

        // Call to the internal callback.
        this->onNewConnection(cmd_req);

        // All ok.
        return BaseServerResult::COMMAND_OK;
    }
}

BaseServerResult CommandServerBase::execDisconnect(const CommandRequest& cmd_req)
{
    // TODO Multiclient

    // Call to the internal callback.
    this->onDisconnected(cmd_req);

    // Request disconnection after sending the response
    this->disconnect_requested_ = true;

    // All ok.
    return BaseServerResult::COMMAND_OK;
}

void CommandServerBase::serverWorker()
{
    // Auxiliar variables.
    BaseServerResult result;

    // Set the working flag to true.
    this->server_working_ = true;

    // Start server socket
    this->resetSocket();

    // Server worker loop.
    // If there is no client connected wait for a client to connect or for an exit message. If there
    // is a client connected set timeout, so if no command comes in time, check the last time connection
    // for each client. The loop can be stopped (in a safe way) if using the stopServer() function.
    while(this->main_socket_ && this->server_working_)
    {
        // Message container.
        CommandRequest cmd_request;

        // Result container.
        CommandReply cmd_reply;

        // Receive the data.
        result = this->recvFromSocket(cmd_request);

        // Process the data.
        if(result == BaseServerResult::COMMAND_OK && !this->server_working_)
        {
            // In this case, we will close the server. Call to the internal callback.
            this->onServerStop();
        }
        else if(result == BaseServerResult::TIMEOUT_REACHED)
        {
            // TODO improve with multiconnection.

            // Execute internal callback.
            this->onDeadClient();

            // TODO multiclient.
            this->client_present_ = false;

            // Disable the timeout.
            this->main_socket_->set(zmq::sockopt::rcvtimeo, -1);
        }
        else if (result != BaseServerResult::COMMAND_OK)
        {
            // Internal callback.
            this->onBadMessageReceived(cmd_request);

            // Prepare the message.
            std::uint8_t res_buff[sizeof(BaseServerResult)];
            utils::binarySerializeDeserialize(&result, sizeof(BaseServerResult), res_buff);
            zmq::message_t message_res(res_buff, sizeof(BaseServerResult));

            // Send response callback.
            cmd_reply.result = result;
            this->onSendingResponse(cmd_reply);

            // Send the response.
            try
            {
                this->main_socket_->send(message_res, zmq::send_flags::none);
            }
            catch (const zmq::error_t &error)
            {
                // Check if we want to close the server.
                // The error code is for ZMQ EFSM error.
                if(!(error.num() == 156384765 && !this->server_working_))
                    this->onServerError(error, "Error while sending a response.");
            }
        }
        else if (result == BaseServerResult::COMMAND_OK)
        {
            // Reply id buffer.
            std::unique_ptr<std::uint8_t> rep_id_buff;

            // Execute the command.
            this->processCommand(cmd_request, cmd_reply);

            // Prepare the command result.
            CommandServerBase::prepareCommandResult(cmd_reply.result, rep_id_buff);
            zmq::message_t message_rep_id(rep_id_buff.get(), sizeof(common::CmdReplyRes));

            // Prepare the multipart msg.
            zmq::multipart_t multipart_msg;
            multipart_msg.add(std::move(message_rep_id));

            // Specific data.
            if(cmd_reply.result == BaseServerResult::COMMAND_OK && cmd_reply.params_size != 0)
            {
                // Prepare the custom response.
                zmq::message_t message_rep_custom(cmd_reply.params.get(), cmd_reply.params_size);
                multipart_msg.add(std::move(message_rep_custom));
            }

            // Sending callback.
            this->onSendingResponse(cmd_reply);

            // Send the message.
            try
            {
                multipart_msg.send(*this->main_socket_);
            }
            catch (const zmq::error_t &error)
            {
                // Check if we want to close the server.
                // The error code is for ZMQ EFSM error.
                if(!(error.num() == 156384765 && !this->server_working_))
                    this->onServerError(error, "Error while sending a response.");
            }

            // TODO rmv
            std::cout<<"Sending response: "<<static_cast<int>(cmd_reply.result)<<std::endl;
            std::cout<<"Sending msg size: "<<multipart_msg.size()<<std::endl;
        }
    }

    if (this->main_socket_)
    {
        delete this->main_socket_;
        this->main_socket_ = nullptr;
    }
}

BaseServerResult CommandServerBase::recvFromSocket(CommandRequest& cmd_req)
{
    // Result variable.
    BaseServerResult result = BaseServerResult::COMMAND_OK;

    // Containers.
    bool recv_result;
    zmq::multipart_t multipart_msg;

    // Try to receive data. If an execption is thrown, receiving fails and an error code is generated.
    try
    {
        // Call to the internal waiting command callback.
        this->onWaitingCommand();

        // Wait the command.
        recv_result = multipart_msg.recv(*(this->main_socket_));
    }
    catch(zmq::error_t& error)
    {
        // Check if we want to close the server.
        // The error code is for ZMQ EFSM error.
        if(error.num() == 156384765 && !this->server_working_)
            return BaseServerResult::COMMAND_OK;

        // Else, call to error callback.
        this->onServerError(error, "Error while receiving a request.");
        return BaseServerResult::INTERNAL_ZMQ_ERROR;
    }

    // Check for empty msg or timeout reached.
    if (multipart_msg.empty() && !recv_result)
        return BaseServerResult::TIMEOUT_REACHED;
    else if (multipart_msg.empty())
        return BaseServerResult::EMPTY_MSG;

    // Check the multipart msg size.
    if (multipart_msg.size() == 4 || multipart_msg.size() == 5)
    {
        // Auxiliar containers.
        std::string ip;
        std::string hostname;
        std::string pid;

        // Get the multipart data.
        zmq::message_t message_ip = multipart_msg.pop();
        zmq::message_t message_hostname = multipart_msg.pop();
        zmq::message_t message_pid = multipart_msg.pop();
        zmq::message_t message_command = multipart_msg.pop();

        // Get the sizes.
        size_t ip_size_bytes = message_ip.size();
        size_t host_size_bytes = message_hostname.size();
        size_t pid_size_bytes = message_pid.size();
        size_t command_size_bytes = message_command.size();

        // First get the ip data.
        if (ip_size_bytes > 0)
            ip = std::string(static_cast<char*>(message_ip.data()), ip_size_bytes);
        else
            return BaseServerResult::EMPTY_CLIENT_IP;

        // Get the hostname data.
        if (host_size_bytes > 0)
            hostname = std::string(static_cast<char*>(message_hostname.data()), host_size_bytes);
        else
            return BaseServerResult::EMPTY_CLIENT_NAME;

        // Get the ip data.
        if (host_size_bytes > 0)
            pid = std::string(static_cast<char*>(message_pid.data()), pid_size_bytes);
        else
            return BaseServerResult::EMPTY_CLIENT_PID;

        // Update the client info.
        cmd_req.client = HostClientInfo(ip, hostname, pid);
        cmd_req.client.last_connection = std::chrono::high_resolution_clock::now();

        // Get the command.
        if (command_size_bytes == sizeof(CmdRequestId))
        {
            BaseServerCommand id;
            utils::binarySerializeDeserialize(message_command.data(), sizeof(CmdRequestId), &id);
            cmd_req.command = id;
        }
        else
            return BaseServerResult::INVALID_COMMAND;

        if (multipart_msg.size() == 5)
        {
            zmq::message_t message_params = multipart_msg.pop();
            size_t params_size_bytes = message_params.size();

            if(params_size_bytes > 0)
            {
                std::unique_ptr<std::uint8_t> cmd_data = std::unique_ptr<std::uint8_t>(new std::uint8_t[params_size_bytes]);
                auto *params_pointer = static_cast<std::uint8_t*>(message_params.data());
                std::copy(params_pointer, params_pointer + params_size_bytes, cmd_data.get());
                cmd_req.params = std::move(cmd_data);
            }
            else
                return BaseServerResult::EMPTY_PARAMS;
        }
    }
    else
        return BaseServerResult::INVALID_PARTS;

    return result;
}


void CommandServerBase::prepareCommandResult(BaseServerResult result, std::unique_ptr<std::uint8_t>& data_out)
{
    data_out = std::unique_ptr<std::uint8_t>(new std::uint8_t[sizeof(BaseServerResult)]);
    utils::binarySerializeDeserialize(&result, sizeof(CmdReplyRes), data_out.get());
}

void CommandServerBase::processCommand(const CommandRequest& cmd_req, CommandReply& cmd_reply)
{
    // First of all, call to the internal callback.
    this->onCommandReceived(cmd_req);

    // Process the different commands. The first command to process is the connect
    // request. If the command is other, check if the client is connected to the server
    // and if it is, then process the rest of the base commands.
    if (BaseServerCommand::REQ_CONNECT == cmd_req.command)
    {
        cmd_reply.result = this->execConnect(cmd_req);
    }
    else if(!this->client_present_)
    {
        cmd_reply.result = BaseServerResult::NOT_CONNECTED;
    }
    else if (BaseServerCommand::REQ_DISCONNECT == cmd_req.command)
    {
        cmd_reply.result = this->execDisconnect(cmd_req);
    }
    else if (BaseServerCommand::REQ_ALIVE == cmd_req.command)
    {
        cmd_reply.result = BaseServerResult::COMMAND_OK;
    }
    else if(BaseServerCommand::INVALID_COMMAND == cmd_req.command)
    {
        cmd_reply.result = BaseServerResult::INVALID_COMMAND;
    }
    else
    {
        // Custom command, so call the internal callback.
        this->onCustomCommandReceived(cmd_req, cmd_reply);
    }
}

void CommandServerBase::resetSocket()
{
    // Auxiliar variables.
    int res = 0;
    const zmq::error_t* last_error;
    unsigned reconnect_count = kReconnectTimes;

    // Delete the previous socket.
    if (this->main_socket_)
    {
        delete this->main_socket_;
        this->main_socket_ = nullptr;
    }
       // Try creating a new socket.
    do
    {
        try
        {
            // Create the ZMQ rep socket.
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            this->main_socket_ = new zmq::socket_t(*this->context_, zmq::socket_type::rep);
            this->main_socket_->bind(this->server_endpoint_);
            this->main_socket_->set(zmq::sockopt::linger, 0);
        }
        catch (const zmq::error_t& error)
        {
            // Delete the socket and store the last error.
            delete this->main_socket_;
            this->main_socket_ = nullptr;
            last_error = &error;
        }
        reconnect_count--;
    } while (res == EADDRINUSE && reconnect_count > 0);

    if (!this->main_socket_ )
    {
        // Update the working flag and calls to the callback.
        this->server_working_ = false;
        this->onServerError(*last_error, "Error during socket creation.");
    }
    else
    {
        // Call to the internal callback.
        this->onServerStart();
    }
}

void CommandServerBase::onNewConnection(const CommandRequest&){}

void CommandServerBase::onDisconnected(const CommandRequest&){}

void CommandServerBase::onDeadClient(){}

void CommandServerBase::onCommandReceived(const CommandRequest&){}

void CommandServerBase::onCustomCommandReceived(const CommandRequest&, CommandReply& rep)
{
    rep.result = BaseServerResult::NOT_IMPLEMENTED;
}

void CommandServerBase::onServerError(const zmq::error_t &error, const std::string&){}

void CommandServerBase::onServerStop(){}

void CommandServerBase::onServerStart(){}

void CommandServerBase::onWaitingCommand(){}

} // END NAMESPACES.
// =====================================================================================================================
