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
 * @file binary_serializer.h
 * @brief This file contains the declaration of the BinarySerializer class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2307.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <cstring>
#include <mutex>
#include <atomic>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Utilities/binary_serializer.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{
// =====================================================================================================================

template<typename... Args>
void BinarySerializer::writeMultiple(const Args&... args)
{
    const size_t total_size = (... + sizeof(args)); // Sum of sizes of all arguments
    reserve(this->size_ + total_size);              // Reserve enough space once
    // Perform each write
    (void)std::initializer_list<int> { (writeSingle(args), 0)... };
}

template<typename T>
void BinarySerializer::writeSingle(const T& value)
{
    static_assert(std::is_trivially_copyable<T>::value, "Non-trivially copyable types are not supported.");
    reserve(this->size_ + sizeof(T));
    std::lock_guard<std::mutex> lock(this->mtx_);
    BinarySerializer::binarySerializeDeserialize(&value, sizeof(T), this->data_.get() + size_);
    this->size_ += sizeof(T);
}

template<typename... Args>
void BinarySerializer::readMultiple(Args&... args)
{
    (void)std::initializer_list<int>{ (readSingle(args), 0)... };
}

template<typename T>
T BinarySerializer::readSingle()
{
    static_assert(std::is_trivially_copyable<T>::value, "Non-trivially copyable types are not supported");
    std::lock_guard<std::mutex> lock(this->mtx_);
    if (this->offset_ + sizeof(T) > this->size_)
        throw std::out_of_range("BinarySerializer: Read beyond the data size");
    T value;
    BinarySerializer::binarySerializeDeserialize(this->data_.get() + this->offset_, sizeof(T), &value);
    this->offset_ += sizeof(T);
    return value;
}

template<typename T>
void BinarySerializer::readSingle(T& value)
{
    static_assert(std::is_trivially_copyable<T>::value, "Non-trivially copyable types are not supported");
    std::lock_guard<std::mutex> lock(mtx_);
    if (this->offset_ + sizeof(T) > this->size_)
        throw std::out_of_range("BinarySerializer: Read beyond the data size");
    BinarySerializer::binarySerializeDeserialize(this->data_.get() + this->offset_, sizeof(T), &value);
    this->offset_ += sizeof(T);
}

}} // END NAMESPACES.
// =====================================================================================================================
