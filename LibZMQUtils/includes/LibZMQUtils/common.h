
// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <cstring>
#include <memory>
#include <zmq/zmq.hpp>
#include <zmq/zmq_addon.hpp>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/libzmqutils_global.h"
#include "LibZMQUtils/utils.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace common{
// =====================================================================================================================

// CONSTANTS
// =====================================================================================================================
constexpr int kClientAliveTimeoutMsec = 5000;   ///< Default timeout for consider a client dead.
constexpr unsigned kReconnectTimes = 10;        ///< Reconnect default number of attempts.
// =====================================================================================================================

// CONVENIENT ALIAS, ENUMERATIONS AND CONSTEXPR
// =====================================================================================================================

// TODO REMOVE
using CmdRequestId = std::uint32_t;
using CmdReplyRes = std::uint32_t;

using CommandType = std::uint32_t;
using ResultType = std::uint32_t;


/**
 * @enum BaseServerCommand
 * @brief Enumerates the possible commands of a base command server. They can be extended in a subclass.
 * @warning Commands 0 to 10 ids must not be used for custom commands, they are special and reserved.
 * @warning Only positive commands will be acepted by the server.
 * @warning Messages with the command 0, sentinel value or a reserved commands are considered invalid.
 */
enum class BaseServerCommand : CommandType
{
    INVALID_COMMAND   = 0,  ///< Invalid command.
    REQ_CONNECT       = 1,  ///< Request to connect to the server.
    REQ_DISCONNECT    = 2,  ///< Request to disconnect from the server.
    REQ_ALIVE         = 3,  ///< Request to check if the server is alive and for notify that the client is alive too.
    RESERVED_COMMANDS = 4,  ///< Sentinel value indicating the start of the reserved commands (not is as a valid msg).
    END_BASE_COMMANDS = 10  ///< Sentinel value indicating the end of the base commands (not is as a valid msg).
};

/**
 * @enum BaseServerCommandResult
 * @brief Enumerates the possible results of a base command operation. They can be extended in a subclass.
 * @warning These results must not be used for custom results, they are special and reserved.
 */
enum class BaseServerResult : CommandType
{
    COMMAND_OK             = 0, ///< The command was executed successfully.
    INTERNAL_ZMQ_ERROR     = 1, ///< An internal ZeroMQ error occurred.
    EMPTY_MSG              = 2, ///< The message is empty.
    EMPTY_CLIENT_IP        = 3, ///< The client IP is missing or empty.
    EMPTY_CLIENT_NAME      = 4, ///< The client name is missing or empty.
    EMPTY_CLIENT_PID       = 5, ///< The client pid is missing or empty.
    EMPTY_PARAMS           = 6, ///< The command parameters are missing or empty.
    TIMEOUT_REACHED        = 7, ///< The operation timed out.
    INVALID_PARTS          = 8, ///< The command has invalid parts.
    UNKNOWN_COMMAND        = 9, ///< The command is not recognized.
    INVALID_MSG            = 10, ///< The command is invalid.
    NOT_CONNECTED          = 11, ///< Not connected to the target.
    ALREADY_CONNECTED      = 12, ///< Already connected to the target.
    BAD_PARAMETERS         = 13, ///< The provided parameters are invalid.
    COMMAND_FAILED         = 14, ///< The command execution failed.
    NOT_IMPLEMENTED        = 15, ///< The command is not implemented.
    END_BASE_ERRORS        = 20  ///< Sentinel value indicating the end of the base errors (not is a valid error).
};

// Usefull const expressions.
constexpr int kMinBaseCmdId = static_cast<int>(BaseServerCommand::INVALID_COMMAND) + 1;
constexpr int kMaxBaseCmdId = static_cast<int>(BaseServerCommand::RESERVED_COMMANDS) - 1;

static constexpr std::array<const char*, 11>  BaseServerCommandStr
{
    "INVALID_COMMAND",
    "REQ_CONNECT",
    "REQ_DISCONNECT",
    "REQ_ALIVE",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "RESERVED_BASE_COMMAND",
    "END_BASE_COMMANDS"
};

static constexpr std::array<const char*, 21>  BaseServerResultStr
{
    "COMMAND_OK - Command executed.",
    "INTERNAL_ZMQ_ERROR - Internal ZeroMQ error.",
    "EMPTY_MSG - Message is empty.",
    "EMPTY_CLIENT_IP - Client IP missing or empty.",
    "EMPTY_CLIENT_NAME - Client name missing or empty.",
    "EMPTY_CLIENT_PID - Client pid missing or empty.",
    "EMPTY_PARAMS - Command parameters missing or empty.",
    "TIMEOUT_REACHED - Operation timed out.",
    "INVALID_PARTS - Command has invalid parts.",
    "UNKNOWN_COMMAND - Command is not recognized.",
    "INVALID_COMMAND - Command is invalid.",
    "NOT_CONNECTED - Not connected to the server.",
    "ALREADY_CONNECTED - Already connected to the server.",
    "BAD_PARAMETERS - Provided parameters are invalid.",
    "COMMAND_FAILED - Command execution failed.",
    "NOT_IMPLEMENTED - Command is not implemented.",
    "RESERVED_BASE_ERROR",
    "RESERVED_BASE_ERROR",
    "RESERVED_BASE_ERROR",
    "RESERVED_BASE_ERROR",
    "RESERVED_BASE_ERROR"
};

// =====================================================================================================================

// COMMON STRUCTS
// =====================================================================================================================

struct LIBZMQUTILS_EXPORT HostClientInfo
{
    HostClientInfo() = default;

    HostClientInfo(const HostClientInfo&) = default;

    HostClientInfo(HostClientInfo&&) = default;

    HostClientInfo& operator=(const HostClientInfo&) = default;

    HostClientInfo& operator=(HostClientInfo&&) = default;

    HostClientInfo(const std::string& ip, const std::string& name,
                   const std::string& pid, const std::string& info = "");

    // Struct members.
    std::string id;                          ///< Dinamic host client identification -> [ip//name//pid]
    std::string ip;                          ///< Host client ip.
    std::string hostname;                        ///< Host client name.
    std::string pid;                         ///< PID of the host client process.
    std::string info;                        ///< Host client information. // TODO
    utils::HRTimePointStd last_connection;   ///< Host client last connection time.
};

struct CommandRequest
{
    CommandRequest():
        command(BaseServerCommand::INVALID_COMMAND),
        params(nullptr),
        params_size(0)
    {}

    HostClientInfo client;
    BaseServerCommand command;
    std::unique_ptr<std::uint8_t> params;
    zmq::multipart_t raw_msg;
    size_t params_size;
};

struct CommandReply
{
    CommandReply():
        params(nullptr),
        params_size(0),
        result(BaseServerResult::COMMAND_OK)
    {}

    std::unique_ptr<std::uint8_t> params;
    size_t params_size;
    BaseServerResult result;
};

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
