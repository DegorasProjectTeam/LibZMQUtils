/** ********************************************************************************************************************
 * @file amelas_log.cpp
 * @brief EXAMPLE FILE - This file contains the definition of the AmelasLog example struct.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// PROJECT INCLUDES
// =====================================================================================================================
#include "AmelasController/amelas_log.h"
// =====================================================================================================================

// NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace controller{
// =====================================================================================================================

zmqutils::utils::SizeUnit AmelasLog::serialize(zmqutils::utils::BinarySerializer &serializer) const
{
    return serializer.write(this->level, this->str_info);
}

void AmelasLog::deserialize(zmqutils::utils::BinarySerializer &serializer)
{
    serializer.read(this->level, this->str_info);
}

zmqutils::utils::SizeUnit AmelasLog::serializedSize() const
{
    return zmqutils::utils::Serializable::calcTotalSize(this->level, this->str_info);
}

}} // END NAMESPACES.
// =====================================================================================================================
