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
#include <vector>
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

class BinarySerializer;

class Serializable
{
public:

    virtual ~Serializable() = default;

    // Method to serialize the object to a binary format.
    virtual size_t serialize(BinarySerializer& serializer) const = 0;

    // Method to deserialize the object from a binary format.
    virtual void deserialize(BinarySerializer& serializer) = 0;

    virtual size_t serializedSize() const = 0;
};



/**
 * @class BinarySerializer
 *
 * @brief This class provides functionality for binary serialization and deserialization of simple data types, as well
 * as classes that implement the Serializable interface.
 *
 * The BinarySerializer class supports serialization and deserialization of trivially copyable and trivial data types.
 * This class provides static methods for fast serialization and deserialization using variadic templates, allowing for
 * multiple data types to be serialized/deserialized at once. Additionally, the class provides methods for checking at
 * compile time whether the types provided meet the requirements for being trivially copyable and trivial.
 *
 * The class also includes support for custom serialization and deserialization of more complex data structures
 * by means of the Serializable interface. Classes that need to be serialized/deserialized with BinarySerializer should
 * inherit from the Serializable interface and implement the `serialize`, `deserialize` and `serializedSize` methods.
 * This allows for handling complex data, enabling fine-grained control over the serialization process.
 *
 * @section Supported Data Types
 *
 * The BinarySerializer class is capable of serializing and deserializing the following types of data:
 *
 * 1. **Trivially Copyable and Trivial Types:** Basic data types that are both trivially copyable and trivial, such as
 *    integers, floating-point numbers, etc.
 * 2. **Strings:** Standard C++ strings are supported, and their size information is stored to facilitate
 *    deserialization.
 * 3. **Vectors of Trivially Copyable and Trivial Types:** Vectors containing trivially copyable and trivial types can
 *    be serialized/deserialized directly.
 * 4. **Subclasses of the Serializable Class (properly implemented):** Classes that inherit from the Serializable
 *    interface and provide proper implementations of the `serialize`, `deserialize`, and `serializedSize` methods.
 *
 * Usage with other complex data structures or types that do not fit into these categories is not supported directly
 * and may require additional consideration or custom handling.
 *
 * @note The class is thread safe.
 *
 * @note It's strongly recommended that this class is only used with simple data types (like ints, floats, double,
 * char*, etc.) that fulfill the conditions of being both trivial and trivially copyable, or with classes implementing
 * the Serializable interface. It's not designed to handle other complex data structures like custom classes directly.
 * Before using this class with unsupported data types, appropriate checks should be added to ensure they meet the
 * necessary conditions for serialization and deserialization.
 *
 * @warning This class assumes a certain byte order (either big endian or little endian) for the data being serialized
 * or deserialized. It's important to be aware of the byte order in the context the data will be used. Therefore,
 * consider detecting the machine's byte order and adjust the reversal accordingly if using this class in a context
 * with different native byte order. However, this is usually not a problem with modern platforms, which typically use
 * the same byte order (little-endian).
 *
 * @see Serializable
 */
class LIBZMQUTILS_EXPORT BinarySerializer
{
public:

    /**
     * @brief Construct a new Binary Serializer object with a given capacity.
     * @param capacity The initial capacity of the serializer. Default is 1024.
     */
    BinarySerializer(size_t capacity = 1024);

    /**
     * @brief Construct a new Binary Serializer object and load the given data.
     * @param data Pointer to the data to load.
     * @param size Size of the data to load.
     * @warning The @a data parameter is a void pointer, so be careful.
     */
    BinarySerializer(void* data, size_t size);

    /**
     * @brief Reserve memory for the serializer.
     * @param size The size of memory to reserve.
     */
    void reserve(size_t size);

    /**
     * @brief Load data into the serializer.
     * @param data Pointer to the data to load.
     * @param size Size of the data to load.
     * @warning The @a data parameter is a void pointer, so be careful.
     */
    void loadData(void *data, size_t size);

    /**
     * @brief Clear the data held by the serializer.
     */
    void clearData();

    /**
     * @brief Reset the internal read offset.
     */
    void resetReading();

    /**
     * @brief Release the data held by the serializer and return a raw pointer to it.
     * @return Raw pointer to the data.
     */
    std::byte *release();

    /**
     * @brief Release the data held by the serializer, return a raw pointer to it, and set the size variable.
     * @param[out] size The size of the data.
     * @return Raw pointer to the data.
     */
    std::byte* release(size_t& size);

    /**
     * @brief Move the unique pointer to the data held by the serializer and return it.
     * @return Unique pointer to the data.
     */
    std::unique_ptr<std::byte> moveUnique();

    /**
     * @brief Move the unique pointer to the data held by the serializer, return it, and set the size variable.
     * @param[out] size The size of the data.
     * @return Unique pointer to the data.
     */
    std::unique_ptr<std::byte> moveUnique(size_t& size);

    /**
     * @brief Get the current size of the data held by the serializer.
     * @return The current size of the data.
     */
    size_t getSize() const;

    /**
     * @brief Check whether all data has been read.
     * @return True if all data has been read, false otherwise.
     */
    bool allReaded() const;

    /**
     * @brief Convert the internal state of the BinarySerializer to a JSON-formatted string.
     *
     * This function generates a JSON string that represents the current state of the BinarySerializer object. The JSON
     * string includes the following properties: size, capacity, offset, and hex_data (which represents the serialized
     * data in hexadecimal format).
     *
     * @return A string representing the BinarySerializer object in JSON format.
     *
     * Example output:
     * @code
     * {
     *     "size": 128,
     *     "capacity": 256,
     *     "offset": 64,
     *     "hex_data": "01 23 45 67 89 ab cd ef"
     * }
     * @endcode
     */
    std::string toJsonString() const;

    /**
     * @brief Get a hex string representation of the data held by the serializer.
     * @return Hex string representation of the data.
     */
    std::string getDataHexString() const;

    // -- GENERIC WRITE

    /**
     * @brief Write data into the serializer.
     * @tparam Args Variadic template argument for types.
     * @param args Arguments to write into the serializer.
     */


    template<typename T, typename... Args>
    size_t write(const T& value, const Args&... args);


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
    template<typename... Args> void read(Args&... args);

    template<typename T, typename... Args>
    void read(T& value, Args&... args)
    {

        // Read the data.
        this->readRecursive(value, args...);
    }

    /**
     * @brief Read a value of a specific type from the internal buffer and return it.
     *
     * @tparam T Type of the value to be read.
     *
     * @return The read value.
     *
     * @warning Make sure to read the values in the exact order and type they were written, otherwise undefined
     *          behavior will occur. Also, this function should not be used with rvalue references.
     *
     * @warning If you read beyond the size of the stored data, this function will throw an out_of_range exception.
     */
    template<typename T> T readSingle();

    std::string readSingle()
    {
        std::string aux;
        this->readSingle(aux);
        return aux;
    }



    /**
     * @brief A static function that serializes multiple input data items.
     *
     * @tparam Args Variadic template argument for types.
     * @param[out] out The unique pointer where the serialized data will be stored.
     * @param[in] args The input data items to be serialized.
     * @return The size of the serialized data.
     *
     * @throw std::invalid_argument If any of the input data types are not trivially copyable and trivial.
     *
     * @note This function first checks that all input data types are both trivially copyable and trivial, then it
     *       serializes each data item in order, storing the resulting binary data in the unique pointer.
     */
    template<typename... Args>
    static size_t fastSerialization(std::unique_ptr<std::byte>& out, const Args&... args);

    /**
     * @brief A static function that deserializes binary data into its original data items.
     *
     * @tparam Args Variadic template argument for types.
     * @param[in] in The binary data to be deserialized.
     * @param[in] size The size of the binary data.
     * @param[out] args The variables where the deserialized data items are stored.
     *
     * @throw std::invalid_argument If any of the output data types are not trivially copyable and trivial.
     * @throw std::out_of_range If not all data was deserialized.
     *
     * @warning The @in parameter is a void pointer, so be careful.
     *
     * @note This function first checks that all output data types are both trivially copyable and trivial, then it
     *       deserializes the binary data in order, storing each resulting data item in its respective variable. If not
     *       all data was deserialized (e.g., if the size of the binary data does not match the total size of the
     *       output data items), the function throws an exception.
     */
    template<typename... Args>
    static void fastDeserialization(void* in, size_t size, Args&... args);

    template<typename T>
    size_t writeSingle(const std::vector<T>& v)
    {
        // Check the types.
        BinarySerializer::checkTriviallyCopyable<T>();
        BinarySerializer::checkTrivial<T>();

        // Get the total size and reserve.
        size_t total_size = sizeof(T)*v.size();
        this->reserve(total_size);

        // Write each value of the vector.
        for(const auto& val : v)
            this->writeSingle(val);

        // Return the writed size.
        return total_size;
    }



    size_t writeSingle(const Serializable& obj)
    {
        std::cout<<"Writing ser obj"<<std::endl;
        return obj.serialize(*this);
    }






private:



    // Internal binary serialization/deserialization helper function.
    template<typename T, typename C>
    static void binarySerializeDeserialize(const T* data, size_t data_size_bytes, C* dest);

    // Internal size calculator function.
    template<typename T>
    static size_t calcSize(const T& value);

    // Internal function to check if the type is trivially copiable.
    template<typename T>
    static void checkTriviallyCopyable();

    // Internal function to check if the type is trivial.
    template<typename T>
    static void checkTrivial();

    // ------------------------------------------------------------------------------------------

    // Helper write methods.

    template<typename T>
    typename std::enable_if<!std::is_base_of<Serializable, T>::value, size_t>::type
    writeSingle(const T& obj);

    template<typename T, typename... Args>
    size_t writeRecursive(const T& value, const Args&... args);

    // ------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------

    // Helper read methods.

    template<typename T>
    void readSingle(T& value);

    template<typename T, typename... Args>
    void readRecursive(T& value, Args&... args)
    {
        this->readSingle(value);
        if constexpr (sizeof...(args) > 0)
            this->readRecursive(std::forward<Args&>(args)...);
    }

    // ------------------------------------------------------------------------------------------


    // Non trivial data functions.



    size_t writeSingle(const std::string& str);

    void readSingle(std::string& value)
    {
        // Mutex.
        std::lock_guard<std::mutex> lock(this->mtx_);

        // Ensure that there's enough data left to read the size of the string.
        if (this->offset_ + sizeof(size_t) > this->size_)
            throw std::out_of_range("BinarySerializer: Not enough data left to read the size of the string");

        // Read the size of the string.
        size_t size;
        BinarySerializer::binarySerializeDeserialize(this->data_.get() + this->offset_, sizeof(size_t), &size);

        // Check if the string is empty.
        if(size == 0)
            return;

        // Update the offset.
        this->offset_ += sizeof(size_t);

        // Check if we have enough data left to read the string.
        if (this->offset_ + size > this->size_)
            throw std::out_of_range("BinarySerializer: Read string beyond the data size.");

        // Read the string.
        value.resize(size);
        BinarySerializer::binarySerializeDeserialize(this->data_.get() + this->offset_, size, value.data());
        this->offset_ += size;
    }


    template<std::size_t N>
    void readSingle(std::array<std::byte, N>& value)
    {
        // Mutex.
        std::lock_guard<std::mutex> lock(this->mtx_);

        // Check if we have enough data.
        if (this->offset_ + N > this->size_)
            throw std::out_of_range("BinarySerializer: Read beyond the data size");

        // Read the size of the array.
        std::copy(this->data_.get() + this->offset_, this->data_.get() + this->offset_ + N, value.begin());

        // Update the offset.
        this->offset_ += N;
    }









    // Internal containers and variables.
    std::unique_ptr<std::byte[]> data_;      ///< Internal data pointer.
    std::atomic<size_t> size_;               ///< Current size of the data.
    std::atomic<size_t> capacity_;           ///< Current capacity.
    std::atomic<size_t> offset_;             ///< Offset when reading.
    mutable std::mutex mtx_;                 ///< Mutex for thread safety
};

}} // END NAMESPACES.
// =====================================================================================================================

// TEMPLATES INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Utilities/binary_serializer.tpp"
// =====================================================================================================================
