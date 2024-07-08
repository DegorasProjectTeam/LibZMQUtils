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
 * @file binary_serializer.tpp
 * @brief This file contains the template implementation part of the BinarySerializer class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
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
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
// -----------------------------------------------------------------------
#include "LibZMQUtils/InternalHelpers/file_helpers.h"
// WARNING
// REMEMBER COMMENT THIS INCLUDE TO AVOID CIRCULAR DEPENDENCIES
// UNCOMMENT ONLY TO HELP THE DEVELOPMENT WITH CLANG
//#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
// -----------------------------------------------------------------------
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace serializer{
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
void BinarySerializer::binarySerialize(const T *src, SizeUnit data_size_bytes, C *dst)
{
    bool reverse = this->endianess_ == Endianess::LT_ENDIAN;
    BinarySerializer::binarySerializeDeserialize(src, data_size_bytes, dst, reverse);
}

template<typename T, typename C>
void BinarySerializer::binaryDeserialize(const T *src, SizeUnit data_size_bytes, C *dst)
{
    bool reverse = this->endianess_ == Endianess::LT_ENDIAN;
    BinarySerializer::binarySerializeDeserialize(src, data_size_bytes, dst, reverse);
}

template<typename TSRC, typename TDEST>
void BinarySerializer::binarySerializeDeserialize(const TSRC* src, SizeUnit data_size_bytes, TDEST* dst, bool reverse)
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

template<typename T, typename... Args>
SizeUnit BinarySerializer::serializedSizeRecursive(const T& value, const Args&... args)
{
    SizeUnit t_size = BinarySerializer::serializedSizeSingle(value);

    if constexpr (sizeof...(args) > 0)
        t_size += BinarySerializer::serializedSizeRecursive(std::forward<const Args&>(args)...);

    return t_size;
}

template<typename T, typename... Args>
void BinarySerializer::writeRecursive(const T& value, const Args&... args)
{
    this->writeSingle(value);

    if constexpr (sizeof...(args) > 0)
        this->writeRecursive(std::forward<const Args&>(args)...);
}

template<typename T, typename... Args>
void BinarySerializer::readRecursive(T& value, Args&... args)
{
    this->readSingle(value);

    if constexpr (sizeof...(args) > 0)
        this->readRecursive(std::forward<Args&>(args)...);
}

template<typename... Args>
SizeUnit BinarySerializer::fastSerialization(BytesSmartPtr& out, const Args&... args)
{
    // Do the serialization
    BinarySerializer serializer;
    const SizeUnit size = serializer.write(std::forward<const Args&>(args)...);
    serializer.moveUnique(out);
    std::stringstream ss;
    for(size_t i = 0; i < size; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(out[i]);
        if (i < size - 1)
            ss << " ";
    }
    return size;
}

template<typename... Args>
void BinarySerializer::fastDeserialization(void* src, SizeUnit size, Args&... args)
{
    // Do the deserialization.
    BinarySerializer serializer(src, size);
    serializer.read(std::forward<Args&>(args)...);
    if(!serializer.allReaded())
        throw std::out_of_range(BinarySerializer::kClassScope + " Not all data was deserialized.");
}

template<typename... Args>
void BinarySerializer::fastDeserialization(BytesSmartPtr&& src, SizeUnit size, Args&... args)
{
    // Do the deserialization.
    BinarySerializer serializer(std::move(src), size);
    serializer.read(std::forward<Args&>(args)...);
    if(!serializer.allReaded())
        throw std::out_of_range(BinarySerializer::kClassScope + " Not all data was deserialized.");
}

// -----------------------------------------------------------------------------------------------------------------

template<typename T, typename... Args>
SizeUnit BinarySerializer::serializedSize(const T& value, const Args&... args)
{
    // Calculate the serialized size.
    return BinarySerializer::serializedSizeRecursive(value, args...);
}

template<typename T>
typename std::enable_if_t<
        !BinarySerializer::is_container<T>::value &&
        !std::is_same_v<std::nullptr_t &&, T> &&
        !std::is_pointer_v<T>, SizeUnit>
BinarySerializer::serializedSizeSingle(const T& data)
{
    if constexpr(std::is_base_of_v<Serializable, std::decay_t<T>>)
    {
        return data.serializedSize();
    }
    else if constexpr(std::is_same_v<std::decay_t<T>, std::string>)
    {
        return sizeof(SizeUnit) + data.size();
    }
    else if constexpr(std::is_trivially_copyable_v<std::decay_t<T>> && std::is_trivial_v<std::decay_t<T>>)
    {
        return sizeof(SizeUnit) + sizeof(data);
    }
    else
        static_assert(sizeof(T) == 0,
                      "[LibDegorasBase,Serialization,BinarySerializer] Unsupported type for total size calculation.");
}

template<typename T, size_t L>
SizeUnit BinarySerializer::serializedSizeSingle(const std::array<T, L>&)
{
    // Check the types.
    (BinarySerializer::checkTriviallyCopyable<T>());
    (BinarySerializer::checkTrivial<T>());
    // Get the total size.
    constexpr SizeUnit array_size = L;
    constexpr SizeUnit elem_size = sizeof(T);
    return sizeof(SizeUnit) + sizeof(SizeUnit) + elem_size * array_size;
}

template<typename T>
SizeUnit BinarySerializer::serializedSizeSingle(const std::vector<T>& data)
{
    SizeUnit total_size = 0;

    if constexpr(std::is_base_of_v<Serializable, std::decay_t<T>>)
    {
        total_size = sizeof(SizeUnit);
        for(const auto& obj : data)
            total_size += obj.serializedSize();
    }
    else
    {
        // Check the types.
        (BinarySerializer::checkTriviallyCopyable<T>());
        (BinarySerializer::checkTrivial<T>());
        // Get the total size.
        const SizeUnit vector_size = data.size();
        constexpr SizeUnit elem_size = sizeof(T);
        total_size = sizeof(SizeUnit) + sizeof(SizeUnit) + elem_size * vector_size;
    }

    return total_size;
}

template<typename T>
SizeUnit BinarySerializer::serializedSizeSingle(const std::vector<std::vector<T>>& data)
{
    SizeUnit total_size = 0;

    if constexpr(std::is_base_of_v<Serializable, std::decay_t<T>>)
    {
        total_size = sizeof(SizeUnit);
        for(const auto& vec : data)
            total_size += BinarySerializer::serializedSizeSingle(vec);
    }
    else
    {
        // Check the types.
        (BinarySerializer::checkTriviallyCopyable<T>());
        (BinarySerializer::checkTrivial<T>());

        // Get the total size.
        constexpr SizeUnit elem_size = sizeof(T);

        // Size of vector size storage and element size storage.
        total_size += sizeof(SizeUnit);
        total_size += sizeof(SizeUnit);

        // Size of each subvector.
        for(const auto& sub_vector : data)
        {
            const SizeUnit subvector_size = sub_vector.size();
            total_size += sizeof(SizeUnit);
            total_size += elem_size * subvector_size;
        }
    }

    // Return the total size.
    return total_size;
}

template<typename T, typename... Args, typename>
SizeUnit BinarySerializer::write(const T& value, const Args&... args)
{
    // Calculate total size of all arguments.
    const SizeUnit t_size = BinarySerializer::serializedSizeRecursive(value, args...);

    // Reserve space in one go.
    this->reserve(this->size_ + t_size);

    // Forward to recursive write function
    this->writeRecursive(value, args...);

    // Return the writed size.
    return t_size;
}

template<typename... Args>
SizeUnit BinarySerializer::write(const std::tuple<Args...>& tup)
{
    // Calculate the total size of all tuple elements
    const SizeUnit t_size = std::apply([&](const auto&... args)
    {
        return (BinarySerializer::serializedSizeSingle(args) + ...);
    }, tup);

    // Reserve space in one go
    this->reserve(this->size_ + t_size);

    // Serialize each element in the tuple
    std::apply([&](const auto&... args)
    {
        (this->writeSingle(args), ...);
    }, tup);

    // Return the total written size
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
        !std::is_same_v<std::filesystem::path, T> &&
        !std::is_pointer_v<T>, void>
BinarySerializer::writeSingle(const T& data)
{
    // Check the types.
    (BinarySerializer::checkTriviallyCopyable<T>());
    (BinarySerializer::checkTrivial<T>());

    // Get the size of the data.
    constexpr SizeUnit data_size = sizeof(T);

    // Safety mutex.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Serialize the size of the data.
    BinarySerializer::binarySerialize(&data_size, sizeof(SizeUnit), this->data_.get() + this->size_);
    this->size_ += sizeof(SizeUnit);

    // Serialize the data.
    BinarySerializer::binarySerialize(&data, data_size, this->data_.get() + this->size_);
    this->size_ += data_size;
}

template<typename T, SizeUnit L>
void BinarySerializer::writeSingle(const std::array<T, L>& arr)
{
    // Check the types.
    BinarySerializer::checkTriviallyCopyable<T>();
    BinarySerializer::checkTrivial<T>();

    // Get the size.
    constexpr SizeUnit array_size = L;
    constexpr SizeUnit elem_size = sizeof(T);

    // Safety mutex.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Serialize array size.
    BinarySerializer::binarySerialize(&array_size, sizeof(SizeUnit), this->data_.get() + size_);
    this->size_ += sizeof(SizeUnit);

    // Serialize the size of each element.
    BinarySerializer::binarySerialize(&elem_size, sizeof(SizeUnit), this->data_.get() + size_);
    this->size_ += sizeof(SizeUnit);

    // Write each value of the array.
    for(const auto& val : arr)
    {
        BinarySerializer::binarySerialize(&val, elem_size, this->data_.get() + this->size_);
        this->size_ += elem_size;
    }
}

template<typename T>
void BinarySerializer::writeSingle(const std::vector<T>& v)
{
    if constexpr(std::is_base_of_v<Serializable, std::decay_t<T>>)
    {
        // Get the size.
        const SizeUnit vector_size = v.size();

        // Safety mutex block.
        {
            std::lock_guard<std::mutex> lock(this->mtx_);

            // Serialize vector size.
            BinarySerializer::binarySerialize(&vector_size, sizeof(SizeUnit), this->data_.get() + size_);
            this->size_ += sizeof(SizeUnit);
        }

        // Write each value of the vector.
        for(const auto& val : v)
            val.serialize(*this);
    }
    else
    {
        // Check the types.
        BinarySerializer::checkTriviallyCopyable<T>();
        BinarySerializer::checkTrivial<T>();

        // Get the size.
        const SizeUnit vector_size = v.size();
        constexpr SizeUnit elem_size = sizeof(T);

        // Safety mutex.
        std::lock_guard<std::mutex> lock(this->mtx_);

        // Serialize vector size.
        BinarySerializer::binarySerialize(&vector_size, sizeof(SizeUnit), this->data_.get() + size_);
        this->size_ += sizeof(SizeUnit);

        // Serialize the size of each element.
        BinarySerializer::binarySerialize(&elem_size, sizeof(SizeUnit), this->data_.get() + size_);
        this->size_ += sizeof(SizeUnit);

        // Write each value of the vector.
        for(const auto& val : v)
        {
            BinarySerializer::binarySerialize(&val, elem_size, this->data_.get() + this->size_);
            this->size_ += elem_size;
        }
    }
}

template<typename T>
void BinarySerializer::writeSingle(const std::vector<std::vector<T>>& v)
{
    if constexpr(std::is_base_of_v<Serializable, std::decay_t<T>>)
    {
        // Get the size.
        const SizeUnit vector_size = v.size();

        // Safety mutex block.
        {
            std::lock_guard<std::mutex> lock(this->mtx_);

            // Serialize vector size.
            BinarySerializer::binarySerialize(&vector_size, sizeof(SizeUnit), this->data_.get() + size_);
            this->size_ += sizeof(SizeUnit);
        }

        // Write each value of the vector.
        for(const auto& vec : v)
            this->writeSingle(vec);
    }

    else
    {
        // Check the types.
        BinarySerializer::checkTriviallyCopyable<T>();
        BinarySerializer::checkTrivial<T>();

        // Get the size.
        const SizeUnit vector_size = v.size();
        constexpr SizeUnit elem_size = sizeof(T);

        // Safety mutex.
        std::lock_guard<std::mutex> lock(this->mtx_);

        // Serialize vector size.
        BinarySerializer::binarySerialize(&vector_size, sizeof(SizeUnit), this->data_.get() + size_);
        this->size_ += sizeof(SizeUnit);

        // Serialize the size of each element.
        BinarySerializer::binarySerialize(&elem_size, sizeof(SizeUnit), this->data_.get() + size_);
        this->size_ += sizeof(SizeUnit);

        // Write each vector.
        for(const auto& sub_vector : v)
        {
            // Serialize vector size.
            const SizeUnit sub_vector_size = sub_vector.size();
            BinarySerializer::binarySerialize(&sub_vector_size, sizeof(SizeUnit), this->data_.get() + size_);
            this->size_ += sizeof(SizeUnit);

            // Write each value of the vector.
            for(const auto& val : sub_vector)
            {
                BinarySerializer::binarySerialize(&val, elem_size, this->data_.get() + this->size_);
                this->size_ += elem_size;
            }
        }
    }
}

template<typename T>
typename std::enable_if_t<
    !BinarySerializer::is_container<T>::value &&
    !std::is_base_of_v<Serializable, T> &&
    !std::is_same_v<std::nullptr_t &&, T> &&
    !std::is_same_v<std::filesystem::path, T> &&
    !std::is_pointer_v<T>, void>
BinarySerializer::readSingle(T& value)
{
    // Check the types.
    (BinarySerializer::checkTriviallyCopyable<T>());
    (BinarySerializer::checkTrivial<T>());

    // Safety mutex.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Ensure that there's enough data left to read the size of the value.
    if (this->offset_ + sizeof(SizeUnit) > this->size_)
        throw std::out_of_range(BinarySerializer::kClassScope + " Not enough data left to read the size of the value.");

    // Read the size of the value.
    SizeUnit size;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &size);

    // Update the offset.
    this->offset_ += sizeof(SizeUnit);

    // Check if we have enough data left to read the value.
    if (this->offset_ + size > this->size_)
        throw std::out_of_range(BinarySerializer::kClassScope + " Read value beyond the data size.");

    // Check if the size is greater than the expected size for the type.
    if (size > sizeof(T))
        throw std::logic_error(BinarySerializer::kClassScope + " The serialized value size is greater than type for storage.");

    // Read the value.
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, size, &value);
    this->offset_ += size;
}

template<typename T, SizeUnit L>
void BinarySerializer::readSingle(std::array<T, L>& arr)
{
    // Safety mutex.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Ensure that there's enough data left to read the size of the array.
    if (this->offset_ + sizeof(SizeUnit) > this->size_)
        throw std::out_of_range(
            BinarySerializer::kClassScope + " Not enough data left to read the size of the array.");

    // Read the size of the array.
    SizeUnit size_array;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &size_array);

    // Update the offset.
    this->offset_ += sizeof(SizeUnit);

    // Check if the array is empty.
    if(size_array == 0)
        return;

    // Ensure that there's enough data left to read the size of each elements.
    if (this->offset_ + sizeof(SizeUnit) > this->size_)
        throw std::out_of_range(
            BinarySerializer::kClassScope + " Not enough data left to read the size of elements of the array.");

    // Read the size of the elements.
    SizeUnit size_elem;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &size_elem);

    // Update the offset.
    this->offset_ += sizeof(SizeUnit);

    // Check if the size is 0.
    if(size_elem == 0)
        throw std::out_of_range("BinarySerializer: Unknow size of elements of the array.");

    // Check if we have enough data left to read the array.
    if (this->offset_ + size_elem*size_array > this->size_)
        throw std::out_of_range("BinarySerializer: Read array data beyond the data size.");

    // Read all the elements.
    for(std::uint64_t i = 0; i < size_array; i++)
    {
        BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, size_elem, &arr[i]);
        this->offset_ += size_elem;
    }
}

template<typename T>
void BinarySerializer::readSingle(std::vector<T>& v)
{
    SizeUnit size_vector;
    {
        // Safety mutex.
        std::lock_guard<std::mutex> lock(this->mtx_);

        // Ensure that there's enough data left to read the size of the vector.
        if (this->offset_ + sizeof(SizeUnit) > this->size_)
            throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the vector.");

        // Read the size of the array.
        BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &size_vector);

        // Update the offset.
        this->offset_ += sizeof(SizeUnit);
    }

    // Check if the vector is empty.
    if(size_vector == 0)
        return;

    if constexpr(std::is_base_of_v<Serializable, std::decay_t<T>>)
    {
        // Prepare the vector.
        v.clear();
        v.resize(size_vector);

        for (SizeUnit i = 0; i < size_vector; i++)
        {
            this->readSingle(v[i]);
        }
    }

    else
    {
        // Ensure that there's enough data left to read the size of each elements.
        if (this->offset_ + sizeof(SizeUnit) > this->size_)
            throw std::out_of_range("BinarySerializer: Not enough data left to read the size of elements of the vector.");

        // Read the size of the elements.
        SizeUnit size_elem;
        BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &size_elem);

        // Update the offset.
        this->offset_ += sizeof(SizeUnit);

        // Check if the size is 0.
        if(size_elem == 0)
            throw std::out_of_range("BinarySerializer: Unknow size of elements of the vector.");

        // Check if we have enough data left to read the data.
        if (this->offset_ + size_elem*size_vector > this->size_)
            throw std::out_of_range("BinarySerializer: Read vector data beyond the data size.");

        // Prepare the vector.
        v.clear();
        v.resize(size_vector);

        // Read all the elements.
        for(std::uint64_t i = 0; i < size_vector; i++)
        {
            BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, size_elem, &v[i]);
            this->offset_ += size_elem;
        }
    }
}

template<typename T>
void BinarySerializer::readSingle(std::vector<std::vector<T>>& v)
{
    SizeUnit size_vector;

    {
        // Safety mutex.
        std::lock_guard<std::mutex> lock(this->mtx_);

        // Ensure that there's enough data left to read the size of the vector.
        if (this->offset_ + sizeof(SizeUnit) > this->size_)
            throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the vector.");

        // Read the size of the vector.
        BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &size_vector);

        // Update the offset.
        this->offset_ += sizeof(SizeUnit);
    }

    // Check if the vector is empty.
    if(size_vector == 0)
        return;

    if constexpr(std::is_base_of_v<Serializable, std::decay_t<T>>)
    {
        // Prepare the vector.
        v.clear();
        v.resize(size_vector);

        for (SizeUnit i = 0; i < size_vector; i++)
        {
            this->readSingle(v[i]);
        }
    }

    else
    {

        // Ensure that there's enough data left to read the size of each elements.
        if (this->offset_ + sizeof(SizeUnit) > this->size_)
            throw std::out_of_range("BinarySerializer: Not enough data left to read the size of elements of the vector.");

        // Read the size of the elements.
        SizeUnit size_elem;
        BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &size_elem);

        // Update the offset.
        this->offset_ += sizeof(SizeUnit);

        // Check if the size is 0.
        if(size_elem == 0)
            throw std::out_of_range("BinarySerializer: Unknow size of elements of the vector.");

        // Prepare the vector.
        v.clear();
        v.resize(size_vector);

        // Read all the subvectors.
        for(std::uint64_t i = 0; i < size_vector; i++)
        {
            // Ensure that there's enough data left to read the size of the subvector.
            if (this->offset_ + sizeof(SizeUnit) > this->size_)
                throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the subvector.");

            // Read the size of the subvector.
            SizeUnit size_subvector;
            BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &size_subvector);

            // Update the offset.
            this->offset_ += sizeof(SizeUnit);

            // Check if we have enough data left to read the data.
            if (this->offset_ + size_elem*size_subvector > this->size_)
                throw std::out_of_range("BinarySerializer: Read subvector data beyond the data size.");

            // Prepare the subvector.
            std::vector<T> subv;
            subv.resize(size_subvector);

            // Read all the elements of the subvector.
            for(std::uint64_t j = 0; j < size_subvector; j++)
            {
                BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, size_elem, &subv[j]);
                this->offset_ += size_elem;
            }

            // Store the subvector.
            v[i] = subv;
        }
    }
}

template<typename... Args>
void BinarySerializer::readSingle(std::tuple<Args...>& tup)
{
    readTupleElements(tup);
}

template<std::size_t I, typename... Tp>
typename std::enable_if<I == sizeof...(Tp), void>::type
BinarySerializer::readTupleElements(std::tuple<Tp...>&){}

template<std::size_t I, typename... Tp>
typename std::enable_if<I < sizeof...(Tp), void>::type
BinarySerializer::readTupleElements(std::tuple<Tp...>& t)
{
    this->readSingle(std::get<I>(t));
    this->readTupleElements<I + 1, Tp...>(t);
}

template<typename T, typename... Args>
SizeUnit Serializable::calcSizeHelper(const T& value, const Args&... args)
{
    return BinarySerializer::serializedSize(value, args...);
}

}} // END NAMESPACES.
// =====================================================================================================================
