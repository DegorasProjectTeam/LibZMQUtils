/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
 *   A modern open-source and cross-platform C++ library with high-level utilities based on the well-known ZeroMQ      *
 *   open-source universal messaging library. Includes a suite of modules that encapsulates the ZMQ communication      *
 *   patterns as well as automatic binary serialization capabilities, specially designed for system infraestructure.   *
 *   The library is suited for the quick and easy integration of new and old systems and can be used in different      *
 *   sectors and disciplines seeking robust messaging and serialization solutions.                                     *
 *                                                                                                                     *
 *   Developed as free software within the context of the Degoras Project for the Satellite Laser Ranging Station      *
 *   (SFEL) at the Spanish Navy Observatory (ROA) in San Fernando, Cádiz. The library is open for use by other SLR     *
 *   stations and organizations, so we warmly encourage you to give it a try and feel free to contact us anytime!      *
 *                                                                                                                     *
 *   Copyright (C) 2024 Degoras Project Team                                                                           *
 *                      < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >                             *
 *                      < Jesús Relinque Madroñal >                                                                    *
 *                                                                                                                     *
 *   This file is part of LibZMQUtils.                                                                                 *
 *                                                                                                                     *
 *   Licensed under the European Union Public License (EUPL), Version 1.2 or subsequent versions of the EUPL license   *
 *   as soon they will be approved by the European Commission (IDABC).                                                 *
 *                                                                                                                     *
 *   This project is free software: you can redistribute it and/or modify it under the terms of the EUPL license as    *
 *   published by the IDABC, either Version 1.2 or, at your option, any later version.                                 *
 *                                                                                                                     *
 *   This project is distributed in the hope that it will be useful. Unless required by applicable law or agreed to in *
 *   writing, it is distributed on an "AS IS" basis, WITHOUT ANY WARRANTY OR CONDITIONS OF ANY KIND; without even the  *
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the EUPL license to check specific   *
 *   language governing permissions and limitations and more details.                                                  *
 *                                                                                                                     *
 *   You should use this project in compliance with the EUPL license. You should have received a copy of the license   *
 *   along with this project. If not, see the license at < https://eupl.eu/ >.                                         *
 **********************************************************************************************************************/

/** ********************************************************************************************************************
 * @file utils.h
 * @brief This file contains the declaration of several utilities for the project development.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <regex>
#include <string>
#include <cstring>
#include <chrono>
#include <array>
#include <utility>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
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
/// System clock time point to store datetimes (should use Unix Time).
using TimePointStd = std::chrono::system_clock::time_point;
///
using HRTimePointStd = std::chrono::high_resolution_clock::time_point;
/// Steady clock time point for measuring intervals.
using SCTimePointStd =  std::chrono::steady_clock::time_point;
/// High resolution clock.
using HRClock = std::chrono::high_resolution_clock;
/// Std milliseconds.
using MsStd = std::chrono::milliseconds;
/// Std microseconds.
using UsStd = std::chrono::microseconds;
// =====================================================================================================================

LIBZMQUTILS_EXPORT long long daysFromCivil(int y, unsigned m, unsigned d);

LIBZMQUTILS_EXPORT std::string timePointToString(const TimePointStd& tp,
                                                 const std::string& format = "%Y-%m-%dT%H:%M:%S",
                                                 bool add_ms = true, bool add_ns = false, bool utc = true);

LIBZMQUTILS_EXPORT std::string timePointToIso8601(const TimePointStd& tp,
                                                  bool add_ms = true, bool add_ns = false, bool utc = true);

LIBZMQUTILS_EXPORT std::string currentISO8601Date(bool add_ms = true, bool add_ns = false, bool utc = true);

LIBZMQUTILS_EXPORT HRTimePointStd iso8601DatetimeToTimePoint(const std::string& datetime);

LIBZMQUTILS_EXPORT bool isValidIso8601Datetime(const std::string& datetime);

// template<typename Enum, std::size_t N>
// std::string getEnumString(Enum value, const std::array<const char*, N>& str_array)
// {
//     static_assert(std::is_enum<Enum>::value, "getEnumString - Template argument must be an enum type.");

//     unsigned long long index = static_cast<unsigned long long>(value);

//     if (index < str_array.size())
//         return str_array[index];

//     return "Unknown value.";
// }

namespace internal
{
    template <typename T, std::size_t... Is1, std::size_t... Is2>
    constexpr std::array<T, sizeof...(Is1) + sizeof...(Is2)>
    joinArrays(const std::array<T, sizeof...(Is1)>& a1, const std::array<T, sizeof...(Is2)>& a2,
               std::index_sequence<Is1...>, std::index_sequence<Is2...>)
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
