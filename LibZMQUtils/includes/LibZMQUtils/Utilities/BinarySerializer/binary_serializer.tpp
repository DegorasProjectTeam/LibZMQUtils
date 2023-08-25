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
 * @version 2308.2
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
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{
// =====================================================================================================================

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

template<typename T, typename C>
void BinarySerializer::binarySerialize(const T *src, size_t data_size_bytes, C *dst)
{
    bool reverse = this->endianess_ == Endianess::LITTLE_ENDIAN;
    BinarySerializer::binarySerializeDeserialize(src, data_size_bytes, dst, reverse);
}

template<typename T, typename C>
void BinarySerializer::binaryDeserialize(const T *src, size_t data_size_bytes, C *dst)
{
    bool reverse = this->endianess_ == Endianess::LITTLE_ENDIAN;
    BinarySerializer::binarySerializeDeserialize(src, data_size_bytes, dst, reverse);
}

template<typename TSRC, typename TDEST>
void BinarySerializer::binarySerializeDeserialize(const TSRC* src, size_t data_size_bytes, TDEST* dst, bool reverse)
{
    // Check the types.
    (BinarySerializer::checkTriviallyCopyable<TDEST>());
    (BinarySerializer::checkTrivial<TDEST>());

    // Zero-fill the entire output first.
    std::memset(dst, 0, sizeof(TDEST));

    // Serialize the data.
    const std::byte* data_bytes = reinterpret_cast<const std::byte*>(src);
    std::byte* dest_bytes = reinterpret_cast<std::byte*>(dst);

    // Copy the data (with reverse if neccesary).
    if (reverse)
    {
        std::reverse_copy(data_bytes, data_bytes + data_size_bytes, dest_bytes);
    }
    else
    {
        std::copy(data_bytes, data_bytes + data_size_bytes, dest_bytes);
    }
}

template<typename T>
std::uint64_t BinarySerializer::calcTotalSize(const T& data)
{
    if constexpr(std::is_base_of_v<Serializable, std::decay_t<T>>)
    {
        return data.serializedSize();
    }
    else if constexpr(std::is_same_v<std::decay_t<T>, std::string>)
    {
        return sizeof(ElementSize) + data.size();
    }
    else if constexpr(std::is_trivially_copyable_v<std::decay_t<T>> && std::is_trivial_v<std::decay_t<T>>)
    {
        return sizeof(ElementSize) + sizeof(data);
    }
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

template<typename T, typename... Args>
void BinarySerializer::readRecursive(T& value, Args&... args)
{
    this->readSingle(value);

    if constexpr (sizeof...(args) > 0)
        this->readRecursive(std::forward<Args&>(args)...);
}

template<typename... Args>
size_t BinarySerializer::fastSerialization(std::unique_ptr<std::byte>& out, const Args&... args)
{
    // Do the serialization
    BinarySerializer serializer;
    const size_t size = serializer.write(std::forward<const Args&>(args)...);
    out = serializer.moveUnique();
    return size;
}

template<typename... Args>
void BinarySerializer::fastDeserialization(void* src, size_t size, Args&... args)
{
    // Do the deserialization.
    BinarySerializer serializer(src, size);
    serializer.read(std::forward<Args&>(args)...);
    if(!serializer.allReaded())
        throw std::out_of_range("BinarySerializer: Not all data was deserialized.");
}

template<typename T, typename... Args, typename>
size_t BinarySerializer::write(const T& value, const Args&... args)
{
    // Calculate total size of all arguments.
    const size_t t_size = (BinarySerializer::calcTotalSize(value) + ... + BinarySerializer::calcTotalSize(args));

    // Reserve space in one go.
    this->reserve(this->size_ + t_size);

    // Forward to recursive write function
    this->writeRecursive(value, args...);

    // Return the writed size.
    return t_size;
}

template<typename T, typename... Args>
void BinarySerializer::read(T& value, Args&... args)
{
    // Read the data.
    this->readRecursive(value, args...);
}

template<typename T>
typename std::enable_if_t<
        !BinarySerializer::is_container<T>::value &&
        !std::is_base_of_v<Serializable, T> &&
        !std::is_same_v<std::nullptr_t &&, T> &&
        !std::is_pointer_v<T>, size_t>
BinarySerializer::writeSingle(const T& data)
{
    // Check the types.
    (BinarySerializer::checkTriviallyCopyable<T>());
    (BinarySerializer::checkTrivial<T>());

    // Get the size of the data.
    const std::uint64_t total_size = BinarySerializer::calcTotalSize(data);
    constexpr std::uint64_t data_size = sizeof(T);

    // Reserve space.
    this->reserve(this->size_ + total_size);

    // Safety mutex.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Serialize the size of the data.
    BinarySerializer::binarySerialize(&data_size, sizeof(ElementSize), this->data_.get() + this->size_);
    this->size_ += sizeof(ElementSize);

    // Serialize the data.
    BinarySerializer::binarySerialize(&data, data_size, this->data_.get() + this->size_);
    this->size_ += data_size;

    // Return the size.
    return total_size;
}


template<typename T, size_t L>
size_t BinarySerializer::writeSingle(const std::array<T, L>& arr)
{
    // Check the types.
    BinarySerializer::checkTriviallyCopyable<T>();
    BinarySerializer::checkTrivial<T>();

    // Get the total size and reserve.
    constexpr std::uint64_t array_size = L;
    constexpr std::uint64_t elem_size = sizeof(T);
    constexpr std::uint64_t total_size = sizeof(ElementSize) + sizeof(ElementSize) + elem_size*array_size;
    this->reserve(this->size_ + total_size);

    // Safety mutex.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Serialize array size.
    BinarySerializer::binarySerialize(&array_size, sizeof(ElementSize), this->data_.get() + size_);
    this->size_ += sizeof(ElementSize);

    // Serialize the size of each element.
    BinarySerializer::binarySerialize(&elem_size, sizeof(ElementSize), this->data_.get() + size_);
    this->size_ += sizeof(ElementSize);

    // Write each value of the array.
    for(const auto& val : arr)
    {
        BinarySerializer::binarySerialize(&val, elem_size, this->data_.get() + this->size_);
        this->size_ += elem_size;
    }

    // Return the writed size.
    return total_size;
}

template<typename T>
size_t BinarySerializer::writeSingle(const std::vector<T>& v)
{
    // Check the types.
    BinarySerializer::checkTriviallyCopyable<T>();
    BinarySerializer::checkTrivial<T>();

    // Get the total size and reserve.
    std::uint64_t total_size = sizeof(ElementSize) + sizeof(T)*v.size();
    this->reserve(this->size_ + total_size);

    // Serialize size.
    this->writeSingle(sizeof(T)*v.size());

    // Write each value of the vector.
    for(const auto& val : v)
        this->writeSingle(val);

    // Return the writed size.
    return total_size;
}

template<typename T>
typename std::enable_if<
        !BinarySerializer::is_container<T>::value &&
        !std::is_base_of_v<Serializable, T> &&
        !std::is_same_v<std::nullptr_t &&, T> &&
        !std::is_pointer_v<T>,
    void>::type
BinarySerializer::readSingle(T& value)
{
    // Check the types.
    (BinarySerializer::checkTriviallyCopyable<T>());
    (BinarySerializer::checkTrivial<T>());

    // Safety mutex.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Ensure that there's enough data left to read the size of the value.
    if (this->offset_ + sizeof(ElementSize) > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the value.");

    // Read the size of the value.
    size_t size;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(ElementSize), &size);

    // Update the offset.
    this->offset_ += sizeof(ElementSize);

    // Check if we have enough data left to read the value.
    if (this->offset_ + size > this->size_)
        throw std::out_of_range("BinarySerializer: Read value beyond the data size.");

    // Check if the size is greater than the expected size for the type.
    if (size > sizeof(T))
        throw std::logic_error("BinarySerializer: The serialized value size is greater than type for storage.");

    // Read the value.
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, size, &value);
    this->offset_ += size;
}

// For arrays of trivial types.
template<typename T, size_t L>
void BinarySerializer::readSingle(std::array<T, L>& arr)
{
    // Safety mutex.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Ensure that there's enough data left to read the size of the array.
    if (this->offset_ + sizeof(ElementSize) > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the array.");

    // Read the size of the array.
    std::uint64_t size_array;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(ElementSize), &size_array);

    // Update the offset.
    this->offset_ += sizeof(ElementSize);

    // Check if the array is empty.
    if(size_array == 0)
        return;

    // Ensure that there's enough data left to read the size of each elements.
    if (this->offset_ + sizeof(ElementSize) > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the size of elements of the array.");

    // Read the size of the elements.
    std::uint64_t size_elem;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(ElementSize), &size_elem);

    // Update the offset.
    this->offset_ += sizeof(ElementSize);

    // Check if the size is 0.
    if(size_elem == 0)
        throw std::out_of_range("BinarySerializer: Unknow size of elements of the array.");

    // Check if we have enough data left to read the string.
    if (this->offset_ + size_elem*size_array > this->size_)
        throw std::out_of_range("BinarySerializer: Read string beyond the data size.");

    // Read all the elemnts.
    for(std::uint64_t i = 0; i < size_array; i++)
    {
        BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, size_elem, &arr[i]);
        this->offset_ += size_elem;
    }
}

}} // END NAMESPACES.
// =====================================================================================================================
