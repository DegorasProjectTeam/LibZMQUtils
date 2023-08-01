
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




CommandClientBase::CommandClientBase(const std::string &server_endpoint, const std::string &client_name) :
    server_endpoint_(server_endpoint),
    client_name_(client_name),
    context_(nullptr),
    client_socket_(nullptr),
    flag_alive_woking_(true),
    flag_alive_callbacks_(true)
{

}

CommandClientBase::~CommandClientBase()
{
    // TODO stop autoalive.

    // Force the stop client execution.
    // Warning: In this case the onClientStop callback can't be executed.
    this->internalStop();
}

const common::HostClientInfo &CommandClientBase::getClientInfo() const
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    return this->client_info_;
}

const common::HostClientInfo &CommandClientBase::internalGetClientInfo() const
{
    return this->client_info_;
}

bool CommandClientBase::startClient(const std::string& interface_name)
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Auxiliar variables.
    std::string ip, name, pid;

    // Get the client interfaces.
    std::vector<utils::NetworkAdapterInfo> interfcs = utils::getHostIPsWithInterfaces();

    // Search the interface we need.
    auto it = std::find_if(interfcs.begin(), interfcs.end(),
                           [&interface_name](const utils::NetworkAdapterInfo& info)
                           {return info.name == interface_name;});

    // Check if the interface exists and store it.
    if (it == interfcs.end())
        return false;
    ip = it->ip;

    // Get the host name.
    name = utils::getHostname();

    // Get the current pid.
    pid = std::to_string(utils::getCurrentPID());

    // Store the info.
    this->client_info_ = common::HostClientInfo(ip, name, pid);

    // If server is already started, do nothing
    if (this->client_socket_)
        return false;

    // Create the ZMQ context.
    if (!this->context_)
        this->context_ = new zmq::context_t(1);

    // Create the ZMQ socket.
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
        if(this->client_socket_)
        {
            delete this->client_socket_;
            this->client_socket_ = nullptr;
        }

        // Call to the internal callback.
        this->onClientError(error, "Error while creating the client.");
        return false;
    }

    // Call to the internal callback.
    this->onClientStart();

    // All ok.
    return true;
}

void CommandClientBase::stopClient()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Call to the internal stop.
    this->internalStop();

    // Call to the internal callback.
    this->onClientStop();
}

bool CommandClientBase::resetClient()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Check the client.
    if (this->client_socket_)
    {
        // Close the client.
        this->client_socket_->close();

        // Destroy the socket and create again to flush.
        delete this->client_socket_;

        // Create the ZMQ socket.
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
            if(this->client_socket_)
            {
                delete this->client_socket_;
                this->client_socket_ = nullptr;
            }

            // Call to the internal callback.
            this->onClientError(error, "Error while creating the client.");
            return false;
        }
    }

    // All ok
    return true;
}

void CommandClientBase::setAliveCallbacksEnabled(bool enable)
{
    this->flag_alive_callbacks_ = enable;
}

void CommandClientBase::setAutomaticAliveEnabled(bool enable)
{
    std::unique_lock<std::mutex> lock(this->mtx_);
    enable ? this->startAutoAlive() : this->stopAutoAlive();
}

const std::string &CommandClientBase::getServerEndpoint() const
{
    // NOTE: Mutex is not neccesary here.
    return this->server_endpoint_;
}

const std::string &CommandClientBase::getClientName() const
{
    // NOTE: Mutex is not neccesary here.
    return this->client_name_;
}

void CommandClientBase::startAutoAlive()
{
    this->flag_alive_woking_ = true;
    this->auto_alive_future_ = std::async(std::launch::async, [this]{this->sendAliveCallback();});
}

void CommandClientBase::stopAutoAlive()
{
    if (this->flag_alive_woking_)
    {
        this->flag_alive_woking_ = false;
        this->auto_alive_cv_.notify_all();
        this->auto_alive_future_.wait();
    }
}

ClientResult CommandClientBase::sendCommand(const RequestData& request, CommandReply& reply)
{
    // Result.
    ClientResult result;

    // Clean the reply.
    reply = CommandReply();

    // Check if we start the client.
    if (!this->client_socket_)
        return ClientResult::CLIENT_STOPPED;

    // Send the command.
    try
    {
        // Prepare the multipart msg.
        zmq::multipart_t multipart_msg(this->prepareMessage(request));

        // Internal send callback.
        this->onSendingCommand(request);

        // Send the multiple messages.
        multipart_msg.send(*this->client_socket_);

    }
    catch (const zmq::error_t &error)
    {
        // Call to the error callback.
        this->onClientError(error, "Error while sending a request.");
        return ClientResult::INTERNAL_ZMQ_ERROR;
    }

    // Now we need to wait the server response.

    // Call to the internal waiting command callback (check first the last request).
    if (static_cast<ServerCommand>(request.command) != ServerCommand::REQ_ALIVE || this->flag_alive_callbacks_)
        this->onWaitingReply();

    // Receive the data.
    result = this->recvFromSocket(reply);

    // Check for errors.
    if (result == ClientResult::TIMEOUT_REACHED)
    {
        // Call to the internall callback and reset the socket.
        // NOTE: The client reset is neccesary for flush the ZMQ internal
        this->onDeadServer();
        this->resetClient();
    }
    else if(result != ClientResult::COMMAND_OK && result != ClientResult::INTERNAL_ZMQ_ERROR)
    {
        // Internal callback.
        this->onInvalidMsgReceived(reply);
    }

    // Use the cv for notify the auto alive worker.
    if (this->flag_alive_woking_)
        this->auto_alive_cv_.notify_one();

    // Return the result.
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
        // Wait the reply.
        recv_result = multipart_msg.recv(*this->client_socket_);
    }
    catch(zmq::error_t& error)
    {
        // Check if we want too close the client.
        // The error code is for ZMQ EFSM error.
        if(error.num() == common::kZmqEFSMError && !this->flag_client_working_)
            return ClientResult::CLIENT_STOPPED;

        // Else, call to error callback.
        this->onClientError(error, "Error while receiving a reply.");
        return ClientResult::INTERNAL_ZMQ_ERROR;
    }

    // Check for empty msg or timeout reached.
    if (multipart_msg.empty() && !recv_result)
    {
        // Call to the timeout callback
        return ClientResult::TIMEOUT_REACHED;
    }
    else if (multipart_msg.empty())
        return ClientResult::EMPTY_MSG;

    // Check the multipart msg size.
    if (multipart_msg.size() == 1 || multipart_msg.size() == 2)
    {
        // Get the multipart data.
        zmq::message_t msg_res = multipart_msg.pop();

        // Get the command.
        if (msg_res.size()== sizeof(ServerCommand))
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

void CommandClientBase::internalStop()
{
    // If server is already stopped, do nothing.
    if (!this->flag_client_working_)
        return;

    // Set the shared working flag to false (is atomic).
    this->flag_client_working_ = false;

    // Delete context.
    if (this->context_)
    {
        delete this->context_;
        this->context_ = nullptr;
    }

    // Delete the socket.
    if(this->client_socket_)
    {
        this->client_socket_->close();
        delete this->client_socket_;
        this->client_socket_ = nullptr;
    }
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

zmq::multipart_t CommandClientBase::prepareMessage(const RequestData &request)
{
    // Serialize the ip.
    utils::BinarySerializer serializer;

    size_t size = serializer.write(this->client_info_.ip);


    zmq::message_t message_ip(serializer.release(), size);

    // Prepare the ip data.
    // Prepare the hostname data.
    zmq::message_t message_host(this->client_info_.hostname.begin(), this->client_info_.hostname.end());
    // Prepare the pid data.
    zmq::message_t message_pid(this->client_info_.pid.begin(), this->client_info_.pid.end());

    serializer.clearData();

    serializer.write(request.command);


    zmq::message_t message_command(serializer.release(), sizeof(common::CommandType));


    // Prepare the multipart msg.
    zmq::multipart_t multipart_msg;
    multipart_msg.add(std::move(message_ip));
    multipart_msg.add(std::move(message_host));
    multipart_msg.add(std::move(message_pid));
    multipart_msg.add(std::move(message_command));

    // Add command parameters if they exist
    if (request.params_size > 0)
    {
        // Prepare the command parameters
        zmq::message_t message_params(request.params.get(), request.params_size);
        multipart_msg.add(std::move(message_params));
    }

    return multipart_msg;
}

} // END NAMESPACES.
// =====================================================================================================================
