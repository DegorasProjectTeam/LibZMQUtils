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
 * @file string_helpers.h
 * @brief This file contains several helper tools related with strings.
 * @warning Not exported. Only for internal library usage.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
//======================================================================================================================
#include <string>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace internal_helpers{
namespace strings{
// =====================================================================================================================

std::string toUpper(const std::string& str);

std::string toLower(const std::string& str);

std::string ltrim(const std::string& str);

std::string rtrim(const std::string& str);

std::string trim(const std::string& str);

std::string rmLastLineBreak(const std::string& str);

std::string replaceStr(const std::string& str, const std::string& target, const std::string& repl);

std::string fillStr(const std::string& fillChar, size_t width);

template <class Container>
void split (Container& result, const std::string& s, const std::string& delimiters, bool empties = true);

template <class Container>
Container split (const std::string& s, const std::string& delimiters, bool empties = true );

template <typename Container>
std::string join(const Container& strings, const std::string& del = " ");

template<typename T>
std::string numberToFixstr(T x, unsigned int prec);

template<typename T>
std::string numberToStr(T x, unsigned int prec, unsigned int dec_places, bool fixed = true);

}}} // END NAMESPACES
// =====================================================================================================================

// TEMPLATES INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/InternalHelpers/string_helpers.tpp"
// =====================================================================================================================
