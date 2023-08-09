/***********************************************************************************************************************
 *   LibDPSLR (Degoras Project SLR Library): A libre base library for SLR related developments.                        *                                      *
 *                                                                                                                     *
 *   Copyright (C) 2023 Degoras Project Team                                                                           *
 *                      < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >                             *
 *                      < Jesús Relinque Madroñal >                                                                    *
 *                                                                                                                     *
 *   This file is part of LibDPSLR.                                                                                    *
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

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <list>
#include <map>
#include <vector>
// =====================================================================================================================

// LIBDPSLR INCLUDES
// =====================================================================================================================
#include "LibDPSLR/libdpslr_global.h"
#include <LibDPSLR/Timing/time_utils.h>
// =====================================================================================================================

// DPSLR NAMESPACES
// =====================================================================================================================
namespace dpslr{
namespace testing{
// =====================================================================================================================


struct TestLog
{

public:

    TestLog(const std::string& module, const std::string& test, const std::string &det_ex,
            bool passed, const dpslr::timing::common::HRTimePointStd &tp, long long elapsed);

    TestLog(const TestLog&) = default;

    std::string makeLog(const std::string& storage_path = std::string()) const;

    const std::string& getModuleName() const;

    bool getResult() const;

    ~TestLog() = default;

private:

    std::string formatResult() const;

    // Stringstreams.
    std::string module_;
    std::string test_;
    std::string det_ex_;
    bool passed_;
    std::string tp_str_;
    long long elapsed_;
};


class LIBDPSLR_EXPORT TestSummary
{
public:

    TestSummary();

    void setSessionName(const std::string& name);

    void addLog(const TestLog& log);

    void clear();

    void makeSummary(bool show = true, const std::string& storage_path = std::string()) const;

    ~TestSummary() = default;

private:

    std::multimap<std::string, TestLog> test_logs_;
    std::string session_;
    unsigned n_pass_;
    unsigned n_fail_;
};

class TestBase
{

protected:

    TestBase(const std::string& name):
        test_name_(name),
        result_(true)
    {}

public:

    template<typename T>
    inline bool expectEQ(const T& arg1, const T& arg2)
    {
        bool result = (arg1 == arg2);
        return result;
    }

    template<typename T>
    inline bool expectNE(const T& arg1, const T& arg2)
    {
        bool result = (arg1 != arg2);
        return result;
    }

    virtual void runTest(){};

    std::string test_name_;
    bool result_;
};



class LIBDPSLR_EXPORT UnitTest
{

public:

    // Deleting the copy constructor.
    UnitTest(const UnitTest& obj) = delete;

    inline static UnitTest& instance()
    {
        static UnitTest uTest;
        return uTest;
    }

    void setSessionName(std::string&& session)
    {
        this->session_ = std::move(session);
        this->summary_.setSessionName(this->session_);
    }

    void addTest(std::pair<std::string, TestBase*> p)
    {
        this->test_dict_.insert(p);
    }

    void runTests();

    void clear()
    {
        this->test_dict_.clear();
        this->summary_.clear();
    }

    template<typename T>
    inline bool expectEQ(const T& arg1, const T& arg2)
    {
        bool result = (arg1 == arg2);
        return result;
    }

    template<typename T>
    inline bool expectNE(const T& arg1, const T& arg2)
    {
        bool result = (arg1 != arg2);
        return result;
    }

    virtual ~UnitTest() {}

private:

    UnitTest() {}

    // Members.
    std::multimap<std::string, TestBase*> test_dict_;
    TestSummary summary_;
    std::string session_;
};

}} // END NAMESPACES.
// =====================================================================================================================
