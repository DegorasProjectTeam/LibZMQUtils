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

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Modules/Utilities>
#include <LibZMQUtils/Modules/Testing>
// =====================================================================================================================

// =====================================================================================================================
using zmqutils::utils::ConsoleConfig;
// =====================================================================================================================

// TODO IMPROVE SENDING AUTOMATIC Ctl_c for example.

// Basic tests.
M_DECLARE_UNIT_TEST(ConsoleConfig, BasicTest1)
M_DECLARE_UNIT_TEST(ConsoleConfig, BasicTest2)

// MOVE THIS TO OTHER TEST
M_DECLARE_UNIT_TEST(ConsoleRedirect, BasicTest1)

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

M_DEFINE_UNIT_TEST(ConsoleRedirect, BasicTest1)
{

}

int main()
{

    std::cout << "Start test..." << std::endl;

    // Configure the console.
    zmqutils::utils::ConsoleConfig& console_cfg = zmqutils::utils::ConsoleConfig::getInstance();
    console_cfg.configureConsole(false, false, true);

    zmqutils::utils::DebugConsole console1("Debug Console 1");
    zmqutils::utils::DebugConsole console2("Debug Console 2");

    if (console1.startProcess())
    {
       console1.sendString("Hello, Debug Console 1!");
       console1.sendString("This is a test message.");
    }

    if (console2.startProcess())
    {
       console2.sendString("Hello, Debug Console 2!");
       console2.sendString("Another test message.");
    }

    std::cout << "Press Enter to close all debug consoles.\n";
    std::cin.get();

    console1.closeProcess();
    console2.closeProcess();


    std::cout << "End test..." << std::endl;

    // Close the console (cleanup)
    //consoleProcess.closeProcess();


    // Restore the console.
    //console_cfg.restoreConsole();

    // ENABLE THIS TEST ONLY IF NECESSARY

    // Start of the session.
   // M_START_UNIT_TEST_SESSION("LibZMQUtils ConsoleConfig Session")

    // Register the tests.
    //M_REGISTER_UNIT_TEST(ConsoleConfig, BasicTest1)
    //M_REGISTER_UNIT_TEST(ConsoleConfig, BasicTest2)
   // M_REGISTER_UNIT_TEST(ConsoleRedirect, BasicTest1)

    // Run the unit tests.
  // M_RUN_UNIT_TESTS()
}
