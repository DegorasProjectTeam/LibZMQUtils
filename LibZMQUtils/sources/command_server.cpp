


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

const common::CommandReqId CommandServerBase::kNoCommand = static_cast<CommandReqId>(-1);
const common::CommandReqId CommandServerBase::kConnectCommand = static_cast<CommandReqId>(0);
const common::CommandReqId CommandServerBase::kDisconnectCommand = static_cast<CommandReqId>(1);
const common::CommandReqId CommandServerBase::kAliveCommand = static_cast<CommandReqId>(2);

constexpr int kClientAliveTimeoutMsec = 8000;
constexpr unsigned kReconnectTimes = 10;



CommandServerBase::CommandServerBase(const std::string &address, unsigned int port) :
    context_(nullptr),
    main_socket_(nullptr),
    server_endpoint_("tcp://" + address + ":" + std::to_string(port)),
    server_port_(port),
    server_address_(address),
    server_working_(false),
    client_present_(false),
    disconnect_requested_(false)
{
}

const std::future<void> &CommandServerBase::getServerWorkerFuture() const {return this->server_worker_future_;}

const unsigned& CommandServerBase::getServerPort() const {return this->server_port_;}

const std::string& CommandServerBase::getServerAddress() const {return this->server_address_;}

const std::string& CommandServerBase::getServerEndpoint() const {return this->server_endpoint_;}

void CommandServerBase::startServer()
{
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
    // If server is already stopped, do nothing.
    if (!this->server_working_)
        return;

    // Set the shared working flag to false (is atomic).
    this->server_working_ = false;

    std::cout<<"Sending close";

    if (this->context_)
    {
        delete this->context_;
        context_ = nullptr;
    }

/*
    // Create an auxiliar client socket for closing the server.
    zmq::socket_t* socket = nullptr;
    try
    {
        socket = new zmq::socket_t(*this->context_, zmq::socket_type::req);
        socket->connect("tcp://127.0.0.1:" + std::to_string(this->server_port_));
        socket->set(zmq::sockopt::linger, 0);
    }
    catch (const zmq::error_t &error)
    {
        delete socket;
        socket = nullptr;
        // TODO: handle error
    }

    // Send 0. The msg will not be processed, only the shared variable.
    uint8_t msg = 0;
    try
    {
        socket->send(zmq::buffer(&msg, 1));
    }
    catch (const zmq::error_t &error)
    {
        // TODO: handle error
    }


    // Destroy the auxiliar interprocess socket.
    delete socket;

    if (this->server_worker_future_.valid())
    {
        // Set the future to wait.
        this->server_worker_future_.wait();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    if (this->context_)
    {
        delete this->context_;
        context_ = nullptr;      
    }
*/
}

void CommandServerBase::setCommandCallback(CommandReqId id, CommandCallback callback)
{
    this->commands_[id] = std::move(callback);
}

void CommandServerBase::setDeadClientCallback(OnDeadClientCallback functor)
{
    this->dead_client_callback_ = std::move(functor);
}

CommandServerBase::~CommandServerBase()
{
    // Stop the server (this function also deletes the pointers).
    this->stopServer();
}

void CommandServerBase::onNewConnection(const CommandExecReq&){}

void CommandServerBase::onDisconnected(const CommandExecReq&){}

void CommandServerBase::onCommandReceived(const CommandExecReq&){}

void CommandServerBase::onServerError(const zmq::error_t &error, const std::string&){}

common::CommandRepId CommandServerBase::execConnect(const CommandExecReq& cmd_req)
{
    // If client is already connected and connect is issued again, report and do nothing
    if (this->client_present_)
        return static_cast<common::CommandRepId>(CommandResult::ALREADY_CONNECTED);
    else
    {
        // Configuration for client present. The socket now has a tiemout to detect dead client.
        this->client_present_ = true;
        this->main_socket_->set(zmq::sockopt::rcvtimeo, kClientAliveTimeoutMsec);

        // Call to the internal callback.
        this->onNewConnection(cmd_req);

        // Call to the connection external callback.
        if (this->connect_callback_)
            this->connect_callback_(cmd_req);

        return static_cast<common::CommandRepId>(CommandResult::COMMAND_OK);
    }
}

common::CommandRepId CommandServerBase::execDisconnect(const CommandExecReq& cmd_req)
{

    // Check if client is already disconnected. In that case, report and do nothing.
    if (!this->client_present_)
        return static_cast<common::CommandRepId>(CommandResult::ALREADY_DISCONNECTED);
    else
    {
        // Call to the internal callback.
        this->onDisconnected(cmd_req);

        // Call to the connection external callback.
        if (this->disconnect_callback_)
            this->disconnect_callback_(cmd_req);

        // Request disconnection after sending the response
        this->disconnect_requested_ = true;

        return static_cast<common::CommandRepId>(CommandResult::COMMAND_OK);
    }
}

void CommandServerBase::serverWorker()
{
    // Return variables.
    CommandResult res;
    common::CommandRepId rep_id;

    void *answer_data = nullptr;
    std::size_t answer_size;

    // Set the working flag to true.
    this->server_working_ = true;


    // Start server socket
    this->resetSocket();


    // Server working loop.
    // If there is no client connected: wait for a client to connect or for an exit message.
    // If there is a client connected: set timeout, so if no command comes in time, consider client as dead.
    // The loop will be stopped if:
    // - The server socket could not be created.
    // - It is commanded to stop using the stopServer function
    while(this->main_socket_ && this->server_working_)
    {
        // Message container.
        CommandExecReq cmd_exec_req;

        // Receive the data.
        res = this->recvFromSocket(cmd_exec_req);


        // Process the data.
        if(!this->server_working_)
        {
            // If a message is received but server was commanded to stop, do nothing.
        }
        else if(res == CommandResult::TIMEOUT_REACHED)
        {
            std::cout<<"Dead client"<<std::endl;
            // Flush message queue and wait for client to come.
            this->client_present_ = false;
            this->main_socket_->set(zmq::sockopt::rcvtimeo, -1);

            //this->resetSocket();

            // Execute dead client callback if it exists.
            //if (this->dead_client_callback_)
            //    this->dead_client_callback_();
        }
        else if (res != CommandResult::COMMAND_OK)
        {
            // Send the current error.
            std::uint8_t res_buff[sizeof(CommandResult)];
            utils::binarySerializeDeserialize(&res, sizeof(CommandResult), res_buff);


            try{this->main_socket_->send(zmq::buffer(answer_data, answer_size));
            }  catch (const zmq::error_t &error)
            {
                std::cout<<"Sending error"<<std::endl;
                // TODO: handle/log error
            }
        }
        else if (res == CommandResult::COMMAND_OK)
        {
            try
            {
                // Reply id buffer.
                std::unique_ptr<std::uint8_t> rep_id_buff;

                // Execute the command.
                rep_id = this->executeCommand(cmd_exec_req, answer_data, answer_size);

                // Prepare the command result.
                CommandServerBase::prepareCommandResult(rep_id, rep_id_buff);
                zmq::message_t message_rep_id(rep_id_buff.get(), sizeof(common::CommandRepId));

                // Prepare the multipart msg.
                zmq::multipart_t multipart_msg;
                multipart_msg.add(std::move(message_rep_id));

                // Send the message.
                multipart_msg.send(*this->main_socket_);

                std::cout<<"Sending response: "<<rep_id<<std::endl;

            }
            catch (const zmq::error_t &error)
            {
                this->onServerError(error, "Error while sending a response.");
            }

            if (this->disconnect_requested_)
            {
                this->resetSocket();
                this->client_present_ = false;
                this->disconnect_requested_ = false;
            }
        }
    }

    if (this->main_socket_)
    {
        delete this->main_socket_;
        this->main_socket_ = nullptr;
    }
}

CommandServerBase::CommandResult CommandServerBase::recvFromSocket(CommandExecReq& msg)
{
    // Result variable.
    CommandResult result = CommandResult::COMMAND_OK;

    // Containers.
    bool recv_result;
    zmq::multipart_t multipart_msg;

    // Try to receive data. If an execption is thrown, receiving fails and an error code is generated.
    try
    {
        std::cout<<"Waiting command "<<multipart_msg.size()<<std::endl;

        recv_result = multipart_msg.recv(*(this->main_socket_));
        std::cout<<"New msg size: "<<multipart_msg.size()<<std::endl;
    }
    catch(zmq::error_t& error)
    {
        this->onServerError(error, "Error while receiving a request.");
        return CommandResult::INTERNAL_ZMQ_ERROR;
    }

    // Check for empty msg or timeout reached.
    if (multipart_msg.empty() && !recv_result)
        return CommandResult::TIMEOUT_REACHED;
    else if (multipart_msg.empty())
        return CommandResult::EMPTY_MSG;

    // Check if at least there are 3 parts.
    if (multipart_msg.size() == 3 || multipart_msg.size() == 4)
    {
        // Get the multipart data.
        zmq::message_t message_ip = multipart_msg.pop();
        zmq::message_t message_host = multipart_msg.pop();
        zmq::message_t message_command = multipart_msg.pop();

        // Get the sizes.
        size_t ip_size_bytes = message_ip.size();
        size_t host_size_bytes = message_host.size();
        size_t command_size_bytes = message_command.size();

        // First get the ip data.
        if (ip_size_bytes > 0)
            msg.client_ip = std::string(static_cast<char*>(message_ip.data()), ip_size_bytes);
        else
            return CommandResult::EMPTY_CLIENT_IP;

        // Get the host data.
        if (host_size_bytes > 0)
            msg.client_name = std::string(static_cast<char*>(message_host.data()), host_size_bytes);
        else
            return CommandResult::EMPTY_CLIENT_NAME;

        // Get the command.
        if (command_size_bytes == sizeof(common::CommandReqId))
        {
            common::CommandReqId id;
            utils::binarySerializeDeserialize(message_command.data(), sizeof(common::CommandReqId), &id);
            msg.command_id = id;
        }
        else
            return CommandResult::INVALID_COMMAND;

        if (multipart_msg.size() == 4)
        {
            zmq::message_t message_params = multipart_msg.pop();
            size_t params_size_bytes = message_params.size();

            if(params_size_bytes > 0)
            {
                std::unique_ptr<std::uint8_t> cmd_data = std::unique_ptr<std::uint8_t>(new std::uint8_t[params_size_bytes]);
                auto *params_pointer = static_cast<std::uint8_t*>(message_params.data());
                std::copy(params_pointer, params_pointer + params_size_bytes, cmd_data.get());
                msg.params = std::move(cmd_data);
            }
            else
                return CommandResult::EMPTY_PARAMS;
        }
    }
    else
        return CommandResult::INVALID_PARTS;

    return result;
}


void CommandServerBase::prepareCommandResult(common::CommandRepId result, std::unique_ptr<std::uint8_t>& data_out)
{
    data_out = std::unique_ptr<std::uint8_t>(new std::uint8_t[sizeof(CommandRepId)]);
    utils::binarySerializeDeserialize(&result, sizeof(CommandRepId), data_out.get());
}

common::CommandRepId CommandServerBase::executeCommand(const CommandExecReq& cmd_req, void* &data_out,
                                                       size_t& out_size_bytes)
{    
    // First of all, call to the internal callback.
    this->onCommandReceived(cmd_req);

    data_out = nullptr;
    out_size_bytes = 0;

    // Process the connect command.
    if (CommandServerBase::kConnectCommand == cmd_req.command_id)
    {
        return this->execConnect(cmd_req);
    }

    // Check if the client is connected.
    if(!this->client_present_)
        return static_cast<common::CommandRepId>(CommandResult::NOT_CONNECTED);

    // Process the rest of the base commands.
    if (CommandServerBase::kDisconnectCommand == cmd_req.command_id)
    {
        return this->execDisconnect(cmd_req);
    }
    else if (CommandServerBase::kAliveCommand == cmd_req.command_id)
    {
        return static_cast<common::CommandRepId>(CommandResult::COMMAND_OK);
    }
    else
    {
        // If there is a registered callback for requested command, execute it and return result
        // Otherwise, report not implemented command
        auto command_it = this->commands_.find(cmd_req.command_id);

        if (command_it != this->commands_.end())
        {
            return static_cast<common::CommandRepId>(
                command_it->second(cmd_req.params.get(), cmd_req.params_size, data_out, out_size_bytes));
        }
        else
        {
            return static_cast<common::CommandRepId>(CommandResult::NOT_IMPLEMENTED);
        }
    }

    // Return default no error.
    return static_cast<common::CommandRepId>(CommandServerBase::CommandResult::COMMAND_OK);
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
}

} // END NAMESPACES.
// =====================================================================================================================
