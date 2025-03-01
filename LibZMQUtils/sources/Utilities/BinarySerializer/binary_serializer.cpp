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
 * @file binary_serializer.cpp
 * @brief This file contains part of the implementation of the BinarySerializer class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <fstream>
#include <istream>
#include <sstream>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.h"
#include "LibZMQUtils/InternalHelpers/file_helpers.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace serializer{
// =====================================================================================================================

Serializable::~Serializable(){}

BinarySerializer::BinarySerializer(SizeUnit capacity) :
        data_(new std::byte[capacity]),
        size_(0),
        capacity_(capacity),
        offset_(0),
        endianess_(this->determineEndianess())
{}

BinarySerializer::BinarySerializer(void *src, SizeUnit size) :
    data_(nullptr),
    size_(0),
    capacity_(0),
    offset_(0),
    endianess_(this->determineEndianess())
{
    // Load the test.
    this->loadData(src, size);
}

BinarySerializer::BinarySerializer(BytesDataPtr&& src, SizeUnit size) :
    data_(std::move(src)),
    size_(size),
    capacity_(size),
    offset_(0),
    endianess_(this->determineEndianess())
{}

void BinarySerializer::reserve(SizeUnit size)
{
    if (size > this->capacity_)
    {
        std::lock_guard<std::mutex> lock(this->mtx_);
        BytesDataPtr new_data(new std::byte[size]);
        if (this->data_)
            std::memcpy(new_data.get(), data_.get(), size_);
        this->data_ = std::move(new_data);
        this->capacity_ = size;
    }
}

void BinarySerializer::loadData(void* src, SizeUnit size)
{
    if(src != nullptr)
    {
        this->reserve(size);
        std::lock_guard<std::mutex> lock(this->mtx_);
        std::memcpy(this->data_.get(), src, size);
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

SizeUnit BinarySerializer::moveUnique(BytesDataPtr &out)
{
    std::lock_guard<std::mutex> lock(this->mtx_);
    SizeUnit size = this->size_;
    out = std::move(this->data_);
    this->size_ = 0;
    this->capacity_ = 0;
    this->offset_ = 0;
    return size;
}

std::byte* BinarySerializer::release()
{
    std::lock_guard<std::mutex> lock(this->mtx_);
    this->size_ = 0;
    this->capacity_ = 0;
    this->offset_ = 0;
    return this->data_.release();
}

std::byte *BinarySerializer::release(SizeUnit& size)
{
    std::lock_guard<std::mutex> lock(this->mtx_);
    size = this->size_;
    this->size_ = 0;
    this->capacity_ = 0;
    this->offset_ = 0;
    return this->data_.release();
}

SizeUnit BinarySerializer::getSize() const
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
    for(size_t i = 0; i < this->size_; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(this->data_[i]);
        if (i < this->size_ - 1)
            ss << " ";
    }
    return ss.str();
}

SizeUnit BinarySerializer::writeFile(const std::string &in_filenamepath)
{
    // Get the filename.
    std::string filename = internal_helpers::files::getFileName(in_filenamepath);

    // Create the ifstream.
    std::ifstream file(in_filenamepath, std::ios::in | std::ios::binary);

    // Check if the file is open.
    if(!file.is_open())
        throw std::runtime_error("BinarySerializer: File for serialization can't be opened.");

    // Get the size of the file.
    file.seekg(0, std::ios::end);
    const SizeUnit file_size = static_cast<SizeUnit>(file.tellg());
    file.seekg(0, std::ios::beg);

    // Get the size of the filename.
    SizeUnit filename_size = filename.size();

    // Get the total size.
    const SizeUnit total_size = sizeof(SizeUnit) + filename_size + sizeof(SizeUnit) + file_size;

    // Reserve space.
    this->reserve(this->size_ + total_size);

    // Lock guard.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Serialize name size.
    BinarySerializer::binarySerialize(&filename_size, sizeof(SizeUnit), this->data_.get() + size_);
    this->size_ += sizeof(SizeUnit);

    // Serialize name string.
    BinarySerializer::binarySerialize(filename.data(), filename_size, this->data_.get() + size_);
    this->size_ += filename_size;

    // Serialize file size.
    BinarySerializer::binarySerialize(&file_size, sizeof(SizeUnit), this->data_.get() + size_);
    this->size_ += sizeof(SizeUnit);

    // Serialize the file.
    file.read(reinterpret_cast<char*>(this->data_.get() + this->size_), static_cast<long long>(file_size));
    this->size_ += file_size;

    // Close the file.
    file.close();

    // Return the written size.
    return total_size;
}

std::string BinarySerializer::readFile(const std::string& out_path)
{
    // Mutex.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Ensure that there's enough data left to read the size of the filename.
    if (this->offset_ + sizeof(SizeUnit) > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the filename.");

    // Read the size of the filename.
    std::uint64_t filename_size;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &filename_size);

    // Update the offset.
    this->offset_ += sizeof(SizeUnit);

    // Check if the filename is empty.
    if(filename_size == 0)
        throw std::runtime_error("BinarySerializer: Empty filename.");

    // Ensure that there's enough data left to read the filename.
    if (this->offset_ + filename_size > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the filename.");

    // Read the filename.
    std::string filename;
    filename.resize(filename_size);
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, filename_size, filename.data());
    this->offset_ += filename_size;

    // Ensure that there's enough data left to read the size of the file.
    if (this->offset_ + sizeof(SizeUnit) > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the file.");

    // Read the size of the file content.
    std::uint64_t file_size;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &file_size);

    // Update the offset.
    this->offset_ += sizeof(SizeUnit);

    // Check if the file is empty.
    if(file_size == 0)
        return {};

    // Ensure that there's enough data left to read the file content.
    if (this->offset_ + file_size > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the file content.");

    // Prepare the stream.
    std::string final_path = out_path.empty() ? filename : (out_path + "/" + filename);
    std::ofstream file_output(final_path, std::ios::binary);
    if (!file_output.is_open())
        throw std::runtime_error("BinarySerializer: File for deserialization can't be opened.");

    // Read the file content.
    file_output.write(reinterpret_cast<const char*>(this->data_.get() + this->offset_),
                      static_cast<long long>(file_size));
    this->offset_ += file_size;

    // Close the file.
    file_output.close();

    return final_path;
}

BinarySerializer::Endianess BinarySerializer::determineEndianess()
{
    constexpr uint16_t number = 1;
    const uint8_t* byte_ptr = reinterpret_cast<const uint8_t*>(&number);
    return (byte_ptr[0] == 1) ? Endianess::LT_ENDIAN : Endianess::BG_ENDIAN;
}

std::string BinarySerializer::toJsonString() const
{
    std::stringstream ss;
    ss << "{"
       << "\"size\": " << this->size_ << ", "
       << "\"capacity\": " << this->capacity_ << ", "
       << "\"offset\": " << this->offset_ << ", "
       << "\"hexadecimal\": \"" << this->getDataHexString() << "\""
       << "}";
    return ss.str();
}

void BinarySerializer::writeSingle(const Serializable &obj)
{
    // Serialize the object.
    obj.serialize(*this);
}

void BinarySerializer::writeSingle(const std::string& str)
{
    // String size.
    SizeUnit str_size = str.size();

    // Lock guard.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Serialize size.
    BinarySerializer::binarySerialize(&str_size, sizeof(SizeUnit), this->data_.get() + size_);
    this->size_ += sizeof(SizeUnit);

    // Serialize string.
    BinarySerializer::binarySerialize(str.data(), str_size, this->data_.get() + size_);
    this->size_ += str_size;
}

#if __MINGW64_VERSION_MAJOR > 6
void BinarySerializer::writeSingle(const std::filesystem::path &file_path)
{
    // Get the filename.
    std::string filename = internal_helpers::files::getFileName(file_path.string());

    // Create the ifstream.
    std::ifstream file(file_path, std::ios::in | std::ios::binary);

    // Check if the file is open.
    if(!file.is_open())
        throw std::runtime_error("BinarySerializer: File for serialization can't be opened.");

    // Get the size of the file.
    const SizeUnit file_size = std::filesystem::file_size(file_path);

    // Get the size of the filename.
    SizeUnit filename_size = filename.size();

    // Lock guard.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Serialize name size.
    BinarySerializer::binarySerialize(&filename_size, sizeof(SizeUnit), this->data_.get() + size_);
    this->size_ += sizeof(SizeUnit);

    // Serialize name string.
    BinarySerializer::binarySerialize(filename.data(), filename_size, this->data_.get() + size_);
    this->size_ += filename_size;

    // Serialize file size.
    BinarySerializer::binarySerialize(&file_size, sizeof(SizeUnit), this->data_.get() + size_);
    this->size_ += sizeof(SizeUnit);

    // Serialize the file.
    file.read(reinterpret_cast<char*>(this->data_.get() + this->size_), static_cast<long long>(file_size));
    this->size_ += file_size;

    // Close the file.
    file.close();
}
#endif

void BinarySerializer::readSingle(Serializable &obj)
{
    // Ensure that there's enough data left to read the object.
    if (this->offset_ + obj.serializedSize() > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the Serializable object.");

    // Deserialize.
    obj.deserialize(*this);
}

void BinarySerializer::readSingle(std::string &str)
{
    // Mutex.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Ensure that there's enough data left to read the size of the string.
    if (this->offset_ + sizeof(SizeUnit) > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the string.");

    // Read the size of the string.
    SizeUnit size;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &size);

    // Update the offset.
    this->offset_ += sizeof(SizeUnit);

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

#if __MINGW64_VERSION_MAJOR > 6
void BinarySerializer::readSingle(std::filesystem::path& out_filepath)
{
    // Mutex.
    std::lock_guard<std::mutex> lock(this->mtx_);

    // Ensure that there's enough data left to read the size of the filename.
    if (this->offset_ + sizeof(SizeUnit) > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the filename.");

    // Read the size of the filename.
    SizeUnit filename_size;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &filename_size);

    // Update the offset.
    this->offset_ += sizeof(SizeUnit);

    // Check if the filename is empty.
    if(filename_size == 0)
        throw std::runtime_error("BinarySerializer: Empty filename.");

    // Ensure that there's enough data left to read the filename.
    if (this->offset_ + filename_size > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the filename.");

    // Read the filename.
    std::string filename;
    filename.resize(filename_size);
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, filename_size, filename.data());
    this->offset_ += filename_size;

    // Ensure that there's enough data left to read the size of the file.
    if (this->offset_ + sizeof(SizeUnit) > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the file.");

    // Read the size of the file content.
    std::uint64_t file_size;
    BinarySerializer::binaryDeserialize(this->data_.get() + this->offset_, sizeof(SizeUnit), &file_size);

    // Update the offset.
    this->offset_ += sizeof(SizeUnit);

    // Check if the file is empty.
    if(file_size == 0)
        return;

    // Ensure that there's enough data left to read the file content.
    if (this->offset_ + file_size > this->size_)
        throw std::out_of_range("BinarySerializer: Not enough data left to read the file content.");

    // Prepare the directory.
    if(!out_filepath.empty())
    {
        try
        {
            std::filesystem::create_directories(out_filepath);
        }
        catch(...)
        {
            throw std::runtime_error("BinarySerializer: Can't create the ouput path for store the file.");
        }
    }

    // Prepare the stream.
    std::filesystem::path final_path = out_filepath.append(filename);
    std::ofstream file_output(final_path.string(), std::ios::out | std::ios::binary);
    if (!file_output.is_open())
        throw std::runtime_error("BinarySerializer: File for deserialization can't be opened.");

    // Read the file content.
    file_output.write(reinterpret_cast<const char*>(this->data_.get() + this->offset_),
                      static_cast<long long>(file_size));
    this->offset_ += file_size;

    // Close the file.
    file_output.close();

    // Update the filepath.
    out_filepath = final_path;
}


SizeUnit BinarySerializer::serializedSizeSingle(const std::filesystem::path& data)
{
    // Get the filename.
    std::string filename = internal_helpers::files::getFileName(data.string());

    // Create the ifstream.
    std::ifstream file(data, std::ios::in | std::ios::binary);

    // Check if the file is open.
    if(!file.is_open())
        throw std::runtime_error("BinarySerializer: File for serialization can't be opened.");

    // Get the size of the file.
    file.seekg(0, std::ios::end);
    const SizeUnit file_size = static_cast<SizeUnit>(file.tellg());
    file.seekg(0, std::ios::beg);

    // Get the size of the filename.
    SizeUnit filename_size = filename.size();

    // Get the total size.
    return sizeof(SizeUnit) + filename_size + sizeof(SizeUnit) + file_size;
}
#endif

BinarySerializedData::BinarySerializedData() :
    bytes(nullptr),
    size(0)
{}

bool BinarySerializedData::isEmpty() const
{
    return (this->size == 0 || !this->bytes);
}

void BinarySerializedData::clear()
{
    this->bytes.reset();
    this->size = 0;
}

zmqutils::serializer::BinarySerializedData::~BinarySerializedData()
{

}

void del_byte_ptr(void *data, void *)
{
    delete reinterpret_cast<std::byte*>(data);
}

}} // END NAMESPACES.
// =====================================================================================================================
