/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *
 *   ExamplesLibZMQUtils related project.                                                                            *
 *                                                                                                        *
 *   A modern open-source C++ library with high-level utilities based on the well-known ZeroMQ open-source universal   *
 *   messaging library. Includes custom command based server-client and publisher-subscriber with automatic binary     *
 *   serialization capabilities, specially designed for system infraestructure. Developed as a free software under the *
 *   context of Degoras Project for the Spanish Navy Observatory SLR station (SFEL) in San Fernando and, of course,    *
 *   for any other station that wants to use it!                                                                       *
 *                                                                                                                     *
 *   Copyright (C) 2024 Degoras Project Team                                                                           *
 *                      < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >                             *
 *                      < Jesús Relinque Madroñal >                                                                    *                                                            *
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
 * @file amelas_controller.cpp
 * @brief EXAMPLE FILE - This file contains the implementation of the AmelasController example class.
 * @author Degoras Project Team
 * @copyright EUPL License
***********************************************************************************************************************/

// PROJECT INCLUDES
// =====================================================================================================================
#include "includes/AmelasController/amelas_controller.h"
// =====================================================================================================================

// AMELAS NAMESPACES
// =====================================================================================================================
namespace amelas{
namespace controller{

AmelasController::AmelasController() :
    home_pos_({-1,-1})
{}

AmelasError AmelasController::setHomePosition(const AltAzPos &pos)
{
    // Auxiliar result.
    AmelasError error = AmelasError::SUCCESS;

    // Check the provided values.
    if (pos.az >= 360.0 ||  pos.az < 0.0 || pos.el >= 90. || pos.el < 0.)
    {
        error = AmelasError::INVALID_POSITION;
    }
    else
    {
        this->home_pos_ = pos;
    }

    // Do things in the hardware (PLC) or FPGA.
    // this->doPLCSetHomePosition(isdhfkljsdhilfhlisd)
    // WARNING: Remember use async if the tasks are computationally demanding.
    // [...]

    // Log.
    std::string cmd_str = ControllerErrorStr[static_cast<size_t>(error)];
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

AmelasError AmelasController::getHomePosition(AltAzPos &pos)
{
    pos = this->home_pos_;

    std::cout << std::string(100, '-') << std::endl;
    std::cout<<"<AMELAS CONTROLLER>"<<std::endl;
    std::cout<<"-> GET_HOME_POSITION"<<std::endl;
    std::cout<<"Time: "<<zmqutils::utils::currentISO8601Date()<<std::endl;
    std::cout << std::string(100, '-') << std::endl;

    return AmelasError::SUCCESS;
}

AmelasError AmelasController::getDatetime(std::string &)
{
    return AmelasError::SUCCESS;
}

// =====================================================================================================================

}} // END NAMESPACES.
// =====================================================================================================================
