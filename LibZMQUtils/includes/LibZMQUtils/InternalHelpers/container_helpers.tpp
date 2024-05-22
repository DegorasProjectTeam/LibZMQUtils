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
 * @file container_helpers.tpp
 * @brief This file contains the template implementation of several helper tools related with containers.
 * @warning Not exported. Only for internal library usage.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
//======================================================================================================================
#include <string>
#include <limits>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <iterator>
#include <cmath>
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace internal_helpers{
namespace containers{
// =====================================================================================================================

template <typename T>
std::size_t searchClosest(const std::vector<T>& sorted_array, T x)
{
    auto iter_geq = std::lower_bound(sorted_array.begin(), sorted_array.end(), x);

    if (iter_geq == sorted_array.begin())
        return 0;

    if (iter_geq == sorted_array.end())
        return sorted_array.size() - 1;

    T a = *(iter_geq - 1);
    T b = *iter_geq;

    if (std::abs(x - a) < std::abs(x - b))
        return std::distance(sorted_array.begin(), iter_geq - 1);

    return std::distance(sorted_array.begin(), iter_geq);
}

template <class Container>
void insert(Container& a, const Container& b)
{
    a.insert(a.end(), b.begin(), b.end());
}

template <typename Container, typename T>
bool contains(const Container& container, T elem)
{
    const auto it = std::find(container.begin(), container.end(), elem);
    return (it != container.end());
}

template <typename K, typename T>
bool contains(const std::map<K,T> &map, K key)
{
    const auto it = map.find(key);
    return (it != map.end());
}

template <typename Container, typename T>
bool find(const Container& container, const T  & elem, int& pos)
{
    bool result = false;
    auto it = std::find(container.begin(), container.end(), elem);
    if (it != container.end())
    {
        pos = std::distance(container.begin(), it);
        result = true;
    }
    return result;
}

template<typename T>
std::vector<T> extract(const std::vector<T>& data, const std::vector<size_t>& indexes)
{
    // Auxiliar container.
    std::vector<T> result;
    // Extract the data.
    for(const std::size_t& index : indexes)
        result.push_back(data[index]);
    // Return the result.
    return result;
}

template<typename T>
std::vector<T> dataBetween(const std::vector<T>& v, T lower, T upper)
{
    auto l = std::lower_bound(v.begin(), v.end(), lower);
    auto u = std::upper_bound(v.begin(), v.end(), upper);
    return std::vector<T>(l, u);
}

template <class T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v)
{
    out << '[';
    if (!v.empty())
        std::copy (v.begin(), v.end(), std::ostream_iterator<T>(out, ","));
    out << "\b]";
    return out;
}

template <typename Container>
std::vector<typename Container::key_type> getMapKeys(const Container& container)
{
    std::vector<typename Container::key_type> keys;
    for(auto it = container.begin(), end = container.end();
         it != end; it = container.equal_range(it->first).second)
    {
        keys.push_back(it->first);
    }
    return keys;
}

}}}// END NAMESPACES.
// =====================================================================================================================
