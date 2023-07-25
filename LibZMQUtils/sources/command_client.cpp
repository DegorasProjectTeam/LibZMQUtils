
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>

#include <iostream>

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>

#include "LibZMQUtils/CommandServerClient/command_client.h"
#include "LibZMQUtils/utils.h"

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

const int CommandClientBase::kClientAliveTimeoutMsec = 5000;
const int CommandClientBase::kClientSendAlivePeriodMsec = 3000;


CommandClientBase::CommandClientBase(const std::string &server_endpoint) :
    server_endpoint_(server_endpoint),
    context_(nullptr),
    socket_(nullptr),
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
    if (this->socket_)
        return false;

    // Create the ZMQ context.
    if (!this->context_)
        this->context_ = new zmq::context_t(1);

    try
    {
        this->socket_ = new zmq::socket_t(*this->context_, zmq::socket_type::req);
        this->socket_->connect(this->server_endpoint_);
        // Set timeout so socket will not wait for answer more than client alive timeout.
        this->socket_->set(zmq::sockopt::rcvtimeo, CommandClientBase::kClientAliveTimeoutMsec);
        this->socket_->set(zmq::sockopt::linger, 0);
    }
    catch (const zmq::error_t &error)
    {
        delete this->socket_;
        this->socket_ = nullptr;

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
    if (!this->socket_)
        return;


    // Destroy the  socket.
    delete this->socket_;
    this->socket_ = nullptr;

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
    if (this->socket_)
    {
        // Destroy the socket and create again to flush.
        delete this->socket_;

        std::this_thread::sleep_for(std::chrono::milliseconds(1050));

        try
        {
            this->socket_ = new zmq::socket_t(*this->context_, zmq::socket_type::req);
            this->socket_->connect(this->server_endpoint_);
            // Set timeout so socket will not wait for answer more than client alive timeout.
            this->socket_->set(zmq::sockopt::rcvtimeo, CommandClientBase::kClientAliveTimeoutMsec);
            this->socket_->set(zmq::sockopt::linger, 0);
        }
        catch (const zmq::error_t &error)
        {
            delete this->socket_;
            this->socket_ = nullptr;

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

int CommandClientBase::sendCommand(const CommandData& msg, void* &data_out, size_t &out_bytes)
{
    if (!this->socket_)
        return -1;
    try
    {

        zmq::multipart_t multipart_msg(this->prepareMessage(msg));

        // Send the multiple messages.
        multipart_msg.send(*this->socket_);

        std::cout<<"Data sent"<<std::endl;

    }  catch (const zmq::error_t &error)
    {
        // TODO: handle error
        return error.num();
    }

    std::cout<<"Waiting response"<<std::endl;

    int res = this->recvFromSocket(this->socket_, data_out, out_bytes);

    if (this->auto_alive_working_)
        this->auto_alive_cv_.notify_one();

    return res;

}

int CommandClientBase::sendBadCommand1(void* &, size_t &)
{
    return 0;
}

int CommandClientBase::recvFromSocket(zmq::socket_t *socket, void *&data, size_t &data_size_bytes) const
{
    // Reset output variables
    data = nullptr;
    data_size_bytes = 0;

    // Try to receive data. If an execption is thrown, receiving fails and an error code is generated.
    int result = 0;
    zmq::recv_result_t recv_result;
    zmq::message_t message_recv;

    try
    {
        recv_result = socket->recv(message_recv);
    }
    catch(zmq::error_t& error)
    {
        result = error.num();
    }

    // Return error code if receiving fails or if no data was received
    if (!recv_result.has_value())
    {
        result = EAGAIN;
    }
    else if (0 == result)
    {
        // If data was received, copy received data to out parameters and return not error code
        data_size_bytes = message_recv.size();
        if (data_size_bytes > 0)
        {
            uint8_t *msg_data = new uint8_t[data_size_bytes];
            uint8_t *message_recv_pointer = static_cast<uint8_t*>(message_recv.data());
            std::copy(message_recv_pointer, message_recv_pointer + data_size_bytes, msg_data);
            data = msg_data;
        }
    }

    return result;
}

void CommandClientBase::sendAliveCallback()
{
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
    alive_socket->set(zmq::sockopt::rcvtimeo, CommandClientBase::kClientAliveTimeoutMsec);
    alive_socket->set(zmq::sockopt::linger, 0);

    while(this->auto_alive_working_)
    {
        auto res =
            this->auto_alive_cv_.wait_for(lk, std::chrono::milliseconds(CommandClientBase::kClientSendAlivePeriodMsec));

        if (std::cv_status::timeout == res)
        {
            msg = this->prepareMessage(
                            CommandData(static_cast<common::CommandType>(BaseServerCommand::REQ_ALIVE)));
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

                if (0 == recv_result && out_size == sizeof(CommandClientBase::CommandError))
                {
                    CommandClientBase::CommandError error;


                    zmqutils::utils::binarySerializeDeserialize(
                                data_bytes, sizeof(CommandClientBase::CommandError), &error);

                    recv_success = error == CommandClientBase::CommandError::NOT_ERROR;

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
}

zmq::multipart_t CommandClientBase::prepareMessage(const CommandData &msg)
{
    // Prepare the ip data.
    zmq::message_t message_ip(this->client_info_.ip.begin(), this->client_info_.ip.end());
    // Prepare the hostname data.
    zmq::message_t message_host(this->client_info_.hostname.begin(), this->client_info_.hostname.end());
    // Prepare the pid data.
    zmq::message_t message_pid(this->client_info_.pid.begin(), this->client_info_.pid.end());
    // Prepare the command data.
    std::uint8_t command_buffer[sizeof(common::CmdRequestId)];
    zmqutils::utils::binarySerializeDeserialize(&msg.command_id, sizeof(common::CmdRequestId), command_buffer);
    zmq::message_t message_command(&command_buffer, sizeof(common::CmdRequestId));


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
