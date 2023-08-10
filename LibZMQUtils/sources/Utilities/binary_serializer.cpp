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
 * @file binary_serializer.cpp
 * @brief This file contains part of the implementation of the BinarySerializer class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2308.2
***********************************************************************************************************************/

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

Serializable::~Serializable(){}

BinarySerializer::BinarySerializer(size_t capacity) :
        data_(new std::byte[capacity]),
        size_(0),
        capacity_(capacity),
        offset_(0),
        endianess_(this->determineEndianess())
{}

BinarySerializer::BinarySerializer(void *src, size_t size) :
    data_(nullptr),
    size_(0),
    capacity_(0),
    offset_(0),
    endianess_(this->determineEndianess())
{
    // Load the test.
    this->loadData(src, size);
}

void BinarySerializer::reserve(size_t size)
{
    if (size > this->capacity_)
    {
        std::lock_guard<std::recursive_mutex> lock(this->mtx_);
        std::unique_ptr<std::byte[]> new_data(new std::byte[size]);
        if (this->data_)
            std::memcpy(new_data.get(), data_.get(), size_);
        this->data_ = std::move(new_data);
        this->capacity_ = size;
    }
}

void BinarySerializer::loadData(void* src, size_t size)
{
    if(src != nullptr)
    {
        this->reserve(size);
        std::lock_guard<std::recursive_mutex> lock(this->mtx_);
        std::memcpy(this->data_.get(), src, size);
        this->size_ = size;
        this->offset_ = 0;
    }
}

void BinarySerializer::clearData()
{
    std::lock_guard<std::recursive_mutex> lock(this->mtx_);
    this->data_.reset(nullptr);
    this->size_ = 0;
    this->capacity_ = 0;
    this->offset_ = 0;
}

void BinarySerializer::resetReading()
{
    this->offset_ = 0;
}

std::unique_ptr<std::byte> BinarySerializer::moveUnique()
{
    std::lock_guard<std::recursive_mutex> lock(this->mtx_);
    this->size_ = 0;
    this->capacity_ = 0;
    this->offset_ = 0;
    return std::unique_ptr<std::byte>(this->data_.release());
}

std::unique_ptr<std::byte> BinarySerializer::moveUnique(size_t& size)
{
    size = this->size_;
    return this->moveUnique();
}

std::byte* BinarySerializer::release()
{
    std::lock_guard<std::recursive_mutex> lock(this->mtx_);
    this->size_ = 0;
    this->capacity_ = 0;
    this->offset_ = 0;
    return this->data_.release();
}

std::byte *BinarySerializer::release(size_t& size)
{
    std::lock_guard<std::recursive_mutex> lock(this->mtx_);
    size = this->size_;
    return this->release();
}

size_t BinarySerializer::getSize() const
{
    return this->size_;
}

bool BinarySerializer::allReaded() const
{
    return this->offset_ == this->size_;
}

std::string BinarySerializer::getDataHexString() const
{
    std::lock_guard<std::recursive_mutex> lock(this->mtx_);
    std::stringstream ss;
    for(size_t i = 0; i < this->size_; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(this->data_.get()[i]);
        if (i < this->size_ - 1)
            ss << " ";
    }
    return ss.str();
}

BinarySerializer::Endianess BinarySerializer::determineEndianess()
{
    uint16_t number = 1;
    uint8_t* byte_ptr = reinterpret_cast<uint8_t*>(&number);
    return (byte_ptr[0] == 1) ? Endianess::LITTLE_ENDIAN : Endianess::BIG_ENDIAN;
}

std::string BinarySerializer::toJsonString() const
{
    std::lock_guard<std::recursive_mutex> lock(this->mtx_);
    std::stringstream ss;
    ss << "{"
       << "\"size\": " << this->size_ << ", "
       << "\"capacity\": " << this->capacity_ << ", "
       << "\"offset\": " << this->offset_ << ", "
       << "\"hexadecimal\": \"" << this->getDataHexString() << "\""
       << "}";
    return ss.str();
}

size_t BinarySerializer::writeSingle(const Serializable &obj)
{
    // Lock guard.
    std::lock_guard<std::recursive_mutex> lock(this->mtx_);
    // Serialize the object.
    return obj.serialize(*this);
}

size_t BinarySerializer::writeSingle(const std::string& str)
{
    // String size.
    uint64_t str_size = str.size();

    // Get the size of the data.
    uint64_t total_size = BinarySerializer::calcTotalSize(str);

    // Reserve space.
    this->reserve(this->size_ + total_size);

    // Lock guard.
    std::lock_guard<std::recursive_mutex> lock(this->mtx_);

    // Serialize size.
    BinarySerializer::binarySerialize(&str_size, sizeof(uint64_t), this->data_.get() + size_);
    this->size_ += sizeof(uint64_t);

    // Serialize string.
    BinarySerializer::binarySerialize(str.data(), str_size, this->data_.get() + size_);
    this->size_ += str_size;

    // Return the writed size.
    return total_size;
}

void BinarySerializer::readSingle(Serializable &obj)
{
    // Mutex.
    std::lock_guard<std::recursive_mutex> lock(this->mtx_);

    // Ensure that there's enough data left to read the object.
    if (this->offset_ + obj.serializedSize() > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the Serializable object.");

    // Deserialize.
    obj.deserialize(*this);
}

void BinarySerializer::readSingle(std::string &str)
{
    // Mutex.
    std::lock_guard<std::recursive_mutex> lock(this->mtx_);

    // Ensure that there's enough data left to read the size of the string.
    if (this->offset_ + sizeof(size_t) > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the string.");

    // Read the size of the string.
    uint64_t size;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(uint64_t), &size);

    // Update the offset.
    this->offset_ += sizeof(uint64_t);

    // Check if the string is empty.
    if(size == 0)
        return;

    // Check if we have enough data left to read the string.
    if (this->offset_ + size > this->size_)
        throw std::out_of_range("BinarySerializer: Read string beyond the data size.");

    // Read the string.
    str.resize(size);
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, size, str.data());
    this->offset_ += size;
}

}} // END NAMESPACES.
// =====================================================================================================================
