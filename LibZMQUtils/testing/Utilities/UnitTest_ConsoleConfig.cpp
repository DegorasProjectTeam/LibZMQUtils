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

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
#include <vector>
#include <fstream>
#include <stdio.h>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/Utilities>
#include <LibZMQUtils/Testing/UnitTest>
// =====================================================================================================================

// =====================================================================================================================
using zmqutils::utils::ConsoleConfig;
// =====================================================================================================================

// Basic tests.
M_DECLARE_UNIT_TEST(ConsoleConfig, BasicTest1)
M_DECLARE_UNIT_TEST(ConsoleConfig, BasicTest2)

// Implementations.

M_DEFINE_UNIT_TEST(ConsoleConfig, BasicTest1)
{
    std::cout << "Start test..." << std::endl;

    // Configure the console.
    zmqutils::utils::ConsoleConfig& console_cfg = zmqutils::utils::ConsoleConfig::getInstance();
    console_cfg.configureConsole(true, true, false);

    // Wait for closing as an infinite loop until ctrl-c.
    console_cfg.waitForClose();

    std::cout << "End test..." << std::endl;

    // Restore the console.
    console_cfg.restoreConsole();
}

M_DEFINE_UNIT_TEST(ConsoleConfig, BasicTest2)
{
    std::cout << "Start test..." << std::endl;

    // Configure the console.
    zmqutils::utils::ConsoleConfig& console_cfg = zmqutils::utils::ConsoleConfig::getInstance();
    console_cfg.configureConsole(true, false, true);

    // Wait for closing as an infinite loop until ctrl-c.
    console_cfg.waitForClose();

    std::cout << "End test..." << std::endl;

    // Restore the console.
    console_cfg.restoreConsole();
}

int main()
{
    // ENABLE THIS TEST ONLY IF NECESSARY

    // Start of the session.
    M_START_UNIT_TEST_SESSION("LibZMQUtils ConsoleConfig Session")

    // Register the tests.
    //M_REGISTER_UNIT_TEST(ConsoleConfig, BasicTest1)
    //M_REGISTER_UNIT_TEST(ConsoleConfig, BasicTest2)

    // Run the unit tests.
    M_RUN_UNIT_TESTS()
}
