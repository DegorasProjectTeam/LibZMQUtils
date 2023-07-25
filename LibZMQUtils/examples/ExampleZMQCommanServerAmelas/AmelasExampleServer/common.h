
#include <functional>
#include <any>

#include <LibZMQUtils/Utils>

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace common{
// =====================================================================================================================

// Specific subclass commands (0 to 4 are reserved for the base server).
// WARNING: In our approach, the server commands must be always in order.
enum class AmelasServerCommand : zmqutils::common::CommandType
{
    REQ_SET_DATETIME      = 11,
    REQ_GET_DATETIME      = 12,
    REQ_SET_HOME_POSITION = 13,
    REQ_GET_HOME_POSITION = 14,
    END_AMELAS_COMMANDS
};

// Specific subclass errors (0 to 20 are reserved for the base server).
enum class AmelasServerResult : zmqutils::common::ResultType
{
    EMPTY_CALLBACK = 21,
    INVALID_CALLBACK = 22
};

// Extend the base command strings with those of the subclass.
static constexpr auto AmelasServerCommandStr = zmqutils::utils::joinArraysConstexpr(
    zmqutils::common::ServerCommandStr,
    std::array<const char*, 5>
    {
        "REQ_SET_DATETIME",
        "REQ_GET_DATETIME",
        "REQ_SET_HOME_POSITION",
        "REQ_GET_HOME_POSITION",
        "END_DRGG_COMMANDS"
    });

// Extend the base result strings with those of the subclass.
static constexpr auto AmelasServerResultStr = zmqutils::utils::joinArraysConstexpr(
    zmqutils::common::ServerResultStr,
    std::array<const char*, 2>
    {
        "EMPTY_CALLBACK - The external callback for the command is empty.",
        "INVALID_CALLBACK - The external callback for the command is invalid."
    });

// Usefull const expressions.
constexpr int kMinCmdId = static_cast<int>(zmqutils::common::ServerCommand::END_BASE_COMMANDS) + 1;
constexpr int kMaxCmdId = static_cast<int>(AmelasServerCommand::END_AMELAS_COMMANDS) - 1;

}} // END NAMESPACES.
// =====================================================================================================================
