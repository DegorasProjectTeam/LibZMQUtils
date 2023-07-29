
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>

#include <iostream>

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>

#include "LibZMQUtils/CommandServerClient/command_client_base.h"
#include "LibZMQUtils/Utilities/utils.h"
#include "LibZMQUtils/Utilities/binary_serializer.h"

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================




CommandClientBase::CommandClientBase(const std::string &server_endpoint) :
    server_endpoint_(server_endpoint),
    context_(nullptr),
    client_socket_(nullptr),
    auto_alive_working_(false)
{

}

CommandClientBase::~CommandClientBase()
{
    if (this->auto_alive_working_)
        this->stopAutoAlive();
    this->stopClient();
}

bool CommandClientBase::startClient(const std::string& interface_name)
{
    // Auxiliar variables.
    std::string ip, name, pid;

    // Get the client ip.
    std::vector<utils::NetworkAdapterInfo> interfcs = utils::getHostIPsWithInterfaces();
    auto it = std::find_if(interfcs.begin(), interfcs.end(), [&interface_name](const utils::NetworkAdapterInfo& info)
                           {return info.name == interface_name;});
    if (it == interfcs.end())
        return false;
    ip = it->ip;

    // Get the host name.
    name = utils::getHostname();

    // Get the current pid.
    pid = std::to_string(utils::getCurrentPID());

    // Store the info.
    this->client_info_ = common::HostClient(ip, name, pid);

    std::cout<<client_info_.id<<std::endl;

    // If server is already started, do nothing
    if (this->client_socket_)
        return false;

    // Create the ZMQ context.
    if (!this->context_)
        this->context_ = new zmq::context_t(1);

    try
    {
        this->client_socket_ = new zmq::socket_t(*this->context_, zmq::socket_type::req);
        this->client_socket_->connect(this->server_endpoint_);
        // Set timeout so socket will not wait for answer more than client alive timeout.
        this->client_socket_->set(zmq::sockopt::rcvtimeo, common::kDefaultServerAliveTimeoutMsec);
        this->client_socket_->set(zmq::sockopt::linger, 0);
    }
    catch (const zmq::error_t &error)
    {
        delete this->client_socket_;
        this->client_socket_ = nullptr;

        std::cerr << "Error at socket creation: " << error.num();
        // TODO: handle error
        return false;
    }

    // All ok.
    return true;
}

void CommandClientBase::stopClient()
{
    // If server is already stopped, do nothing.
    if (!this->client_socket_)
        return;


    // Destroy the  socket.
    delete this->client_socket_;
    this->client_socket_ = nullptr;

    std::this_thread::sleep_for(std::chrono::milliseconds(1050));

    // Delete context

    if (this->context_)
    {
        delete this->context_;
        this->context_ = nullptr;
    }
}

void CommandClientBase::resetClient()
{
    if (this->client_socket_)
    {
        // Destroy the socket and create again to flush.
        delete this->client_socket_;

        std::this_thread::sleep_for(std::chrono::milliseconds(1050));

        try
        {
            this->client_socket_ = new zmq::socket_t(*this->context_, zmq::socket_type::req);
            this->client_socket_->connect(this->server_endpoint_);
            // Set timeout so socket will not wait for answer more than server alive timeout.
            this->client_socket_->set(zmq::sockopt::rcvtimeo, common::kDefaultServerAliveTimeoutMsec);
            this->client_socket_->set(zmq::sockopt::linger, 0);
        }
        catch (const zmq::error_t &error)
        {
            delete this->client_socket_;
            this->client_socket_ = nullptr;

            std::cerr << "Error at socket creation: " << error.num();
            // TODO: handle error
        }
    }
}

void CommandClientBase::startAutoAlive()
{
    this->auto_alive_working_ = true;
    this->auto_alive_future_ = std::async(std::launch::async, [this]{this->sendAliveCallback();});
}

void CommandClientBase::stopAutoAlive()
{
    if (this->auto_alive_working_)
    {
        this->auto_alive_working_ = false;
        this->auto_alive_cv_.notify_all();
        this->auto_alive_future_.wait();
    }
}

void CommandClientBase::setClientHostIP(const std::string&){}

void CommandClientBase::setClientId(const std::string &){}

ClientResult CommandClientBase::sendCommand(const RequestData& msg, CommandReply& reply)
{
    // Result.
    ClientResult result;

    // Check if we start the client.
    if (!this->client_socket_)
        return ClientResult::CLIENT_STOPPED;

    // Send the command.
    try
    {

        zmq::multipart_t multipart_msg(this->prepareMessage(msg));

        // Internal send callback.
        this->onSendingCommand(msg, multipart_msg);

        // Send the multiple messages.
        multipart_msg.send(*this->client_socket_);

    }  catch (const zmq::error_t &error)
    {
        // TODO: handle error
        std::cout<<error.what()<<std::endl;
        return ClientResult::INTERNAL_ZMQ_ERROR;
    }

    std::cout<<"Waiting response"<<std::endl;


    // TODO multipart.

    result = this->recvFromSocket(reply);





    if (this->auto_alive_working_)
        this->auto_alive_cv_.notify_one();

    return result;

}

ClientResult CommandClientBase::recvFromSocket(CommandReply& reply)
{
    // Result variable.
    ClientResult result = ClientResult::COMMAND_OK;

    // Containers.
    bool recv_result;
    zmq::multipart_t multipart_msg;

    // Try to receive data. If an execption is thrown, receiving fails and an error code is generated.
    try
    {
        // Call to the internal waiting command callback. TODO
        this->onWaitingReply();

        // Wait the reply.
        recv_result = multipart_msg.recv(*this->client_socket_);

        // Store the raw data.
        reply.raw_msg = multipart_msg.clone();
    }
    catch(zmq::error_t& error)
    {
        // Call to error callback. TODO
        //this->onClientError(error, "Error while receiving a request.");

        std::cout<<"INTERNAL ERRROR "<<error.what()<<std::endl;

        return ClientResult::INTERNAL_ZMQ_ERROR;
    }

    // Check for empty msg or timeout reached.
    if (multipart_msg.empty() && !recv_result)
        return ClientResult::TIMEOUT_REACHED;
    else if (multipart_msg.empty())
        return ClientResult::EMPTY_MSG;

    // Check the multipart msg size.
    if (multipart_msg.size() == 1 || multipart_msg.size() == 2)
    {
        // Get the multipart data.
        zmq::message_t msg_res = multipart_msg.pop();

        // Get the sizes.
        size_t result_size_bytes = msg_res.size();

        // Get the command.
        if (result_size_bytes == sizeof(ServerCommand))
        {
            utils::BinarySerializer::fastDeserialization(msg_res.data(), msg_res.size(), reply.result);
        }
        else
            return ClientResult::INVALID_MSG;

        // If there is still one more part, they are the parameters.
        if (multipart_msg.size() == 1)
        {
            // Get the message and the size.
            zmq::message_t msg_params = multipart_msg.pop();

            // Check the parameters.
            if(msg_params.size() > 0)
            {
                // Get and store the parameters data.
                utils::BinarySerializer serializer(msg_params.data(), msg_params.size());
                reply.params = serializer.moveUnique(reply.params_size);
            }
            else
                return ClientResult::EMPTY_PARAMS;
        }
    }
    else
        return ClientResult::INVALID_PARTS;

    // Return the result.
    return result;
}

void CommandClientBase::sendAliveCallback()
{
    /*
    std::mutex m;
    std::unique_lock<std::mutex> lk(m);
    bool send_success = true;
    bool recv_success = true;
    void *data_out;
    size_t out_size;
    zmq::multipart_t msg;
    zmq::socket_t *alive_socket = new zmq::socket_t(*this->context_, zmq::socket_type::req);
    alive_socket->connect(this->server_endpoint_);
    // Set timeout so socket will not wait for answer more than client alive timeout.
    alive_socket->set(zmq::sockopt::rcvtimeo, common::kDefaultServerAliveTimeoutMsec);
    alive_socket->set(zmq::sockopt::linger, 0);

    while(this->auto_alive_working_)
    {
        auto res =
            this->auto_alive_cv_.wait_for(lk, std::chrono::milliseconds(common::kClientAlivePeriodMsec));

        if (std::cv_status::timeout == res)
        {
            msg = this->prepareMessage(
                            RequestData(static_cast<common::CommandType>(ServerCommand::REQ_ALIVE)));
            try
            {
                msg.send(*alive_socket);
            }  catch (const zmq::error_t &error)
            {
                // TODO: handle error
                std::cerr << "Failed to automatically send alive command with error: " << error.num() << std::endl;
                send_success = false;
            }

            if (send_success)
            {
                auto recv_result = this->recvFromSocket(alive_socket, data_out, out_size);
                auto *data_bytes = static_cast<std::uint8_t*>(data_out);

                if (0 == recv_result && out_size == sizeof(common::ServerResult))
                {
                    common::ServerResult result;


                    zmqutils::utils::binarySerializeDeserialize(
                                data_bytes, sizeof(common::CommandReply), &result);

                    recv_success = result == common::ServerResult::COMMAND_OK;

                }
                else
                {
                    std::cerr << "Auto alive message answer receive failed" << std::endl;
                    recv_success = false;
                }

                delete[] data_bytes;
            }

            if (!send_success || !recv_success)
            {
                std::cerr << "Failed auto sending alive message. Process will be stopped." << std::endl;
                this->auto_alive_working_ = false;
            }

        }

    }

    delete alive_socket;
    */
}

zmq::multipart_t CommandClientBase::prepareMessage(const RequestData &msg)
{
    // Prepare the ip data.
    zmq::message_t message_ip(this->client_info_.ip.begin(), this->client_info_.ip.end());
    // Prepare the hostname data.
    zmq::message_t message_host(this->client_info_.hostname.begin(), this->client_info_.hostname.end());
    // Prepare the pid data.
    zmq::message_t message_pid(this->client_info_.pid.begin(), this->client_info_.pid.end());
    // Prepare the command data.
    std::uint8_t command_buffer[sizeof(common::CommandType)];
    zmqutils::utils::binarySerializeDeserialize(&msg.command, sizeof(common::CommandType), command_buffer);
    zmq::message_t message_command(&command_buffer, sizeof(common::CommandType));


    // Prepare the multipart msg.
    zmq::multipart_t multipart_msg;
    multipart_msg.add(std::move(message_ip));
    multipart_msg.add(std::move(message_host));
    multipart_msg.add(std::move(message_pid));
    multipart_msg.add(std::move(message_command));

    // Add command parameters if they exist
    if (msg.params_size > 0)
    {
        // Prepare the command parameters
        zmq::message_t message_params(msg.params.get(), msg.params_size);
        multipart_msg.add(std::move(message_params));
    }

    return multipart_msg;
}

} // END NAMESPACES.
// =====================================================================================================================
