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
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <cstddef>
#include <mutex>
#include <random>
#include <set>
#include <array>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
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

    LIBZMQUTILS_EXPORT static inline constexpr unsigned kUUIDSize = 16;  ///< UUID bytes size.

    /**
     * @brief Construct a new UUID object from an array of 16 bytes.
     * @param bytes An array of 16 bytes representing the UUID.
     */
    LIBZMQUTILS_EXPORT UUID(const std::array<std::byte, 16>& bytes);

    /**
     * @brief Default constructor.
     */
    LIBZMQUTILS_EXPORT UUID() = default;

    LIBZMQUTILS_EXPORT UUID(const UUID&) = default;
    LIBZMQUTILS_EXPORT UUID(UUID&&) = default;

    LIBZMQUTILS_EXPORT UUID& operator=(const UUID&) = default;
    LIBZMQUTILS_EXPORT UUID& operator=(UUID&&) = default;

    /**
     * @brief Returns string representation of the UUID
     *
     * This function converts the UUID into a string following the standard representation of a UUID, which consists of
     * 32 hexadecimal digits displayed in five groups separated by hyphens, in the form 8-4-4-4-12 for a total of 36
     * characters (including the hyphens).
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
     * The function std::stringstream, std::hex, std::setfill, std::setw, and std::hex are used to convert each part of
     * the UUID into a hex string and concatenate them together with '-' as separator. The std::setw(2) ensures that
     * each byte is represented by exactly two hex characters, padding with a zero if necessary.
     *
     * This method's implementation is in alignment with RFC 4122: A Universally Unique IDentifier (UUID) URN Namespace,
     * available at: https://www.ietf.org/rfc/rfc4122.txt
     *
     * @return String representation of the UUID
     */
    LIBZMQUTILS_EXPORT std::string toRFC4122String() const;

    LIBZMQUTILS_EXPORT const std::array<std::byte, 16>& getBytes() const;


private:

    // Members.
    std::array<std::byte, 16> bytes_;  ///< Bytes of the UUID.
};

// UUID logical comparison operators
/**
 * @brief UUID less operator
 * @param a
 * @param b
 * @return true if a is less than b, false otherwise
 */
LIBZMQUTILS_EXPORT bool operator<(const UUID& a, const UUID& b);
/**
 * @brief UUID greater operator
 * @param a
 * @param b
 * @return true if a is greater than b, false otherwise
 */
LIBZMQUTILS_EXPORT bool operator>(const UUID& a, const UUID& b);
/**
 * @brief UUID less or equal operator
 * @param a
 * @param b
 * @return true if a is less or equal than b, false otherwise
 */
LIBZMQUTILS_EXPORT bool operator<=(const UUID& a, const UUID& b);
/**
 * @brief UUID greater or equal operator
 * @param a
 * @param b
 * @return true if a is greater or equal than b, false otherwise
 */
LIBZMQUTILS_EXPORT bool operator>=(const UUID& a, const UUID& b);
/**
 * @brief UUID equal operator
 * @param a
 * @param b
 * @return true if a is equal than b, false otherwise
 */
LIBZMQUTILS_EXPORT bool operator==(const UUID& a, const UUID& b);
/**
 * @brief UUID not equal operator
 * @param a
 * @param b
 * @return true if a is not equal than b, false otherwise
 */
LIBZMQUTILS_EXPORT bool operator!=(const UUID& a, const UUID& b);

/**
 * @class UUIDGenerator
 *
 * @brief This singleton class provides functionality for generating UUID (Universally Unique Identifier).
 *
 * This singleton class supports generation of Version 4 UUIDs as per RFC 4122. A Version 4 UUID is randomly generated.
 *
 * The UUID generated by this class is a 128-bit value. The string representation is a series of lowercase hexadecimal
 * digits in groups, separated by hyphens, in the form 8-4-4-4-12 for a total of 36 characters (32 alphanumeric
 * characters and four hyphens).
 *
 * The algorithm used for generation is as follows:
 * 1. Generate 16 random bytes
 * 2. Adjust certain bits according to RFC 4122 section 4.4 as follows:
 *    a. Set the four most significant bits of the 7th byte to 0100'B, so the high nibble is "4".
 *    b. Set the two most significant bits of the 9th byte to 10'B, so the high nibble will be one of "8", "9",
 *    "A", or "B".
 * 3. Convert the byte array to a string in the form of 8-4-4-4-12
 *
 * @note The class is thread safe.
 *
 * @note The UUIDs generated are pseudo-random numbers. While the randomness of the generated UUIDs is sufficient for
 * most purposes, it is not suitable for functions that need truly random numbers.
 *
 * @warning This class relies on std::random_device for random number generation. On some platforms, this does not
 * actually provide a non-deterministic random number generator. In such cases, the randomness of the generated UUIDs
 * may be weaker and a random seed will be generated using a timestamp.
 */
class UUIDGenerator
{
public:

    /**
     * @brief Access to the singleton instance
     */
    static UUIDGenerator& getInstance();

    /**
     * @brief Generates a version 4 UUID
     * @return A unique UUID
     */
    UUID generateUUIDv4();

    // Deleted constructors and assignment operators.
    UUIDGenerator(const UUIDGenerator &) = delete;
    UUIDGenerator(UUIDGenerator&&) = delete;
    UUIDGenerator& operator=(const UUIDGenerator &) = delete;
    UUIDGenerator& operator=(UUIDGenerator&&) = delete;

private:

    // Private constructor.
    UUIDGenerator();

    // Members.
    std::mutex mtx_;                  ///< Safety mutex.
    std::random_device rd_;           ///< Random device.
    std::mt19937_64 gen_;             ///< Generator.
    std::set<UUID> generated_uuids_;  ///< Set of all generated UUIDs to ensure uniqueness.
};

}} // END NAMESPACES.
// =====================================================================================================================
