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
 * @file unit_test_macros.h
 * @brief
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <stdlib.h>
#include <thread>
#include <chrono>
// =====================================================================================================================

// LIBDPSLR INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Testing/unit_test.h"
// =====================================================================================================================

// =====================================================================================================================
using zmqutils::testing::TestBase;
using zmqutils::testing::UnitTest;
using zmqutils::testing::TestSummary;
// =====================================================================================================================

// MACROS
// =====================================================================================================================

#define M_START_UNIT_TEST_SESSION(SessionName)                    \
UnitTest::instance().clear();                                     \
UnitTest::instance().setSessionName(std::string(SessionName));    \

#define M_DECLARE_UNIT_TEST(Module, TestName)               \
                           \
class Test_##Module##_##TestName : public TestBase          \
{                                                           \
        Test_##Module##_##TestName(): TestBase(#TestName){} \
        public:                                             \
        static Test_##Module##_##TestName* instance()       \
    {                                                       \
            static Test_##Module##_##TestName test;         \
            return &test;                                   \
    }                                                       \
        void runTest() override;                            \
};                                                          \

#define M_DEFINE_UNIT_TEST(Module, TestName)    \
void Test_##Module##_##TestName::runTest()      \

#define M_REGISTER_UNIT_TEST(Module, TestName)                                                     \
    UnitTest::instance().addTest(                                                                  \
            std::pair<std::string, TestBase*>(#Module, Test_##Module##_##TestName::instance()));   \

#define M_RUN_UNIT_TESTS()            \
    UnitTest::instance().runTests();  \
    return 0;                         \

#define M_EXPECTED_EQ(arg1, arg2) \
[&] { bool expected = this->result_.load(); \
this->result_.compare_exchange_strong(expected, expected && expectEQ(arg1, arg2)); }();

#define M_EXPECTED_EQ_F(arg1, arg2, eps) \
[&] { bool expected = this->result_.load(); \
this->result_.compare_exchange_strong(expected, expected && expectEQ(arg1, arg2, eps)); }();

#define M_EXPECTED_NE(arg1, arg2) \
[&] { bool expected = this->result_.load(); \
this->result_.compare_exchange_strong(expected, expected && expectNE(arg1, arg2)); }();

#define M_FORCE_FAIL() \
[&] { bool expected = this->result_.load(); \
this->result_.compare_exchange_strong(expected, expected && forceFail()); }();

#define M_SLEEP_US(arg1)                                        \
std::this_thread::sleep_for(std::chrono::microseconds(arg1));   \
 \
// =====================================================================================================================
