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
 * @file container_helpers.h
 * @brief This file contains several helper tools related with containers.
 * @warning Not exported. Only for internal library usage.
 * @author Degoras Project Team.
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
//======================================================================================================================
#include <string>
#include <vector>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace internal_helpers{
namespace containers{
// =====================================================================================================================

// Custom search closest.
template <typename T>
std::size_t searchClosest(const std::vector<T>& sorted_array, T x);

// Custom push back.
template <class Container>
void insert(Container& a, const Container& b);

// Custom contains helper.
template <typename Container, typename T>
bool contains(const Container& container, T elem);

template <typename Container, typename T>
bool find(const Container& container, const T  & elem, int& pos);

// Extract from a vector the input indexes.
template<typename T>
std::vector<T> extract(const std::vector<T>& data, const std::vector<size_t>& indexes);

template<typename T>
std::vector<T> dataBetween(const std::vector<T>& v, T lower, T upper);

template <class T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v);

template <typename Container>
std::vector<typename Container::key_type> getMapKeys(const Container& container);

}}} // END NAMESPACES.
// =====================================================================================================================

// TEMPLATES INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/InternalHelpers/container_helpers.tpp"
// =====================================================================================================================
