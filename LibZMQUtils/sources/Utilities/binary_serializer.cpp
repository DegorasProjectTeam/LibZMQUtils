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
 * @version 2307.1
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

BinarySerializer::BinarySerializer(size_t capacity) :
        data_(new std::uint8_t[capacity]),
        size_(0),
        capacity_(capacity),
        offset_(0){}

BinarySerializer::BinarySerializer(void *data, size_t size)
        : data_(nullptr), size_(0), capacity_(0), offset_(0)
{
    this->loadData(data, size);
}

void BinarySerializer::reserve(size_t size)
{
    if (size > this->capacity_)
    {
        std::lock_guard<std::mutex> lock(this->mtx_);
        std::unique_ptr<std::uint8_t[]> new_data(new std::uint8_t[size]);
        if (this->data_)
            std::memcpy(new_data.get(), data_.get(), size_);
        this->data_ = std::move(new_data);
        this->capacity_ = size;
    }
}

void BinarySerializer::loadData(void* data, size_t size)
{
    if(data != nullptr)
    {
        this->reserve(size);
        std::lock_guard<std::mutex> lock(this->mtx_);
        std::memcpy(this->data_.get(), data, size);
        this->size_ = size;
        this->offset_ = 0;
    }
}

void BinarySerializer::clearData()
{
    std::lock_guard<std::mutex> lock(this->mtx_);
    this->data_.reset(nullptr);
    this->size_ = 0;
    this->capacity_ = 0;
    this->offset_ = 0;
}

void BinarySerializer::resetReading()
{
    this->offset_ = 0;
}

std::unique_ptr<std::uint8_t> BinarySerializer::moveUnique()
{
    std::lock_guard<std::mutex> lock(this->mtx_);
    this->size_ = 0;
    this->capacity_ = 0;
    this->offset_ = 0;
    return std::unique_ptr<std::uint8_t>(this->data_.release());
}

std::unique_ptr<std::uint8_t> BinarySerializer::moveUnique(size_t& size)
{
    size = this->size_;
    return this->moveUnique();
}


std::uint8_t* BinarySerializer::release()
{
    std::lock_guard<std::mutex> lock(this->mtx_);
    this->size_ = 0;
    this->capacity_ = 0;
    this->offset_ = 0;
    return this->data_.release();
}

std::uint8_t* BinarySerializer::release(size_t& size)
{
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
    std::lock_guard<std::mutex> lock(this->mtx_);
    std::stringstream ss;
    for(size_t i = 0; i < size_; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(data_.get()[i]) << " ";
    return ss.str();
}

std::string BinarySerializer::toString() const
{
    std::stringstream ss;
    ss << "{size: " << size_
       << ", capacity: " << capacity_
       << ", offset: " << offset_
       << ", hex_data: " << this->getDataHexString() << "}";
    return ss.str();
}

void BinarySerializer::binarySerializeDeserialize(const void *data, size_t data_size_bytes, void *dest)
{
    const std::uint8_t* data_byes = reinterpret_cast<const std::uint8_t *>(data);
    std::uint8_t* dest_byes = reinterpret_cast<std::uint8_t*>(dest);
    std::reverse_copy(data_byes, data_byes + data_size_bytes, dest_byes);
}


}} // END NAMESPACES.
// =====================================================================================================================
