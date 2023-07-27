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
#include "LibZMQUtils/libzmqutils_global.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace utils{
// =====================================================================================================================

/**
 * @class BinarySerializer
 *
 * @brief This class provides functionality for binary serialization and deserialization of data.
 *
 * The class is thread safe.
 *
 * @warning This class assumes a certain byte order (either big endian or little endian) for the data being serialized
 *          or deserialized. The binarySerializeDeserialize function reverses the byte order of the data. This might
 *          not be correct if the code runs in a different context or on a machine with a different native byte order.
 *          Therefore, consider detecting the machine's byte order and adjust the reversal accordingly if using this
 *          class in such an other specific context.
 */
class LIBZMQUTILS_EXPORT BinarySerializer
{
public:

    BinarySerializer(size_t capacity = 1024);

    BinarySerializer(std::uint8_t* data, size_t size);

    void reserve(size_t size);

    void loadData(std::uint8_t* data, size_t size);

    void clearData();

    void resetReading();

    template<typename... Args>
    void writeMultiple(const Args&... args);

    template<typename T>
    void writeSingle(const T& value);

    /**
     * @brief Variadic template function to read multiple data types at once from the internal buffer.
     *
     * @tparam Args Variadic template argument for types.
     * @param[out] args Lvalue references to the variables where the read data should be stored.
     *
     * @warning Make sure to read the values in the exact order and type they were written, otherwise undefined
     *          behavior will occur. Also, this function should not be used with rvalue references.
     *
     * @warning If you read beyond the size of the stored data, this function will throw an out_of_range exception.
     */
    template<typename... Args>
    void readMultiple(Args&... args);

    /**
     * @brief Read a value of a specific type from the internal buffer and return it.
     *
     * @tparam T Type of the value to be read.
     *
     * @return The read value.
     *
     * @warning If you read beyond the size of the stored data, this function will throw an out_of_range exception.
     */
    template<typename T>
    T readSingle();

    /**
     * @brief Read a value of a specific type from the internal buffer and store it in the given variable.
     *
     * @tparam T Type of the value to be read.
     * @param[out] value Reference to the variable where the read data should be stored.
     *
     * @warning If you read beyond the size of the stored data, this function will throw an out_of_range exception.
     */
    template<typename T>
    void readSingle(T& value);

    std::unique_ptr<std::uint8_t> moveData();

    std::unique_ptr<std::uint8_t> moveData(size_t& size);

    //std::uint8_t* getData() const;

    size_t getSize() const;

    std::string toString() const;

    std::string getDataHexString() const;


private:

    static void binarySerializeDeserialize(const void *data, size_t data_size_bytes, void *dest);

    std::unique_ptr<std::uint8_t[]> data_;
    std::atomic<size_t> size_;               ///< Current size of the data.
    std::atomic<size_t> capacity_;           ///< Current capacity.
    std::atomic<size_t> offset_;             ///< Offset when reading.
    mutable std::mutex mtx_;                 ///< Mutex for thread safety

};

using SerializedData = BinarySerializer;

}} // END NAMESPACES.
// =====================================================================================================================

// TEMPLATES INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Utilities/binary_serializer.tpp"
// =====================================================================================================================
