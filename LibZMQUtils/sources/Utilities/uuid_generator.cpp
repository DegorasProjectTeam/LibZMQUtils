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
 * @file uuid_generator.cpp
 * @brief This file contains the implementation of the UUIDGenerator class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// C++ INCLUDES
// =====================================================================================================================
#include <cstddef>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <iomanip>
#include <array>
#include <memory>
#include <set>
#include <chrono>
#include <thread>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Utilities/uuid_generator.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{
// =====================================================================================================================

UUIDGenerator::UUIDGenerator() :
    rd_(std::random_device()),
    gen_(std::mt19937_64{std::random_device{}()})
{
    // Safe mutex lock.
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Check the entropy.
    if(this->rd_.entropy() == 0.0)
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto now_int = std::chrono::time_point_cast<std::chrono::nanoseconds>(now).time_since_epoch().count();
        std::uint_fast64_t seed = std::hash<decltype(now_int)>{}(now_int);
        this->gen_ = std::mt19937_64(seed);
    }
    else
        this->gen_ = std::mt19937_64(this->rd_());
}

UUIDGenerator &UUIDGenerator::getInstance()
{
    // Guaranteed to be destroyed, instantiated on first use.
    static UUIDGenerator instance;
    return instance;
}

UUID UUIDGenerator::generateUUIDv4()
{
    // Auxiliar containers.
    std::array<std::byte, 16> bytes;
    std::uniform_int_distribution<> distrib(0, 255);
    UUID uuid;

    // Generate the uuid.
    do
    {
        // Random generation.
        for(auto& byte : bytes)
            byte = static_cast<std::byte>(distrib(this->gen_));

        // Set the version to 4 (random)
        bytes[6] = static_cast<std::byte>((static_cast<std::uint8_t>(bytes[6]) & 0x0F) | 0x40);

        // Set the variant to 1 (RFC4122)
        bytes[8] = static_cast<std::byte>((static_cast<std::uint8_t>(bytes[8]) & 0x3F) | 0x80);

        // Generate the UUID.
        uuid = UUID(bytes);

    } while(generated_uuids_.find(uuid) != generated_uuids_.end());

    // Safe mutex lock.
    std::unique_lock<std::mutex> lock(this->mtx_);

    // Insert the generated uuid.
    generated_uuids_.insert(uuid);

    // Return the generated uuid.
    return uuid;
}

UUID::UUID()
{
    this->clear();
}

UUID::UUID(const std::array<std::byte, 16> &bytes):
    bytes_(bytes)
{}

std::string UUID::toRFC4122String() const
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    // time-low
    for (size_t i = 0; i < 4; i++)
        ss << std::setw(2) << static_cast<int>(this->bytes_[i]);
    ss << '-';

    // time-mid
    for (size_t i = 4; i < 6; i++)
        ss << std::setw(2) << static_cast<int>(this->bytes_[i]);
    ss << '-';

    // time-high-and-version
    for (size_t i = 6; i < 8; i++)
        ss << std::setw(2) << static_cast<int>(this->bytes_[i]);
    ss << '-';

    // clock-seq-and-reserved and clock-seq-low
    for (size_t i = 8; i < 10; i++)
        ss << std::setw(2) << static_cast<int>(this->bytes_[i]);
    ss << '-';

    // node
    for (size_t i = 10; i < 16; i++)
        ss << std::setw(2) << static_cast<int>(this->bytes_[i]);

    return ss.str();
}

const std::array<std::byte, 16> &UUID::getBytes() const
{
    return this->bytes_;
}

void UUID::clear()
{
    std::fill(this->bytes_.begin(), this->bytes_.end(), std::byte{0});
}

bool operator<(const UUID &a, const UUID &b)
{
    return a.getBytes() < b.getBytes();
}

bool operator>(const UUID &a, const UUID &b)
{
    return a.getBytes() > b.getBytes();
}

bool operator<=(const UUID &a, const UUID &b)
{
    return a.getBytes() <= b.getBytes();
}

bool operator>=(const UUID &a, const UUID &b)
{
    return a.getBytes() >= b.getBytes();
}

bool operator==(const UUID &a, const UUID &b)
{
    return a.getBytes() == b.getBytes();
}

bool operator!=(const UUID &a, const UUID &b)
{
    return a.getBytes() != b.getBytes();
}


}} // END NAMESPACES.
// =====================================================================================================================
