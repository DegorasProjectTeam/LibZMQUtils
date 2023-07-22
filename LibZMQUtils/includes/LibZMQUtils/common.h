
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

// CONVENIENT ALIAS AND ENUMERATIONS
// =====================================================================================================================

using CmdRequestId = std::uint32_t;
using CmdReplyRes = std::uint32_t;

/**
 * @enum BaseServerCommand
 * @brief Enumerates the possible commands of a base command server. They can be extended in a subclass.
 * @warning These command ids must not be used for custom commands, they are special and reserved.
 */
enum class BaseServerCommand : std::uint32_t
{
    INVALID_COMMAND  = 0,  ///< Invalid command.
    REQ_CONNECT      = 1,  ///< Request to connect to the server.
    REQ_DISCONNECT   = 2,  ///< Request to disconnect from the server.
    REQ_ALIVE        = 3   ///< Request to check if the server is alive and for notify that the client is alive too.
};

/**
 * @enum BaseServerCommandResult
 * @brief Enumerates the possible results of a base command operation. They can be extended in a subclass.
 * @warning These results must not be used for custom results, they are special and reserved.
 */
enum class BaseServerResult : std::uint32_t
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
    INVALID_COMMAND        = 10, ///< The command is invalid.
    NOT_CONNECTED          = 11, ///< Not connected to the target.
    ALREADY_CONNECTED      = 12, ///< Already connected to the target.
    BAD_PARAMETERS         = 13, ///< The provided parameters are invalid.
    COMMAND_FAILED         = 14, ///< The command execution failed.
    NOT_IMPLEMENTED        = 15  ///< The command is not implemented.
};

static constexpr std::array<const char*, 4>  BaseServerCommandStr
{
    "INVALID_COMMAND",
    "REQ_CONNECT",
    "REQ_DISCONNECT",
    "REQ_ALIVE"
};

static constexpr std::array<const char*, 16>  BaseServerResultStr
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
    "NOT_IMPLEMENTED - Command is not implemented."
};

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

}} // END NAMESPACES.
// =====================================================================================================================
