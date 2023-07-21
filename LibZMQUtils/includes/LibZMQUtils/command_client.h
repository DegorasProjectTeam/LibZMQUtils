
// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <future>
#include <map>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/libzmqutils_global.h"
#include "LibZMQUtils/common.h"
// =====================================================================================================================

using zmqutils::common::CommandReqId;

namespace zmq
{
    class context_t;
    class socket_t;
}

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
// =====================================================================================================================

struct LIBZMQUTILS_EXPORT CommandData
{
    CommandData(CommandReqId id) :
        command_id(id),
        params(nullptr),
        params_size(0){}

    CommandReqId command_id;
    void* params;
    size_t params_size;
};

class LIBZMQUTILS_EXPORT CommandClientBase
{

public:

    static const CommandReqId kNoCommand;
    static const CommandReqId kConnectCommand;
    static const CommandReqId kDisconnectCommand;
    static const CommandReqId kAliveCommand;

    // TODO: maybe this should be configurable
    static const int kClientAliveTimeoutMsec;
    static const int kClientSendAlivePeriodMsec;

    enum class CommandError : std::uint32_t
    {
        NOT_ERROR,
        NO_COMMAND,
        NOT_CONNECTED,
        ALREADY_DISCONNECTED,
        ALREADY_CONNECTED,
        BAD_PARAMETERS,
        COMMAND_FAILED,
        NOT_IMPLEMENTED
    };


    CommandClientBase(const std::string &server_endpoint);

    bool startClient(const std::string& interface_name);
    void stopClient();
    void resetClient();

    void startAutoAlive();
    void stopAutoAlive();

    void setClientHostIP(const std::string& interf);

    void setClientId(const std::string &id){};

    int sendCommand(const CommandData& msg, void* &data_out, size_t &out_bytes);

    // Remove. Only for testing.
    int sendBadCommand1(void* &data_out, size_t &out_bytes);

    static void binarySerializeDeserialize(const void *data, size_t data_size_bytes, void *dest);

private:

    int recvFromSocket(zmq::socket_t *socket, void *&data, size_t &data_size_bytes) const;
    void sendAliveCallback();

    // Internal client identification.
    std::string client_host_ip_;
    std::string client_host_name_;
    std::string client_host_id_;

    // Server endpoint.
    std::string server_endpoint_;


    // ZMQ context and socket.
    zmq::context_t *context_;
    zmq::socket_t *socket_;


    std::future<void> auto_alive_future_;
    std::condition_variable auto_alive_cv_;
    std::atomic_bool auto_alive_working_;

};

} // END NAMESPACES.
// =====================================================================================================================
