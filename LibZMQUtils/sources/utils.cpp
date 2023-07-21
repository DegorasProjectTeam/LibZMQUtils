



#include <iomanip>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#else
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <sstream>
#include <iphlpapi.h>

#include "LibZMQUtils/utils.h"

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

                WideCharToMultiByte(CP_ACP,0,adapter_addrs->FriendlyName,-1, f_name_ch,260, &df_char, NULL);
                WideCharToMultiByte(CP_ACP,0,adapter_addrs->Description,-1, desc_ch,260, &df_char, NULL);

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

void binarySerializeDeserialize(const void *data, size_t data_size_bytes, void *dest)
{
    const std::uint8_t* data_byes = reinterpret_cast<const std::uint8_t *>(data);
    std::uint8_t* dest_byes = reinterpret_cast<std::uint8_t*>(dest);
    std::reverse_copy(data_byes, data_byes + data_size_bytes, dest_byes);
}

std::string timePointToString(const HRTimePointStd &tp, const std::string &format, bool add_ms, bool add_ns, bool utc)
{
    // Stream to hold the formatted string and the return container.
    std::ostringstream ss;
    // Convert the time point to a duration and get the different time fractions.
    HRTimePointStd::duration dur = tp.time_since_epoch();
    const time_t secs = duration_cast<std::chrono::seconds>(dur).count();
    const long long mill = duration_cast<std::chrono::milliseconds>(dur).count();
    const unsigned long long ns = duration_cast<std::chrono::nanoseconds>(dur).count();
    const unsigned long long s_ns = secs * 1e9;
    const unsigned long long t_ns = (ns - s_ns);
    // Format the duration.
    if (const std::tm *tm = (utc ? std::gmtime(&secs) : std::localtime(&secs)))
    {
        ss << std::put_time(tm, format.c_str());
        if(add_ms && !add_ns)
            ss << '.' << std::setw(3) << std::setfill('0') << (mill - secs * 1e3);
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

int getCurrentPID()
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
