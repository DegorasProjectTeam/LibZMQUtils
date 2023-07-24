
// C++ INCLUDES
// =====================================================================================================================
#include <unordered_map>
#include <string>
#include <any>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/CommandServer>
#include <LibZMQUtils/Utils>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "amelas_example_common.h"
// =====================================================================================================================

using namespace zmqutils;


class DRGGController
{
public:

    DRGGController() = default;


};


// Example of creating a command server from the base.
class AmelasExampleServer : public CommandServerBase
{
public:

    AmelasExampleServer(unsigned port, const std::string& local_addr = "*");

private:

    // Helper to check if the custom command is valid.
    static bool validateAmelasCommand(AmelasServerCommand command);

    // Process the specific commands.
    void processAmelasCommand(const CommandRequest&, CommandReply&);

    void execSetHomePosition(const CommandRequest&, CommandReply&);

    // Internal overrided custom command received callback.
    // The most important part.
    virtual void onCustomCommandReceived(const CommandRequest&, CommandReply&) final;

    // Internal overrided start callback.
    virtual void onServerStart() final;

    // Internal overrided close callback.
    virtual void onServerStop() final;

    // Internal waiting command callback.
    virtual void onWaitingCommand() final;

    // Internal dead client callback.
    virtual void onDeadClient(const HostClient&) final;

    // Internal overrided connect callback.
    virtual void onConnected(const HostClient&) final;

    // Internal overrided disconnect callback.
    virtual void onDisconnected(const HostClient&) final;

    // Internal overrided command received callback.
    virtual void onCommandReceived(const CommandRequest&) final;

    // Internal overrided bad command received callback.
    virtual void onInvalidMsgReceived(const CommandRequest&) final;

    // Internal overrided sending response callback.
    virtual void onSendingResponse(const CommandReply&) final;

    // Internal overrided server error callback.
    virtual void onServerError(const zmq::error_t&, const std::string& ext_info) final;

    // Containers.
};
