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
 * @file common.h
 * @brief EXAMPLE FILE - This file contains common elements for the example AmelasController module.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <string>
#include <map>
#include <vector>
#include <variant>
#include <functional>
// =====================================================================================================================

#include <LibZMQUtils/Utils>

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace controller{
// =====================================================================================================================

// CONSTANTS
// =====================================================================================================================
// =====================================================================================================================

// CONVENIENT ALIAS, ENUMERATIONS AND CONSTEXPR
// =====================================================================================================================

class AmelasController;

enum class ControllerError : std::int32_t
{
    INVALID_ERROR = -1,
    SUCCESS = 0,
    INVALID_POSITION = 1,
    UNSAFE_POSITION = 2
};

static constexpr std::array<const char*, 21>  ControllerErrorStr
{
    "SUCCESS - Controller process success",
    "INVALID_POSITION - The provided position (az/alt) is invalid.",
    "UNSAFE_POSITION - The provided position (az/alt) is unsafe."
};

struct AltAzPos : public zmqutils::utils::Serializable
{
    inline AltAzPos(double az, double el):
        az(az), el(el){}

    inline AltAzPos(): az(-1), el(-1){}

    double az;
    double el;

    size_t serialize(zmqutils::utils::BinarySerializer& serializer) const final
    {
        return serializer.write(az, el);
    }

    void deserialize(zmqutils::utils::BinarySerializer& serializer) final
    {
        serializer.read(az, el);
    }

    size_t serializedSize() const final
    {
        return (2*sizeof(uint64_t) + sizeof(double)*2);
    }
};

// Generic callback.
template<typename... Args>
using AmelasControllerCallback = controller::ControllerError(AmelasController::*)(Args...);

// Callback function type aliases
using SetHomePositionCallback = std::function<ControllerError(const AltAzPos&)>;
using GetHomePositionCallback = std::function<ControllerError(AltAzPos&)>;
using GetDatetimeCallback = std::function<ControllerError(std::string&)>;

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
