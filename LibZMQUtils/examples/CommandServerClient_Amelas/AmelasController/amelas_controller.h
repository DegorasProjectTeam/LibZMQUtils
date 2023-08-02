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
 * @file amelas_controller.h
 * @brief EXAMPLE FILE - This file contains the declaration of the AmelasController example class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2307.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <map>
#include <string>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/CommandServer>
#include <LibZMQUtils/Utils>
// =====================================================================================================================

// PROJECT INCLUDES
// =====================================================================================================================
#include "common.h"
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace controller{
// =====================================================================================================================

class AmelasController
{
public:

    AmelasController() :
        home_pos_({-1,-1})
    {}

    ControllerError setHomePosition(const AltAzPos& pos)
    {
        // Auxiliar result.
        ControllerError error = ControllerError::SUCCESS;

        // Check the provided values.
        if (pos.az >= 360.0 ||  pos.az < 0.0 || pos.el >= 90. || pos.el < 0.)
        {
            error = ControllerError::INVALID_POSITION;
        }
        else
        {
            this->home_pos_ = pos;
        }

        // Do things in the hardware.
        // Changues in PLC.

        // Log.
        std::string cmd_str = ControllerErrorStr[static_cast<int>(error)];
        std::cout << std::string(100, '-') << std::endl;
        std::cout<<"<AMELAS CONTROLLER>"<<std::endl;
        std::cout<<"-> SET_HOME_POSITION"<<std::endl;
        std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout<<"Az: "<<pos.az<<std::endl;
        std::cout<<"El: "<<pos.el<<std::endl;
        std::cout<<"Error: "<<static_cast<int>(error)<<" ("<<cmd_str<<")"<<std::endl;
        std::cout << std::string(100, '-') << std::endl;

        return error;
    }

    ControllerError getHomePosition(AltAzPos& pos)
    {
        pos = this->home_pos_;

        std::cout << std::string(100, '-') << std::endl;
        std::cout<<"<AMELAS CONTROLLER>"<<std::endl;
        std::cout<<"-> GET_HOME_POSITION"<<std::endl;
        std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
        std::cout << std::string(100, '-') << std::endl;

        return ControllerError::SUCCESS;
    }

    ControllerError getDatetime(std::string&)
    {
        return ControllerError::SUCCESS;
    }

private:

    AltAzPos home_pos_;

};

}} // END NAMESPACES.
// =====================================================================================================================

