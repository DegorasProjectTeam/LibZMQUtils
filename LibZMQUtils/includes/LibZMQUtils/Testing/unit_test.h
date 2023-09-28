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
 * @file unit_test.h
 * @brief This file contains the declaration of the UnitTest class and related.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
#include <sstream>
#include <list>
#include <map>
#include <vector>
// =====================================================================================================================

// LIBDPSLR INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
#include <LibZMQUtils/Utilities/utils.h>
// =====================================================================================================================

// DPSLR NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace testing{
// =====================================================================================================================


struct TestLog
{

public:

    LIBZMQUTILS_EXPORT TestLog(const std::string& module, const std::string& test, const std::string &det_ex,
                               bool passed, const utils::TimePointStd &tp, long long elapsed);

    LIBZMQUTILS_EXPORT std::string makeLog(const std::string& storage_path = std::string()) const;

    LIBZMQUTILS_EXPORT const std::string& getModuleName() const;

    LIBZMQUTILS_EXPORT bool getResult() const;

private:

    // Format result helper.
    std::string formatResult() const;

    // Stringstreams.
    std::string module_;
    std::string test_;
    std::string det_ex_;
    bool passed_;
    std::string tp_str_;
    long long elapsed_;
};


class TestSummary
{

public:

    LIBZMQUTILS_EXPORT TestSummary();

    LIBZMQUTILS_EXPORT void setSessionName(const std::string& name);

    LIBZMQUTILS_EXPORT void addLog(const TestLog& log);

    LIBZMQUTILS_EXPORT void clear();

    LIBZMQUTILS_EXPORT void makeSummary(bool show = true, const std::string& storage_path = std::string()) const;

private:

    std::multimap<std::string, TestLog> test_logs_;
    std::string session_;
    unsigned n_pass_;
    unsigned n_fail_;
};

class TestBase
{
public:

protected:

    LIBZMQUTILS_EXPORT TestBase(const std::string& name);

public:

    // Type traits.
    template <typename T>
    struct is_container : std::false_type {};

    template <typename... Args>
    struct is_container<std::vector<Args...>> : std::true_type {};

    template <typename T, size_t N>
    struct is_container<std::array<T, N>> : std::true_type {};

    LIBZMQUTILS_EXPORT bool forceFail();

    template<typename T>
    typename std::enable_if_t<
            !is_container<T>::value &&
            !std::is_floating_point_v<T>, bool>
    expectEQ(const T& arg1, const T& arg2)
    {
        bool result = (arg1 == arg2);
        return result;
    }

    template<typename T>
    typename std::enable_if_t<
            !is_container<T>::value &&
            std::is_floating_point_v<T>, bool>
    expectEQ(const T& arg1, const T& arg2, const T& tolerance = std::numeric_limits<T>::epsilon())
    {
        return std::abs(arg1 - arg2) <= tolerance;
    }

    template<typename T>
    typename std::enable_if_t<
            !std::is_floating_point_v<T>, bool>
    expectEQ(const std::vector<T>& v1, const std::vector<T>& v2)
    {
        if (v1.size() != v2.size())
            return false;

        for (size_t i = 0; i < v1.size(); ++i)
        {
            if (v1[i] != v2[i])
                return false;
        }
        return true;
    }

    template<typename T>
    typename std::enable_if_t<
            std::is_floating_point_v<T>, bool>
    expectEQ(const std::vector<T>& v1, const std::vector<T>& v2,  const T& tol = std::numeric_limits<T>::epsilon())
    {
        if (v1.size() != v2.size())
            return false;

        for (size_t i = 0; i < v1.size(); ++i)
        {
            if (std::abs(v1[i] - v2[i]) > tol)
                return false;
        }
        return true;
    }

    template <typename T, size_t N>
    typename std::enable_if_t<!std::is_floating_point_v<T>, bool>
    expectEQ(const std::array<T, N>& arr1, const std::array<T, N>& arr2)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (arr1[i] != arr2[i])
                return false;
        }
        return true;
    }

    template <typename T, size_t N>
    typename std::enable_if_t<std::is_floating_point_v<T>, bool>
    expectEQ(const std::array<T, N>& arr1, const std::array<T, N>& arr2, const T& tol = std::numeric_limits<T>::epsilon())
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (std::abs(arr1[i] - arr2[i]) > tol)
                return false;
        }
        return true;
    }

    template<typename T>
    typename std::enable_if_t<
            !is_container<T>::value &&
            !std::is_floating_point_v<T>, bool>
    expectNE(const T& arg1, const T& arg2)
    {
        bool result = (arg1 != arg2);
        return result;
    }

    template<typename T>
    typename std::enable_if_t<
            !is_container<T>::value &&
            std::is_floating_point_v<T>, bool>
    expectNE(const T& arg1, const T& arg2, const T& tolerance = std::numeric_limits<T>::epsilon())
    {
        return std::abs(arg1 - arg2) > tolerance;
    }

    LIBZMQUTILS_EXPORT virtual void runTest();

    LIBZMQUTILS_EXPORT virtual ~TestBase();

    std::string test_name_;
    bool result_;
};



class UnitTest
{

public:

    // Deleting the copy constructor.
    UnitTest(const UnitTest& obj) = delete;

    LIBZMQUTILS_EXPORT static UnitTest& instance();

    LIBZMQUTILS_EXPORT void setSessionName(std::string&& session);

    LIBZMQUTILS_EXPORT void addTest(std::pair<std::string, TestBase*> p);

    LIBZMQUTILS_EXPORT void runTests();

    LIBZMQUTILS_EXPORT void clear();

private:

    UnitTest() = default;

    // Members.
    std::multimap<std::string, TestBase*> test_dict_;
    TestSummary summary_;
    std::string session_;

};

}} // END NAMESPACES.
// =====================================================================================================================
