
// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/CommandServer>
#include <LibZMQUtils/Utils>
// =====================================================================================================================

// Example of creating a command server from the base.
class CustomCommandServer : public zmqutils::CommandServerBase
{
public:


    enum class AmelasCommand : std::uint32_t
    {
        // Base commands.
        REQ_CONNECT              = 0,
        REQ_DISCONNECT           = 1,
        REQ_ALIVE                = 2,
        // Specific commands.
        REQ_SET_HOME_POSITION,
        REQ_GET_HOME_POSITION
    };




    CustomCommandServer(const std::string &listen_address, int port) :
        CommandServerBase(listen_address, port){}

private:

    // Internal overrided connect callback.
    virtual void onNewConnection(const zmqutils::CommandExecReq& cmd_req) override
    {
        std::cout<<"--------------------------------------"<<std::endl;
        std::cout<<"ON CONNECTED: "<<std::endl;
        std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout<<"Ip: "<<cmd_req.client_ip<<std::endl;
        std::cout<<"Host: "<<cmd_req.client_name<<std::endl;
        std::cout<<"--------------------------------------"<<std::endl;
    }

    // Internal overrided disconnect callback.
    virtual void onDisconnected(const zmqutils::CommandExecReq& cmd_req) override
    {
        std::cout<<"--------------------------------------"<<std::endl;
        std::cout<<"ON DISCONNECTED: "<<std::endl;
        std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout<<"Ip: "<<cmd_req.client_ip<<std::endl;
        std::cout<<"Host: "<<cmd_req.client_name<<std::endl;
        std::cout<<"--------------------------------------"<<std::endl;
    }

    // Internal overrided command received callback.
    virtual void onCommandReceived(const zmqutils::CommandExecReq& cmd_req) override
    {
        std::cout<<"--------------------------------------"<<std::endl;
        std::cout<<"ON COMMAND RECEIVED: "<<std::endl;
        std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout<<"Ip: "<<cmd_req.client_ip<<std::endl;
        std::cout<<"Host: "<<cmd_req.client_name<<std::endl;
        std::cout<<"Command: "<<cmd_req.command_id<<std::endl;
        std::cout<<"--------------------------------------"<<std::endl;
    }

    // Internal overrided server error callback.
    virtual void onServerError(const zmq::error_t &error, const std::string& ext_info = "") override
    {
        std::cout<<"--------------------------------------"<<std::endl;
        std::cout<<"ON SERVER ERROR: "<<std::endl;
        std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout<<"Code: "<<error.num()<<std::endl;
        std::cout<<"Error: "<<error.what()<<std::endl;
        std::cout<<"Info: "<<ext_info<<std::endl;
        std::cout<<"--------------------------------------"<<std::endl;
    }
};
