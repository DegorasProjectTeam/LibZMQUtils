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
 * @file network_helpers.cpp
 * @brief This file contains the implementation of several helper tools related with networking.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// SYSTEM DEPENDANT INCLUDES
// =====================================================================================================================
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#elif _WIN32_WINNT < 0x0600
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#elif __unix__
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netdb.h>
#endif
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <vector>
#include <regex>
// =====================================================================================================================

// LIBDPSLR INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/InternalHelpers/network_helpers.h>
// =====================================================================================================================

// DPSLR NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace internal_helpers{
namespace network{
// =====================================================================================================================

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

#elif __unix__

    char buffer[256];
    if (gethostname(buffer, sizeof(buffer)) != 0)
        return "";

    name = std::string(buffer);

#endif

    // Return the hostname.
    return name;
}

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

#elif __unix__

    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return adapters;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        // Check it is IP4
        // We use IPv4 for simplicity, change to AF_INET6 for IPv6
        if (family == AF_INET)
        {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
                break;

            NetworkAdapterInfo adapter;
            adapter.id = std::string(ifa->ifa_name);
            adapter.name = std::string(ifa->ifa_name); // Typically, the friendly name isn't available like in Windows.
            adapter.descr = "";                        // Description is also typically not available
            adapter.ip = std::string(host);
            adapters.push_back(adapter);
        }
    }

    freeifaddrs(ifaddr);

#endif

    // Return the ip interface maps.
    return adapters;
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

bool isValidIP(const std::string &ipAddress)
{
    // Regular expression pattern for both IPv4 and IPv6 addresses.
    std::regex pattern(
        R"(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)"
        R"(|^([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}$)"
        R"(|^([0-9a-fA-F]{1,4}:){1,7}:$)"
        R"(|^([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}$)"
        R"(|^([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}$)"
        R"(|^([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}$)"
        R"(|^([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}$)"
        R"(|^([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}$)"
        R"(|^[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})?$)"
        );

    // Check if the input string matches the pattern.
    return std::regex_match(ipAddress, pattern);
}

}}} // END NAMESPACES
// =====================================================================================================================
