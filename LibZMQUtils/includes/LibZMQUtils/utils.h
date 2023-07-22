
// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <algorithm>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <cstring>
#include <chrono>
#include <array>
#include <utility>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/libzmqutils_global.h"
// =====================================================================================================================

// DEFINITIONS
// =====================================================================================================================
#if defined(__MINGW32__) || defined(_MSC_VER)
#define MKGMTIME _mkgmtime
#else
#define MKGMTIME timegm
#endif
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{
// =====================================================================================================================

// CONVENIENT ALIAS AND ENUMERATIONS
// =====================================================================================================================
/// High resolution time point to store datetimes (uses Unix Time).
using HRTimePointStd = std::chrono::time_point<std::chrono::high_resolution_clock>;
// =====================================================================================================================

struct LIBZMQUTILS_EXPORT NetworkAdapterInfo
{
    std::string id;
    std::string name;
    std::string descr;
    std::string ip;
};

LIBZMQUTILS_EXPORT void binarySerializeDeserialize(const void *data, size_t data_size_bytes, void *dest);

LIBZMQUTILS_EXPORT std::vector<NetworkAdapterInfo> getHostIPsWithInterfaces();

LIBZMQUTILS_EXPORT std::string getHostname();

LIBZMQUTILS_EXPORT unsigned getCurrentPID();

LIBZMQUTILS_EXPORT std::string timePointToString(const HRTimePointStd& tp,
                                                 const std::string& format = "%Y-%m-%dT%H:%M:%S",
                                                 bool add_ms = true, bool add_ns = false, bool utc = true);

LIBZMQUTILS_EXPORT std::string timePointToIso8601(const HRTimePointStd& tp, bool add_ms = true, bool add_ns = false);

LIBZMQUTILS_EXPORT std::string currentISO8601Date(bool add_ms = true);

namespace internal
{
template <typename T, std::size_t... Is1, std::size_t... Is2>
constexpr std::array<T, sizeof...(Is1) + sizeof...(Is2)>
joinArrays(const std::array<T, sizeof...(Is1)>& a1, const std::array<T, sizeof...(Is2)>& a2, std::index_sequence<Is1...>, std::index_sequence<Is2...>)
{
    return { a1[Is1]..., a2[Is2]... };
}
}

template <typename T, std::size_t N1, std::size_t N2>
constexpr std::array<T, N1 + N2> joinArraysConstexpr(const std::array<T, N1>& a1, const std::array<T, N2>& a2)
{
    return internal::joinArrays(a1, a2, std::make_index_sequence<N1>(), std::make_index_sequence<N2>());
}

}} // END NAMESPACES.
// =====================================================================================================================
