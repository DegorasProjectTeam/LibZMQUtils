
// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
#include <map>
#include <vector>
#include <variant>
#include <functional>
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace common{
// =====================================================================================================================

// CONSTANTS
// =====================================================================================================================

// =====================================================================================================================

// CONVENIENT ALIAS, ENUMERATIONS AND CONSTEXPR
// =====================================================================================================================

enum class ControllerError : std::uint32_t
{
    SUCCESS = 0,
    INVALID_POSITION = 1,
    UNSAFE_POSITION = 2
};

struct AltAzPos
{
    AltAzPos(double az, double el):
        az(az), el(el){}

    AltAzPos(): az(-1), el(-1){}

    double az;
    double el;
};

// Callback function type aliases
using SetHomePositionCallback = std::function<ControllerError(const AltAzPos&)>;
using GetHomePositionCallback = std::function<ControllerError(AltAzPos&)>;
using GetDatetimeCallback = std::function<ControllerError(std::string&)>;

// Callback variant.
using ControllerCallback = std::variant<SetHomePositionCallback,
                                        GetHomePositionCallback,
                                        GetDatetimeCallback>;





// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
