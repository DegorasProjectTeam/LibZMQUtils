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
 * @file uuid_generator.h
 * @brief This file contains the declaration of the UUIDGenerator class.
 * @author Degoras Project Team
 * @copyright EUPL License
 * @version 2307.1
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <array>
#include <memory>
#include <set>
#include <chrono>
#include <thread>
// =====================================================================================================================


// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{
// =====================================================================================================================

/**
 * @brief Class to encapsulate the UUID bytes and provide string representation
 */
class UUID
{

public:

    /**
     * @brief Construct a new UUID object from an array of 16 bytes.
     * @param bytes An array of 16 bytes representing the UUID.
     */
    UUID(const std::array<uint8_t, 16>& bytes):
        bytes_(bytes)
    {}

    UUID() = default;

    /**
     * @brief Returns string representation of the UUID
     *
     * This function converts the UUID into a string following the standard representation of a UUID,
     * which consists of 32 hexadecimal digits displayed in five groups separated by hyphens,
     * in the form 8-4-4-4-12 for a total of 36 characters (including the hyphens).
     *
     * The string representation is divided as follows:
     * 1. time-low: The first 8 hex digits (4 bytes).
     * 2. time-mid: The next 4 hex digits (2 bytes).
     * 3. time-high-and-version: The next 4 hex digits (2 bytes).
     * 4. clock-seq-and-reserved and clock-seq-low: The next 4 hex digits (2 bytes).
     * 5. node: The last 12 hex digits (6 bytes).
     *
     * An example of a UUID string: 550e8400-e29b-41d4-a716-446655440000
     *
     * The function std::stringstream, std::hex, std::setfill, std::setw, and std::hex are used to convert
     * each part of the UUID into a hex string and concatenate them together with '-' as separator.
     * The std::setw(2) ensures that each byte is represented by exactly two hex characters, padding with
     * a zero if necessary.
     *
     * This method's implementation is in alignment with RFC 4122: A Universally Unique IDentifier (UUID) URN Namespace,
     * available at: https://www.ietf.org/rfc/rfc4122.txt
     *
     * @return String representation of the UUID
     */
    std::string toRFC4122String() const
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        // time-low
        for (int i = 0; i < 4; i++)
            ss << std::setw(2) << static_cast<int>(this->bytes_[i]);
        ss << '-';

        // time-mid
        for (int i = 4; i < 6; i++)
            ss << std::setw(2) << static_cast<int>(this->bytes_[i]);
        ss << '-';

        // time-high-and-version
        for (int i = 6; i < 8; i++)
            ss << std::setw(2) << static_cast<int>(this->bytes_[i]);
        ss << '-';

        // clock-seq-and-reserved and clock-seq-low
        for (int i = 8; i < 10; i++)
            ss << std::setw(2) << static_cast<int>(this->bytes_[i]);
        ss << '-';

        // node
        for (int i = 10; i < 16; i++)
            ss << std::setw(2) << static_cast<int>(this->bytes_[i]);

        return ss.str();
    }

    const std::array<uint8_t, 16>& getBytes() const {return this->bytes_;}

    bool operator<(const UUID& rhs) const
    {
        return this->bytes_ < rhs.bytes_;
    }

private:

    // Members.
    std::array<uint8_t, 16> bytes_;  /// Bytes of the UUID.
};

/**
 * @class UUIDGenerator
 *
 * @brief This class provides functionality for generating UUID (Universally Unique Identifier).
 *
 * The UUIDGenerator class supports generation of Version 4 UUIDs as per RFC 4122. A Version 4 UUID is randomly generated.
 *
 * The UUID generated by this class is a 128-bit value. The string representation is a series of lowercase hexadecimal digits
 * in groups, separated by hyphens, in the form 8-4-4-4-12 for a total of 36 characters (32 alphanumeric characters and four hyphens).
 *
 * @note The class is thread safe.
 *
 * @note The UUIDs generated are pseudo-random numbers. While the randomness of the generated UUIDs is sufficient for most
 *       purposes, it is not suitable for functions that need truly random numbers.
 *
 * @warning This class relies on std::random_device for random number generation. On some platforms, std::random_device does
 *          not actually provide a non-deterministic random number generator. In such cases, the randomness of the generated
 *          UUIDs may be weaker.
 */
class UUIDGenerator
{
public:



    /**
     * @brief Generates a version 4 UUID
     * @return A unique UUID
     *
     * The algorithm used for generation is as follows:
     * 1. Generate 16 random bytes
     * 2. Adjust certain bits according to RFC 4122 section 4.4 as follows:
     *    a. Set the four most significant bits of the 7th byte to 0100'B, so the high nibble is "4".
     *    b. Set the two most significant bits of the 9th byte to 10'B, so the high nibble will be one of "8", "9", "A", or "B".
     * 3. Convert the byte array to a string in the form of 8-4-4-4-12
     */
    UUID generateUUIDv4()
    {
        std::array<uint8_t, 16> bytes;
        UUID uuid;
        std::random_device rd;  // Will be used to obtain a seed for the random number engine
        std::uniform_int_distribution<> distrib(0, 255);
        std::mt19937 gen;

        if(rd.entropy() == 0)
        {
            gen = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        }
        else
        {
            gen = std::mt19937(rd());
        }

        do
        {
            // Safe sleep.
            std::this_thread::sleep_for(std::chrono::microseconds(50));

            for(auto& byte : bytes)
                byte = static_cast<uint8_t>(distrib(gen));

            // Set the version to 4 (random)
            bytes[6] = (bytes[6] & 0x0F) | 0x40;

            // Set the variant to 1 (RFC4122)
            bytes[8] = (bytes[8] & 0x3F) | 0x80;

            // Generate the UUID.
            uuid = UUID(bytes);

        } while(generatedUUIDs.find(uuid) != generatedUUIDs.end());

        generatedUUIDs.insert(uuid);

        return uuid;
    }

private:
    /// \brief Set of all generated UUIDs to ensure uniqueness
    std::set<UUID> generatedUUIDs;


};

}} // END NAMESPACES.
// =====================================================================================================================
