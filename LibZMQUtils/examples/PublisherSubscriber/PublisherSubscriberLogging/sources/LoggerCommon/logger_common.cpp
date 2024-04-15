#include "includes/LoggerCommon/logger_common.h"

zmqutils::utils::SizeUnit logger::AmelasLog::serialize(zmqutils::utils::BinarySerializer &serializer) const
{
    return serializer.write(this->level, this->str_info);
}

void logger::AmelasLog::deserialize(zmqutils::utils::BinarySerializer &serializer)
{
    serializer.read(this->level, this->str_info);
}

zmqutils::utils::SizeUnit logger::AmelasLog::serializedSize() const
{
    return zmqutils::utils::Serializable::calcTotalSize(this->level, this->str_info);
}
