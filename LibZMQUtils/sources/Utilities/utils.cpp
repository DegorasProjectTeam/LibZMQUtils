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



#include <iomanip>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <sstream>
#include <iphlpapi.h>

#include "LibZMQUtils/Utilities/utils.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#elif _WIN32_WINNT < 0x0600
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{
// =====================================================================================================================

// =====================================================================================================================
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::time_point_cast;
// =====================================================================================================================

std::vector<NetworkAdapterInfo> getHostIPsWithInterfaces()
{
    // Result container.
    std::vector<NetworkAdapterInfo> adapters;

#ifdef _WIN32

    // Buffer size.
    ULONG  buff_size = 0;

    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
                             nullptr, nullptr, &buff_size) != ERROR_BUFFER_OVERFLOW)
    {
        return adapters;
    }

    std::vector<char> buffer(buff_size);

    PIP_ADAPTER_ADDRESSES adapter_addrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&buffer[0]);

    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
                             nullptr, adapter_addrs, &buff_size) != NO_ERROR)
    {
        return adapters;
    }

    while (adapter_addrs != nullptr)
    {
        if (adapter_addrs->OperStatus == IfOperStatusUp)
        {
            PIP_ADAPTER_UNICAST_ADDRESS unicast_addrs = adapter_addrs->FirstUnicastAddress;
            while (unicast_addrs != nullptr)
            {
                sockaddr_in* sockaddr = reinterpret_cast<sockaddr_in*>(unicast_addrs->Address.lpSockaddr);
                char* ip = inet_ntoa(sockaddr->sin_addr);
                char f_name_ch[260];
                char desc_ch[260];
                char df_char = ' ';

                WideCharToMultiByte(CP_ACP,0,adapter_addrs->FriendlyName,-1, f_name_ch,260, &df_char, nullptr);
                WideCharToMultiByte(CP_ACP,0,adapter_addrs->Description,-1, desc_ch,260, &df_char, nullptr);

                NetworkAdapterInfo adaptr;
                adaptr.id = std::string(adapter_addrs->AdapterName);
                adaptr.name = std::string(f_name_ch);
                adaptr.descr = std::string(desc_ch);
                adaptr.ip = std::string(ip);
                adapters.push_back(adaptr);
                unicast_addrs = unicast_addrs->Next;
            }
        }

        adapter_addrs = adapter_addrs->Next;
    }
#else
  // TODO
#endif

    // Return the ip interface maps.
    return adapters;
}

std::string getHostname()
{
    std::string name;

#ifdef _WIN32

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return "";

    char buffer[256];
    if (gethostname(buffer, sizeof(buffer)) != 0)
        WSACleanup();

    // Clear.
    WSACleanup();

    // Store the data.
    name = std::string(buffer);

#else
    // TODO
#endif

    // Return the hostname.
    return name;
}


std::string timePointToString(const HRTimePointStd &tp, const std::string &format, bool add_ms, bool add_ns, bool utc)
{
    // Stream to hold the formatted string and the return container.
    std::ostringstream ss;
    // Convert the time point to a duration and get the different time fractions.
    HRTimePointStd::duration dur = tp.time_since_epoch();
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
        return std::string();
    }
    // Return the container.
    return ss.str();
}

std::string timePointToIso8601(const HRTimePointStd &tp, bool add_ms, bool add_ns)
{
    // Return the ISO 8601 datetime.
    return timePointToString(tp, "%Y-%m-%dT%H:%M:%S", add_ms, add_ns) + 'Z';
}

std::string currentISO8601Date(bool add_ms)
{
    auto now = high_resolution_clock::now();
    return timePointToIso8601(now, add_ms);
}

unsigned getCurrentPID()
{
#if defined(_WIN32)
    return GetCurrentProcessId();
#elif defined(__unix__) || defined(__APPLE__) || defined(__linux__)
    return getpid();
#else
// Unsupported or unknown platform
    return 0;
#endif
}

}} // END NAMESPACES.
// =====================================================================================================================
