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
#include <type_traits>
#include <atomic>
#include <iostream>
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

template<typename T, typename C>
void BinarySerializer::binarySerializeDeserialize(const T* src, size_t data_size_bytes, C* dst)
{
    // Check the types.
    (BinarySerializer::checkTriviallyCopyable<T>());
    (BinarySerializer::checkTrivial<T>());

    // Serialize the data.
    const std::byte* data_bytes = reinterpret_cast<const std::byte*>(src);
    std::byte* dest_bytes = reinterpret_cast<std::byte*>(dst);
    std::reverse_copy(data_bytes, data_bytes + data_size_bytes, dest_bytes);
}



template<typename T>
size_t BinarySerializer::calcSize(const T& value)
{
    if constexpr(std::is_base_of_v<Serializable, std::decay_t<T>>)
        return value.serializedSize();
    else if constexpr(std::is_same_v<std::decay_t<T>, std::string>)
        return sizeof(size_t) + value.size();
    else if constexpr(std::is_trivially_copyable_v<std::decay_t<T>> && std::is_trivial_v<std::decay_t<T>>)
        return sizeof(value);
    else
        static_assert(sizeof(T) == 0, "Unsupported type.");
}

template<typename T, typename... Args>
size_t BinarySerializer::writeRecursive(const T& value, const Args&... args)
{
    if constexpr (sizeof...(args) == 0)
        return this->writeSingle(value);
    else
        return this->writeSingle(value) + this->writeRecursive(std::forward<const Args&>(args)...);
}

//template<typename... Args>
//size_t BinarySerializer::write(const Args&... args)
//{
//    // Check the types.
//    (BinarySerializer::checkTriviallyCopyable<Args>(), ...);
//    (BinarySerializer::checkTrivial<Args>(), ...);

//    // Sum of sizes of all arguments and reserve.
//    const size_t total_size = (sizeof(args) + ... + 0);
//    reserve(this->size_ + total_size);

//    // Perform each write.
//    (void)std::initializer_list<int> { (this->writeSingle(args), 0)... };

//    // Return the total size.
//    return total_size;
//}



template<typename T, typename... Args, typename>
size_t BinarySerializer::write(const T& value, const Args&... args)
{
    // Calculate total size of all arguments.
    std::cout<<"Total size internal"<<std::endl;
    size_t total_size = (BinarySerializer::calcSize(value) + ... + BinarySerializer::calcSize(args));
    std::cout<<total_size<<std::endl;

    // Reserve space in one go.
    reserve(this->size_ + total_size);

    // Forward to recursive write function
    this->writeRecursive(value, args...);

    // Return the writed size.
    return total_size;
}

template<typename T>
typename std::enable_if<
        !std::is_base_of<Serializable, T>::value &&
        !std::is_same<std::nullptr_t &&, T>::value &&
        !std::is_pointer<T>::value, size_t>::type
BinarySerializer::writeSingle(const T& obj)
{
    // Check the types.
    (BinarySerializer::checkTriviallyCopyable<T>());
    (BinarySerializer::checkTrivial<T>());

    // Write single.
    reserve(this->size_ + sizeof(T));
    std::lock_guard<std::mutex> lock(this->mtx_);
    BinarySerializer::binarySerializeDeserialize(&obj, sizeof(T), this->data_.get() + size_);
    this->size_ += sizeof(T);

    // Return the size.
    return sizeof(T);
}


template<typename... Args>
void BinarySerializer::read(Args&... args)
{
    // Check the types.
    (BinarySerializer::checkTriviallyCopyable<Args>(), ...);
    (BinarySerializer::checkTrivial<Args>(), ...);

    // Read the data.
    (void)std::initializer_list<int>{ (readSingle(args), 0)... };
}

template<typename T>
T BinarySerializer::readSingle()
{
    // Check the types.
    (BinarySerializer::checkTriviallyCopyable<T>());
    (BinarySerializer::checkTrivial<T>());

    // Read single.
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
    // Check the types.
    (BinarySerializer::checkTriviallyCopyable<T>());
    (BinarySerializer::checkTrivial<T>());

    // Read single.
    std::lock_guard<std::mutex> lock(mtx_);
    if (this->offset_ + sizeof(T) > this->size_)
        throw std::out_of_range("BinarySerializer: Read beyond the data size");
    BinarySerializer::binarySerializeDeserialize(this->data_.get() + this->offset_, sizeof(T), &value);
    this->offset_ += sizeof(T);
}

template<typename T>
void BinarySerializer::checkTrivial()
{
    static_assert(std::is_trivial_v<T>, "Non-trivial types are not supported.");
}

template<typename T>
void BinarySerializer::checkTriviallyCopyable()
{
    static_assert(std::is_trivially_copyable_v<T>, "Non-trivially copyable types are not supported.");
}

template<typename... Args>
size_t BinarySerializer::fastSerialization(std::unique_ptr<std::byte>& out, const Args&... args)
{
    // Do the serialization
    BinarySerializer serializer;
    size_t size = serializer.write(std::forward<const Args&>(args)...);
    out = serializer.moveUnique();

    std::cout<<"Writing internal"<<std::endl;
    std::cout<<size<<std::endl;


    return size;
}

template<typename... Args>
void BinarySerializer::fastDeserialization(void* in, size_t size, Args&... args)
{
    // Do the deserialization.
    BinarySerializer serializer(in, size);
    serializer.read(std::forward<Args&>(args)...);
    if(!serializer.allReaded())
        throw std::out_of_range("BinarySerializer: Not all data was deserialized.");
}

}} // END NAMESPACES.
// =====================================================================================================================
