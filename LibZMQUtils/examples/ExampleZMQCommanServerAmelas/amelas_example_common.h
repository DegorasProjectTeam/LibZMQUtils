
#include <functional>
#include <any>

#include <LibZMQUtils/Utils>

// =====================================================================================================================
#pragma once
// =====================================================================================================================

using namespace zmqutils;

// Convenient aliases.
//using AmelasControllerCallback = std::function<bool(std::any)>;

// Specific subclass commands (0 to 4 are reserved for the base server).
// WARNING: In our approach, the server commands must be always in order.
enum class AmelasServerCommand : common::CommandType
{
    REQ_SET_DATETIME      = 11,
    REQ_GET_DATETIME      = 12,
    REQ_SET_HOME_POSITION = 13,
    REQ_GET_HOME_POSITION = 14,
    END_AMELAS_COMMANDS
};

// Specific subclass errors (0 to 15 are reserved for the base server).
enum class AmelasServerResult : common::CommandType
{
    INVALID_DATETIME = 16,
    INVALID_POSITION = 17
};

// Extend the base command strings with those of the subclass.
static constexpr auto AmelasServerCommandStr = utils::joinArraysConstexpr(
    common::BaseServerCommandStr,
    std::array<const char*, 5>
    {
        "REQ_SET_DATETIME",
        "REQ_GET_DATETIME",
        "REQ_SET_HOME_POSITION",
        "REQ_GET_HOME_POSITION",
        "END_DRGG_COMMANDS"
    });

// Extend the base result strings with those of the subclass.
static constexpr auto AmelasServerResultStr = utils::joinArraysConstexpr(
    common::BaseServerResultStr,
    std::array<const char*, 2>
    {
        "INVALID_DATETIME - Datetime provided is invalid.",
        "INVALID_POSITION - Position (az, el) provided is invalid."
    });

// Usefull const expressions.
constexpr int kMinCmdId = static_cast<int>(common::BaseServerCommand::END_BASE_COMMANDS) + 1;
constexpr int kMaxCmdId = static_cast<int>(AmelasServerCommand::END_AMELAS_COMMANDS) - 1;
