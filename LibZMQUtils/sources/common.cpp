#include "LibZMQUtils/common.h"



zmqutils::common::HostClientInfo::HostClientInfo(const std::string &ip, const std::string &name, const std::string &pid, const std::string &info) :
    ip(ip),
    hostname(name),
    pid(pid),
    info(info)
{
    // Create the host client internal identification.
    this->id = ip + "//" + name + "//" + pid;
}
