
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
#include "amelas_example_common.h"
// =====================================================================================================================

using namespace zmqutils;

namespace amelascontrol{
namespace utils{

template<typename ClassType, typename ReturnType, typename... Args>
static std::function<ReturnType(Args...)> makeCallback(ClassType* object,
                                                       ReturnType(ClassType::*memberFunction)(Args...))
{
    return [object, memberFunction](Args... args) -> ReturnType
    {
        return (object->*memberFunction)(std::forward<Args>(args)...);
    };
}

}}

class AmelasExampleController
{
public:

    enum class AmelasError : common::ResultType
    {
        SUCCESS = 0,
        INVALID_POSITION = 1,
        UNSAFE_POSITION = 2
    };


    AmelasExampleController();

    AmelasError setHomePosition(double az, double el)
    {
        // Auxiliar result.
        AmelasError error = AmelasError::SUCCESS;

        // Check the provided values.
        if (az >= 360.0 ||  az < 0.0 || el >= 90. || el < 0.)
        {
            error = AmelasError::INVALID_POSITION;
        }
        else
        {
            this->home_pos_az_ = az;
            this->home_pos_el_ = el;
        }

        std::cout << std::string(80, '-') << std::endl;
        std::cout<<"<AMELAS CONTROLLER>"<<std::endl;
        std::cout<<"-> SET_HOME_POSITION"<<std::endl;
        std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout<<"Az: "<<az<<std::endl;
        std::cout<<"El: "<<el<<std::endl;
        std::cout << std::string(80, '-') << std::endl;

        return error;
    }

    AmelasError getHomePosition(double &az, double &el)
    {
        az = this->home_pos_az_;
        el = this->home_pos_el_;

        std::cout << std::string(80, '-') << std::endl;
        std::cout<<"<AMELAS CONTROLLER>"<<std::endl;
        std::cout<<"-> GET_HOME_POSITION"<<std::endl;
        std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout<<"Az: "<<az<<std::endl;
        std::cout<<"El: "<<el<<std::endl;
        std::cout << std::string(80, '-') << std::endl;

        return AmelasError::SUCCESS;
    }

    AmelasError getDatetime(std::string&)
    {
        return AmelasError::SUCCESS;
    }



    // Callback function type aliases
    using SetHomePositionCallback = std::function<AmelasError(double, double)>;
    using GetHomePositionCallback = std::function<AmelasError(double&, double&)>;
    using GetDatetimeCallback = std::function<AmelasError(std::string&)>;

    // Callback variant.
    using AmelasCallback = std::variant<SetHomePositionCallback,
                                        GetHomePositionCallback,
                                        GetDatetimeCallback>;

private:

    double home_pos_az_;
    double home_pos_el_;

};


// Example of creating a command server from the base.
class AmelasExampleServer : public CommandServerBase
{
public:

    AmelasExampleServer(unsigned port, const std::string& local_addr = "*");

    void setCallback(AmelasServerCommand command, AmelasExampleController::AmelasCallback callback)
    {
        callback_map_[command] = callback;
    }

    template<typename ClassT = void, typename ReturnT = void, typename... Args>
    void setCallback(AmelasServerCommand command,
                     ClassT* object,
                     ReturnT(ClassT::*callback)(Args...))
    {
        callback_map_[command] = amelascontrol::utils::makeCallback(object, callback);
    }

private:

    template <typename CallbackType, typename... Args>
    AmelasExampleController::AmelasError invoke(AmelasServerCommand command, Args&&... args)
    {
        if (auto callback = std::get_if<CallbackType>(&callback_map_[command]))
        {
            return (*callback)(std::forward<Args>(args)...);
        }
        throw std::runtime_error("Invalid command or incorrect callback type");
    }

    // Helper to check if the custom command is valid.
    static bool validateAmelasCommand(AmelasServerCommand command);

    // Process the specific commands.
    void processAmelasCommand(const CommandRequest&, CommandReply&);

    void processSetHomePosition(const CommandRequest&, CommandReply&);
    void processGetHomePosition(const CommandRequest&, CommandReply&);

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

    // External callbacks map.
    std::map<AmelasServerCommand, AmelasExampleController::AmelasCallback> callback_map_;
};
