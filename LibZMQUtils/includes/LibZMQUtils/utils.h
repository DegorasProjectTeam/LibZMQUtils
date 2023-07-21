
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

LIBZMQUTILS_EXPORT int getCurrentPID();

LIBZMQUTILS_EXPORT std::string timePointToString(const HRTimePointStd& tp,
                                                 const std::string& format = "%Y-%m-%dT%H:%M:%S",
                                                 bool add_ms = true, bool add_ns = false, bool utc = true);

LIBZMQUTILS_EXPORT std::string timePointToIso8601(const HRTimePointStd& tp, bool add_ms = true, bool add_ns = false);

LIBZMQUTILS_EXPORT std::string currentISO8601Date(bool add_ms = true);

}} // END NAMESPACES.
// =====================================================================================================================
