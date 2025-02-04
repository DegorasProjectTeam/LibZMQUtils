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
 * @brief This file contains the implementation of several utilities for the project development.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <cmath>
#include <iomanip>
#include <sstream>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Utilities/utils.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{
// =====================================================================================================================

// =====================================================================================================================
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::time_point_cast;
// =====================================================================================================================

std::string timePointToString(const TimePointStd &tp, const std::string &format, bool add_ms, bool add_ns, bool utc)
{
    // Stream to hold the formatted string and the return container.
    std::ostringstream ss;
    // Convert the time point to a duration and get the different time fractions.
    TimePointStd::duration dur = tp.time_since_epoch();
    const time_t secs = duration_cast<std::chrono::seconds>(dur).count();
    const long long mill = duration_cast<std::chrono::milliseconds>(dur).count();
    const long long ns = duration_cast<std::chrono::nanoseconds>(dur).count();
    const long long s_ns = secs * static_cast<long long>(1e9);
    const long long t_ns = (ns - s_ns);
    // Format the duration.
    if (const std::tm *tm = (utc ? std::gmtime(&secs) : std::localtime(&secs)))
    {
        ss << std::put_time(tm, format.c_str());
        if(add_ms && !add_ns)
            ss << '.' << std::setw(3) << std::setfill('0') << (mill - secs * static_cast<long long>(1e3));
        else if(add_ns)
            ss << '.' << std::setw(9) << std::setfill('0') << t_ns;
    }
    else
    {
        // If error, return an empty string.
        return {};
    }
    // Return the container.
    return ss.str();
}

std::string timePointToIso8601(const TimePointStd &tp, bool add_ms, bool add_ns, bool utc)
{
    // Format the ISO 8601 datetime without UTC offset.
    std::string iso8601_datetime = timePointToString(tp, "%Y-%m-%dT%H:%M:%S", add_ms, add_ns, true);

    if (!utc)
    {
        // Get the offsets.
        std::ostringstream ss;
        TimePointStd::duration dur = tp.time_since_epoch();
        const time_t secs = duration_cast<std::chrono::seconds>(dur).count();
        const std::tm *tm_local = std::localtime(&secs);
        if(!tm_local)
            return std::string();
        const int hh_local = tm_local->tm_hour;
        const int mm_local = tm_local->tm_min;
        const std::tm *tm_utc = std::gmtime(&secs);
        if(!tm_utc)
            return std::string();
        int offset_hours = hh_local - tm_utc->tm_hour;
        int offset_minutes = mm_local - tm_utc->tm_min;

        // Format the UTC offset as [+/-]HH:MM.
        std::ostringstream offset_ss;
        offset_ss << std::setfill('0');
        offset_ss << (offset_hours >= 0 ? '+' : '-');
        offset_ss << std::setw(2) << std::abs(offset_hours);
        offset_ss << ':';
        offset_ss << std::setw(2) << std::abs(offset_minutes);

        // Append the UTC offset to the ISO 8601 datetime.
        iso8601_datetime += offset_ss.str();
    }
    else
    {
        // Add 'Z' to indicate UTC.
        iso8601_datetime += "Z";
    }

    return iso8601_datetime;
}

std::string currentISO8601Date(bool add_ms, bool add_ns, bool utc)
{
    auto now = TimePointStd::clock::now();
    return timePointToIso8601(now, add_ms, add_ns, utc);
}

HRTimePointStd iso8601DatetimeToTimePoint(const std::string &datetime)
{
    // TODO: use LibDegorasBase
    int y, m, d, h, M, s;
    std::smatch match;
    std::regex iso8601_regex_extended(R"(^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})(?:\.(\d+))?(Z)?$)");
    std::regex iso8601_regex_basic(R"(^(\d{4})(\d{2})(\d{2})T(\d{2})(\d{2})(\d{2})(?:\.(\d+))?(Z)?$)");

    if (!std::regex_search(datetime, match, iso8601_regex_extended) &&
        !std::regex_search(datetime, match, iso8601_regex_basic))
    {
        throw std::invalid_argument("[LibZMQUtils,Timing,iso8601DatetimeToTimePoint] Invalid argument: " + datetime);
    }

    y = std::stoi(match[1].str());
    m = std::stoi(match[2].str());
    d = std::stoi(match[3].str());
    h = std::stoi(match[4].str());
    M = std::stoi(match[5].str());
    s = std::stoi(match[6].str());
    std::string fractional_seconds_str = match[7].str();
    bool is_utc = match[8].str() == "Z";

    auto days_since_epoch = daysFromCivil(y, static_cast<unsigned>(m), static_cast<unsigned>(d));
    HRTimePointStd t = HRClock::time_point(std::chrono::duration<int, std::ratio<86400>>(days_since_epoch));

    t += std::chrono::hours(h);
    t += std::chrono::minutes(M);
    t += std::chrono::seconds(s);

    if (!fractional_seconds_str.empty()) {
        long long fractional_seconds = std::stoll(fractional_seconds_str);
        size_t length = fractional_seconds_str.length();

        if (length == 1) // Normalize to milliseconds
            t += std::chrono::milliseconds(fractional_seconds * 100);
        else if (length == 2) // Normalize to milliseconds
            t += std::chrono::milliseconds(fractional_seconds * 10);
        else if (length == 3) // Already in milliseconds
            t += std::chrono::milliseconds(fractional_seconds);
        else if (length <= 6) // Normalize to microseconds
            t += std::chrono::microseconds(fractional_seconds * static_cast<long long>(std::pow(10, 6 - length)));
        else // Normalize to nanoseconds
            t += std::chrono::nanoseconds(fractional_seconds * static_cast<long long>(std::pow(10, 9 - length)));
    }

    if (!is_utc) {
        // Adjust for local timezone if 'Z' is not present
        std::time_t now = std::time(nullptr);
        std::tm* now_tm = std::localtime(&now);
        std::tm* gm_tm = std::gmtime(&now);
        auto local_diff = std::mktime(now_tm) - std::mktime(gm_tm);
        t += std::chrono::seconds(local_diff);
    }

    return t;
}

bool isValidIso8601Datetime(const std::string &datetime)
{
    std::smatch match;

    // Regex.
    const std::regex iso8601_regex_extended(
        R"(^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})(?:\.(\d+))?(?:(Z)|((\+|\-)(\d{2}):(\d{2})))?$)");
    const std::regex iso8601_regex_basic(
        R"(^(\d{4})(\d{2})(\d{2})T(\d{2})(\d{2})(\d{2})(?:\.(\d+))?(?:(Z)|((\+|\-)(\d{2}):(\d{2})))?$)");

    // Check if datetime matches one of ISO datetime patterns.
    return std::regex_search(datetime, match, iso8601_regex_extended) ||
           std::regex_search(datetime, match, iso8601_regex_basic);
}

long long daysFromCivil(int y, unsigned int m, unsigned int d)
{
    // Check the numeric limits.
    static_assert(std::numeric_limits<unsigned>::digits >= 18,
                  "[LibDegorasBase,Timing,daysFromCivil] >= 16 bit unsigned integer");
    static_assert(std::numeric_limits<int>::digits >= 20,
                  "[LibDegorasBase,Timing,daysFromCivil] >= 16 bit signed integer");
    // Calculate the number of days since 1970-01-01.
    y -= m <= 2;
    const int era = (y >= 0 ? y : y-399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);   // [0, 399]
    const unsigned doy = (153*(m > 2 ? m-3 : m+9) + 2)/5 + d-1;  // [0, 365]
    const unsigned doe = yoe * 365 + yoe/4 - yoe/100 + doy;      // [0, 146096]
    // Return the result.
    return era * 146097LL + static_cast<int>(doe) - 719468LL;
}



}} // END NAMESPACES.
// =====================================================================================================================
