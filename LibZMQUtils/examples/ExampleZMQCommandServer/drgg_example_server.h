
// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/CommandServer>
#include <LibZMQUtils/Utils>
// =====================================================================================================================

using namespace zmqutils;

// Specific subclass commands (0 to 3 are reserved for the base server).
enum class DRGGServerCommand : std::uint32_t
{
    REQ_SET_RANGE_GATES = 4,
    REQ_GET_RANGE_GATES = 5
};

// Specific subclass errors (0 to 15 are reserved for the base server).
enum class DRGGServerResult : std::uint32_t
{
    INVALID_COMMAND       = 16,
    INVALID_HOME_POSITION = 17
};

static constexpr auto DRGGServerCommandStr = utils::joinArraysConstexpr(
    common::BaseServerCommandStr,
    std::array<const char*, 2>
    {
        "REQ_SET_RANGE_GATES",
        "REQ_GET_RANGE_GATES"
    });

// Example of creating a command server from the base.
class DRGGCustomServer : public CommandServerBase
{
public:

    DRGGCustomServer(unsigned port, const std::string& local_addr = "*");

private:

    // Internal overrided custom command received callback.
    virtual void onCustomCommandReceived(const CommandRequest&, CommandReply&) final;

    // Internal overrided start callback.
    virtual void onServerStart() final;

    // Internal overrided close callback.
    virtual void onServerStop() final;

    // Internal waiting command callback.
    virtual void onWaitingCommand() final;

    // Internal dead client callback.
    virtual void onDeadClient() override;

    // Internal overrided connect callback.
    virtual void onNewConnection(const CommandRequest&) final;

    // Internal overrided disconnect callback.
    virtual void onDisconnected(const CommandRequest&) final;

    // Internal overrided server error callback.
    virtual void onServerError(const zmq::error_t&, const std::string& ext_info) final;

    // Internal overrided command received callback.
    virtual void onCommandReceived(const CommandRequest&) final;

    // Internal overrided command received callback.
    virtual void onBadMessageReceived(const CommandRequest&) final;

    // Internal overrided command received callback.
    virtual void onSendingResponse(const CommandReply&) final;
};
