// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <unordered_map>
#include <string>
#include <any>
#include <variant>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/CommandServer>
#include <LibZMQUtils/Utils>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasExampleController/common.h"
#include "AmelasExampleController/utils.h"
#include "common.h"
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
// =====================================================================================================================

using namespace zmqutils;

// Example of creating a command server from the base.
class AmelasServer : public CommandServerBase
{
public:

    AmelasServer(unsigned port, const std::string& local_addr = "*");

    const std::map<common::AmelasServerCommand, common::ControllerCallback>& getCallbackMap() const;

    void setCallback(common::AmelasServerCommand command, common::ControllerCallback callback);

    template<typename ClassT = void, typename RetT = void, typename... Args>
    void setCallback(common::AmelasServerCommand command,
                     ClassT* object,
                     RetT(ClassT::*callback)(Args...))
    {
        callback_map_[command] = zmqutils::utils::makeCallback(object, callback);
    }

    // Removes a callback for a command
    void removeCallback(common::AmelasServerCommand);

    // Clears all the callbacks
    void clearCallbacks();

    // Checks if a callback is set for a command
    bool isCallbackSet(common::AmelasServerCommand) const;

private:

    template <typename CallbackType, typename... Args>
    common::ControllerError invokeCallback(common::AmelasServerCommand command, Args&&... args)
    {
        if (auto callback = std::get_if<CallbackType>(&callback_map_[command]))
        {
            return (*callback)(std::forward<Args>(args)...);
        }
        throw std::runtime_error("Invalid command or incorrect callback type");
    }

    // Helper to check if the custom command is valid.
    static bool validateAmelasCommand(common::AmelasServerCommand command);

    // Process the specific commands.
    void processAmelasCommand(const CommandRequest&, CommandReply&);
    void processSetHomePosition(const CommandRequest&, CommandReply&);
    void processGetHomePosition(const CommandRequest&, CommandReply&);

    // Internal overrided custom command received callback.
    // WARNING The most important part.
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

    // External callbacks map.
    std::map<common::AmelasServerCommand, common::ControllerCallback> callback_map_;
};

} // END NAMESPACES.
// =====================================================================================================================
