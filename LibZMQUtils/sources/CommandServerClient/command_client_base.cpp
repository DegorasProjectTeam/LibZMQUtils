
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
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================




CommandClientBase::CommandClientBase(const std::string& server_endpoint,
                                     const std::string& client_name,
                                     const std::string& interf_name) :
    client_name_(client_name),
    server_endpoint_(server_endpoint),
    client_socket_(nullptr),
    rep_close_socket_(nullptr),
    req_close_socket_(nullptr),
    flag_client_closed_(true),
    flag_autoalive_enabled_(true),
    flag_alive_callbacks_(true)
{
    // Auxiliar variables and containers.
    std::string ip, hostname, pid;
    utils::NetworkAdapterInfo sel_interf;

    // Generate a unique UUID (v4) for the client.
    utils::UUID uuid = utils::UUIDGenerator::getInstance().generateUUIDv4();

    // Get the client interfaces.
    std::vector<utils::NetworkAdapterInfo> interfcs = utils::getHostIPsWithInterfaces();

    // Check if we have active interfaces.
    if(interfcs.empty())
        throw std::invalid_argument("CommandClientBase: No active network interface found.");

    // If no interface name provided, use the first active one.
    if (interf_name.empty())
    {
        // Store the interface.
        sel_interf = interfcs.front();
    }
    else
    {
        // Search the interface we need.
        auto it = std::find_if(interfcs.begin(), interfcs.end(),
                               [&interf_name](const utils::NetworkAdapterInfo& info)
                               {return info.name == interf_name;});

        // Check if the interface exists.
        if (it == interfcs.end())
            throw std::invalid_argument("CommandClientBase: Network interface not found <" + interf_name + ">.");

        // Store the interface.
        sel_interf = *it;
    }

    // Store the ip, pid and hostname.
    ip = sel_interf.ip;
    hostname = utils::getHostname();
    pid = std::to_string(utils::getCurrentPID());

    // Store all the client info.
    this->client_info_ = common::HostInfo(uuid, ip, pid, hostname, this->client_name_);
}

CommandClientBase::~CommandClientBase()
{
    // TODO stop autoalive.

    // Force the stop client execution.
    // Warning: In this case the onClientStop callback can't be executed.
    this->internalStopClient();
}

const common::HostInfo &CommandClientBase::getClientInfo() const
{
    return this->client_info_;
}

bool CommandClientBase::startClient()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // If server is already started, do nothing
    if (this->client_socket_)
        return false;

    // Start the client.
    return this->internalResetClient() ? (this->onClientStart(), true) : false;
}

void CommandClientBase::stopClient()
{
    // Atomic.
    // If server is already stopped, do nothing.
    if (!this->flag_client_working_)
        return;

    // Call to the internal stop.
    this->internalStopClient();

    // Call to the internal callback.
    this->onClientStop();

    // Update the close flag.
    this->flag_client_closed_ = true;

    std::unique_lock<std::mutex> lock(client_close_mtx_);
    this->client_close_cv_.notify_all();
}

bool CommandClientBase::resetClient()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Call to the internal method.
    return this->internalResetClient();
}

bool CommandClientBase::internalResetClient()
{
    // Close the previous sockets to flush.
    this->deleteSockets();

    // Create the ZMQ socket.
    try
    {
        // Zmq client socket.
        this->client_socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::req);
        this->client_socket_->connect(this->server_endpoint_);
        // Set timeout so socket will not wait for answer more than client alive timeout.
        //this->client_socket_->set(zmq::sockopt::rcvtimeo, common::kDefaultServerAliveTimeoutMsec);
        this->client_socket_->set(zmq::sockopt::linger, 0);

        // Bind the REP close socket to an internal endpoint.
        rep_close_socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::rep);
        rep_close_socket_->bind("inproc://close"+this->client_info_.uuid.toRFC4122String());
        rep_close_socket_->set(zmq::sockopt::linger, 0);

        // Connect the REQ close socket to the same internal endpoint.
        req_close_socket_ = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::req);
        req_close_socket_->connect("inproc://close"+this->client_info_.uuid.toRFC4122String());
        req_close_socket_->set(zmq::sockopt::linger, 0);

        // Update the working flag.
        this->flag_client_working_ = true;
        this->flag_client_closed_ = false;
    }
    catch (const zmq::error_t &error)
    {
        // Delete the sockets.
        this->deleteSockets();

        // Update the working flag.
        this->flag_client_working_ = false;
        this->flag_client_closed_ = true;

        // Call to the internal callback.
        this->onClientError(error, "CommandClientBase: Error while creating the client.");
        return false;
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
    this->flag_autoalive_enabled_ = true;
    //this->auto_alive_future_ = std::async(std::launch::async, [this]{this->aliveWorker();});
}

void CommandClientBase::stopAutoAlive()
{
    if (this->flag_autoalive_enabled_)
    {
        this->flag_autoalive_enabled_ = false;
        this->auto_alive_cv_.notify_all();
        this->auto_alive_future_.wait();
    }
}

ClientResult CommandClientBase::sendCommand(const RequestData& request, CommandReply& reply)
{
    // Result.
    ClientResult result = ClientResult::COMMAND_OK;

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

//        bool more = multipart_msg.size() > 0;
//        while (more)
//        {
//            auto msg = multipart_msg.pop();
//            more = multipart_msg.size() > 0;
//            this->client_socket_->send(msg,  zmq::send_flags::none);
//        }

//        auto msg_1 = multipart_msg.pop();
//        this->client_socket_->send(msg_1, zmq::send_flags::sndmore);
        //auto msg_2 = multipart_msg.pop();
        //this->client_socket_->send(msg_2, zmq::send_flags::none);
        // Send the multiple messages.

        //multipart_msg.pop();
        multipart_msg.send(*this->client_socket_);
    }
    catch (const zmq::error_t &error)
    {
        // Call to the error callback and stop the client for safety.
        this->onClientError(error, "CommandClientBase: Error while sending a request. Stopping the client.");
        this->internalStopClient();
        return ClientResult::INTERNAL_ZMQ_ERROR;
    }

    // Now we need to wait the server response.

    // Call to the internal waiting command callback (check first the last request).
    if (static_cast<ServerCommand>(request.command) != ServerCommand::REQ_ALIVE || this->flag_alive_callbacks_)
        this->onWaitingReply();

    // Receive the data.    
    fut_recv_ = std::async(std::launch::async, &CommandClientBase::recvFromSocket, this, std::ref(reply));

    using namespace std::chrono_literals;

    // Retrieve the result and reset the future
    while (true)
    {
        if (fut_recv_.wait_for(20ms) == std::future_status::ready)
        {
            result = fut_recv_.get();
            break;
        }
    }

    // Use the cv for notify the auto alive worker.
    // TODO
    //    if (this->flag_autoalive_enabled_)
    //        this->auto_alive_cv_.notify_one();

    // Check if the client stopped.
    if (result == ClientResult::CLIENT_STOPPED)
        return result;

    // Check if was a timeout.
    if (result == ClientResult::TIMEOUT_REACHED)
    {
        // Call to the internall callback and reset the socket.
        // NOTE: The client reset is neccesary for flush the ZMQ internal
        this->onDeadServer();
        this->internalResetClient();
    }

    // Check if was ok.
    if(result == ClientResult::COMMAND_OK)
    {
        // Internal callback.
        this->onReplyReceived(reply);
    }
    else if(result != ClientResult::COMMAND_OK && result != ClientResult::INTERNAL_ZMQ_ERROR)
    {
        // Internal callback.
        this->onInvalidMsgReceived(reply);
    }

    // Return the result.
    return result;
}

bool CommandClientBase::waitForClose(std::chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(client_close_mtx_);
    if (timeout == std::chrono::milliseconds::zero()) {
        client_close_cv_.wait(lock, [this]{ return this->flag_client_closed_.load(); });
        return true;
    } else {
        return client_close_cv_.wait_for(lock, timeout, [this]{ return this->flag_client_closed_.load(); });
    }
}


//ClientResult CommandClientBase::recvFromSocket(CommandReply& reply)
//{
//    // Result variable.
//    ClientResult result = ClientResult::COMMAND_OK;

//    // Containers.
//    bool recv_result;
//    zmq::multipart_t multipart_msg;

//    // Try to receive data. If an execption is thrown, receiving fails and an error code is generated.
//    try
//    {
//        // Wait the reply.
//        recv_result = multipart_msg.recv(*this->client_socket_);






//    }
//    catch(zmq::error_t& error)
//    {
//        // Check if we want too close the client.
//        // The error code is for ZMQ EFSM error.
//        if(error.num() == common::kZmqEFSMError && !this->flag_client_working_)
//            return ClientResult::CLIENT_STOPPED;

//        // Else, call to error callback.
//        this->onClientError(error, "Error while receiving a reply.");
//        return ClientResult::INTERNAL_ZMQ_ERROR;
//    }

//    // Check for empty msg or timeout reached.
//    if (multipart_msg.empty() && !recv_result)
//    {
//        // Call to the timeout callback
//        return ClientResult::TIMEOUT_REACHED;
//    }
//    else if (multipart_msg.empty())
//        return ClientResult::EMPTY_MSG;

//    // Check the multipart msg size.
//    if (multipart_msg.size() == 1 || multipart_msg.size() == 2)
//    {
//        // Get the multipart data.
//        zmq::message_t msg_res = multipart_msg.pop();

//        // Get the command.
//        if (msg_res.size()== sizeof(ServerCommand))
//        {
//            utils::BinarySerializer::fastDeserialization(msg_res.data(), msg_res.size(), reply.result);
//        }
//        else
//            return ClientResult::INVALID_MSG;

//        // If there is still one more part, they are the parameters.
//        if (multipart_msg.size() == 1)
//        {
//            // Get the message and the size.
//            zmq::message_t msg_params = multipart_msg.pop();

//            // Check the parameters.
//            if(msg_params.size() > 0)
//            {
//                // Get and store the parameters data.
//                utils::BinarySerializer serializer(msg_params.data(), msg_params.size());
//                reply.params = serializer.moveUnique(reply.params_size);
//            }
//            else
//                return ClientResult::EMPTY_PARAMS;
//        }
//    }
//    else
//        return ClientResult::INVALID_PARTS;

//    // Return the result.
//    return result;
//}

ClientResult CommandClientBase::recvFromSocket(CommandReply& reply)
{
    // Prepare the poller.
    std::vector<zmq::pollitem_t> items = {
        { static_cast<void*>(*client_socket_), 0, ZMQ_POLLIN, 0 },
        { static_cast<void*>(*rep_close_socket_), 0, ZMQ_POLLIN, 0 }};

    // Start time for check the timeout.
    auto start = std::chrono::steady_clock::now();

    // Poller loop.
    while(true)
    {
        try
        {
            // Use zmq::poll to set a timeout for receiving a message
            zmq::poll(items.data(), items.size(), std::chrono::milliseconds(common::kDefaultServerAliveTimeoutMsec));

            // Check if we must to close.
            if(!this->flag_client_working_ || (items[1].revents & ZMQ_POLLIN))
                return ClientResult::CLIENT_STOPPED;

            // Calculate elapsed time.
            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            // Check for timeout.
            if (elapsed.count() >= common::kDefaultServerAliveTimeoutMsec)
                return ClientResult::TIMEOUT_REACHED;

            // We have data.
            if (items[0].revents & ZMQ_POLLIN)
            {
                // Message has been received, try to process it
                zmq::multipart_t multipart_msg;
                multipart_msg.recv(*this->client_socket_);

                // Check for empty msg or timeout reached.
                if (multipart_msg.empty())
                    return ClientResult::EMPTY_MSG;

                // Check the multipart msg size.
                if (multipart_msg.size() != 1 && multipart_msg.size() != 2)
                    return ClientResult::INVALID_PARTS;

                // Get the multipart data.
                zmq::message_t msg_res = multipart_msg.pop();

                // Check the result size.
                if (msg_res.size() != sizeof(utils::BinarySerializer::ElementSize) + sizeof(common::ResultType))
                    return ClientResult::INVALID_MSG;

                // Get the command.
                utils::BinarySerializer::fastDeserialization(msg_res.data(), msg_res.size(), reply.result);

                // If there is still one more part, they are the parameters.
                if (multipart_msg.size() == 1)
                {
                    // Get the message and the size.
                    zmq::message_t msg_params = multipart_msg.pop();

                    // Check the parameters.
                    if(msg_params.size() == 0)
                        return ClientResult::EMPTY_PARAMS;

                    // Get and store the parameters data.
                    utils::BinarySerializer serializer(msg_params.data(), msg_params.size());
                    reply.params = serializer.moveUnique(reply.params_size);
                }

                // All ok.
                return ClientResult::COMMAND_OK;
            }
        }
        catch(zmq::error_t& error)
        {
            // Check if we want too close the client.
            // The error code is for ZMQ EFSM error.
            if(error.num() == common::kZmqEFSMError && !this->flag_client_working_)
                return ClientResult::CLIENT_STOPPED;

            // Call to the error callback and stop the client for safety.
            this->onClientError(error, "CommandClientBase: Error while receiving a reply. Stopping the client.");
            this->internalStopClient();
            return ClientResult::INTERNAL_ZMQ_ERROR;
        }
    }
}

void CommandClientBase::internalStopClient()
{
    // If server is already stopped, do nothing.
    if (!this->flag_client_working_)
        return;

    // Set the shared working flag to false (is atomic).
    this->flag_client_working_ = false;

    // If the client is waiting a response.
    if(this->fut_recv_.valid() && this->fut_recv_.wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
    {
        // Send the close msg and wait the future.
        this->req_close_socket_->send(zmq::message_t(), zmq::send_flags::none);
        this->fut_recv_.wait();
    }

    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Delete the sockets.
    this->deleteSockets();
}

void CommandClientBase::aliveWorker()
{
    // Mutex for the condiction variable.
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);

    // Request and reply.
    RequestData request;
    CommandReply reply;

    // Update the request.
    request.command = common::ServerCommand::REQ_ALIVE;

//    // Create the ZMQ auxiliar alive socket.
//    try
//    {
//        zmq::socket_t *alive_socket = new zmq::socket_t(*this->getContext().get(), zmq::socket_type::req);
//        alive_socket->connect(this->server_endpoint_);
//        alive_socket->set(zmq::sockopt::rcvtimeo, 100);
//        alive_socket->set(zmq::sockopt::linger, 0);

//    }
//    catch (const zmq::error_t &error)
//    {

//        // Call to the internal callback.
//        this->onClientError(error, "TODO");

//        return;
//    }

//    // Worker alive loop.
//    while(this->flag_autoalive_enabled_)
//    {
//        std::chrono::steady_clock::time_point bef;
//        std::chrono::steady_clock::time_point aft;

//        bef = std::chrono::steady_clock::now();

//        do
//        {

//        }while((bef - aft).count())




//        auto res =
//            this->auto_alive_cv_.wait_for(lk, std::chrono::milliseconds(common::kClientAlivePeriodMsec));

//        if (std::cv_status::timeout == res)
//        {
//            msg = this->prepareMessage(
//                            RequestData(static_cast<common::CommandType>(ServerCommand::REQ_ALIVE)));
//            try
//            {
//                msg.send(*alive_socket);
//            }  catch (const zmq::error_t &error)
//            {
//                // TODO: handle error
//                std::cerr << "Failed to automatically send alive command with error: " << error.num() << std::endl;
//                send_success = false;
//            }

//            if (send_success)
//            {
//                auto recv_result = this->recvFromSocket(alive_socket, data_out, out_size);
//                auto *data_bytes = static_cast<std::uint8_t*>(data_out);

//                if (0 == recv_result && out_size == sizeof(common::ServerResult))
//                {
//                    common::ServerResult result;


//                    zmqutils::utils::binarySerializeDeserialize(
//                                data_bytes, sizeof(common::CommandReply), &result);

//                    recv_success = result == common::ServerResult::COMMAND_OK;

//                }
//                else
//                {
//                    std::cerr << "Auto alive message answer receive failed" << std::endl;
//                    recv_success = false;
//                }

//                delete[] data_bytes;
//            }

//            if (!send_success || !recv_success)
//            {
//                std::cerr << "Failed auto sending alive message. Process will be stopped." << std::endl;
//                this->auto_alive_working_ = false;
//            }

//        }

//    }

//    delete alive_socket;
//    */
}

ClientResult CommandClientBase::doConnect()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Containers.
    RequestData request;
    CommandReply reply;

    // Serializer.
    utils::BinarySerializer serializer;

    // Serialize the parameters data.
    serializer.write(this->client_info_.ip, this->client_info_.pid, this->client_info_.hostname, this->client_name_);

    // Update the request.
    request.command = common::ServerCommand::REQ_CONNECT;
    request.params = serializer.moveUnique(request.params_size);


    // Send the command.
    return this->sendCommand(request, reply);
}

ClientResult CommandClientBase::doDisconnect()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Containers.
    RequestData request;
    CommandReply reply;

    // Update the request.
    request.command = common::ServerCommand::REQ_DISCONNECT;

    // Send the command.
    return this->sendCommand(request, reply);
}

ClientResult CommandClientBase::doAlive()
{
    // Safe mutex lock
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Containers.
    RequestData request;
    CommandReply reply;

    // Update the request.
    request.command = common::ServerCommand::REQ_ALIVE;

    // Send the command.
    return this->sendCommand(request, reply);
}

zmq::multipart_t CommandClientBase::prepareMessage(const RequestData &request)
{
    // Serializer.
    utils::BinarySerializer serializer;

    // Prepare the uuid message.
    const std::array<std::byte, 16>& uuid_bytes = this->client_info_.uuid.getBytes();
    zmq::message_t msg_uuid(uuid_bytes.size());
    std::copy(uuid_bytes.begin(), uuid_bytes.end(), static_cast<std::byte*>(msg_uuid.data()));

    // Preprare the command message.
    size_t cmd_size = serializer.write(request.command);
    zmq::message_t msg_command(serializer.release(), cmd_size);

    // Prepare the multipart msg.
    zmq::multipart_t multipart_msg;
    multipart_msg.add(std::move(msg_uuid));
    multipart_msg.add(std::move(msg_command));

    // Add command parameters if they exist
    if (request.params_size > 0)
    {
        // Prepare the command parameters
        zmq::message_t message_params(request.params.get(), request.params_size);
        multipart_msg.add(std::move(message_params));
    }

    // Return the multipart msg.
    return multipart_msg;
}

} // END NAMESPACES.
// =====================================================================================================================
