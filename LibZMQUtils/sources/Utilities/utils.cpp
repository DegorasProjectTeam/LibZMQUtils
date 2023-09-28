/***********************************************************************************************************************
 *   LibZMQUtils (ZMQ Utilitites Library): A libre library with ZMQ related useful utilities.                          *
 *                                                                                                                     *
 *   Copyright (C) 2023 Degoras Project Team                                                                           *
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

}} // END NAMESPACES.
// =====================================================================================================================
