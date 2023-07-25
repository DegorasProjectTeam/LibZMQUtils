
// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
#include <map>
#include <vector>
#include <functional>
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace utils{
// =====================================================================================================================

template<typename ClassType, typename ReturnType, typename... Args>
static std::function<ReturnType(Args...)> makeCallback(ClassType* object,
                                                       ReturnType(ClassType::*memberFunction)(Args...))
{
    return [object, memberFunction](Args... args) -> ReturnType
    {
        return (object->*memberFunction)(std::forward<Args>(args)...);
    };
}

}} // END NAMESPACES.
// =====================================================================================================================