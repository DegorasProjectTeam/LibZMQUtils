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
 * @brief This file contains the declaration of the Serializable class and BinarySerializer class.
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
#include <atomic>
#include <fstream>
#include <istream>
#include <vector>
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

class BinarySerializer;

// Traits.
// ---------------------------------------------------------------------------------------------------------------------

template<typename... Ts>
struct trait_has_nullptr_t : std::disjunction<std::is_same<std::nullptr_t, Ts>...> {};

// ---------------------------------------------------------------------------------------------------------------------

/**
 * @class Serializable
 * @brief An interface that defines the contract for serializable objects.
 *
 * The Serializable class provides a common interface for all objects that can be serialized into and deserialized from
 * a binary format. Any class that needs to be serialized should inherit from this interface and implement the methods.
 *
 * The interface defines three key methods:
 *
 *   - `serialize`: Used to serialize the object into a binary format.
 *   - `deserialize`: Used to reconstruct the object from its binary representation.
 *   - `serializedSize`: Used to determine the size of the serialized object in bytes.
 *
 * Implementing these methods allows the object to be easily serialized and deserialized using a BinarySerializer.
 *
 * Example of a class that implements Serializable:
 *
 * @code{.cpp}
 * struct AltAzPos : Serializable
 * {
 *     AltAzPos(double az, double el): az(az), el(el){}
 *
 *     AltAzPos(): az(-1), el(-1){}
 *
 *     double az;
 *     double el;
 *
 *     size_t serialize(BinarySerializer& serializer) const override
 *     {
 *         return serializer.write(az, el);
 *     }
 *
 *     void deserialize(BinarySerializer& serializer) override
 *     {
 *         serializer.read(az, el);
 *     }
 *
 *     size_t serializedSize() const override
 *     {
 *         return (sizeof(double) + sizeof(double));
 *     }
 * };
 * @endcode
 *
 * @see BinarySerializer
 */
class LIBZMQUTILS_EXPORT Serializable
{
public:

    /**
     * @brief Virtual destructor to allow proper cleanup of derived classes.
     */
    virtual ~Serializable();

    /**
     * @brief Method to serialize the object into a binary format.
     * @param serializer The BinarySerializer instance that will handle the serialization.
     * @return The size of the serialized data in bytes.
     */
    virtual size_t serialize(BinarySerializer& serializer) const = 0;

    /**
     * @brief Method to deserialize the object from its binary representation.
     * @param serializer The BinarySerializer instance that will handle the deserialization.
     */
    virtual void deserialize(BinarySerializer& serializer) = 0;

    /**
     * @brief Returns the size of the object when serialized into binary format.
     * @return The size of the serialized object in bytes.
     */
    virtual size_t serializedSize() const = 0;
};

// ---------------------------------------------------------------------------------------------------------------------

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
 * - **Trivially Copyable and Trivial Types:** Basic data types that are both trivially copyable and trivial, such as
 *   integers, floating-point numbers, etc.
 *
 * - **Strings:** Standard C++ strings are supported, and their size information is stored to facilitate
 *   deserialization.
 *
 * - **Vectors and arrays of Trivially Copyable and Trivial Types:** Vectors containing trivially copyable and trivial
 *   types can be serialized/deserialized directly.
 *
 * - **Subclasses of the Serializable Class (properly implemented):** Classes that inherit from the Serializable
 *   interface and provide proper implementations of the `serialize`, `deserialize`, and `serializedSize` methods.
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
 * @note This class will detect the machine's byte order and will adjust the reversal accordingly if using this
 * class in a context with different native byte order.
 *
 * @see Serializable
 */
class LIBZMQUTILS_EXPORT BinarySerializer
{
public:

    using ElementSize = std::uint64_t;

    /// Enumeration representing the byte order (endianness) of data.
    enum class Endianess
    {
        LITTLE_ENDIAN, ///< Little-endian byte order (LSB first).
        BIG_ENDIAN     ///< Big-endian byte order (MSB first).
    };

    /**
     * @brief Construct a new Binary Serializer object with a given capacity.
     * @param capacity The initial capacity of the serializer. Default is 1024.
     */
    BinarySerializer(size_t capacity = 1024);

    /**
     * @brief Construct a new Binary Serializer object and load the given data.
     * @param src Pointer to the data source to load.
     * @param size Size of the data to load.
     * @warning The @a src parameter is a void pointer, so be careful.
     */
    BinarySerializer(void* src, size_t size);

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
    void loadData(void *src, size_t size);

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
    std::byte* release();

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
     * Example Output:
     *
     * @code
     * {
     *     "size": 128,
     *     "capacity": 256,
     *     "offset": 64,
     *     "hexadecimal": "01 23 45 67 89 ab cd ef"
     * }
     * @endcode
     *
     * @return A string representing the BinarySerializer object in JSON format.
     */
    std::string toJsonString() const;

    /**
     * @brief Get a hex string representation of the data held by the serializer.
     * @return Hex string representation of the data.
     */
    std::string getDataHexString() const;

    /**
     * @brief A static function that serializes multiple input data items into binary data.
     *
     * This function takes multiple input data items and serializes them into binary data, storing the resulting binary
     * data in the provided unique pointer. It first checks that all input data types are both trivially copyable and
     * trivial, then serializes each data item in order, consolidating the binary data.
     *
     * Usage Example:
     *
     * @code{.cpp}
     *   int x = 42;
     *   double y = 3.14;
     *   std::unique_ptr<std::byte> serializedData;
     *   size_t size = BinarySerializer::fastSerialization(serializedData, x, y);
     * @endcode
     *
     * @tparam Args Variadic template argument for types.
     * @param[out] out The unique pointer where the serialized data will be stored. The pointer will be allocated within the function.
     * @param[in] args The input data items to be serialized.
     * @return The size of the serialized data.
     */
    template<typename... Args>
    static size_t fastSerialization(std::unique_ptr<std::byte>& out, const Args&... args);

    /**
     * @brief A static function that deserializes binary data into its original data items.
     *
     * This function takes binary data and deserializes it into the original data items, storing them in the provided
     * variables. It first checks that all output data types are both trivially copyable and trivial, and then
     * deserializes the binary data in order, storing each resulting data item in its respective variable.
     *
     * Usage Example:
     *
     * @code{.cpp}
     *   BinarySerializer serializer;
     *   int x = 42;
     *   double y = 3.14;
     *   serializer.write(x, y);
     *   int read_x;
     *   double read_y;
     *   BinarySerializer::fastDeserialization(serializer.data(), serializer.size(), read_x, read_y);
     * @endcode
     *
     * @tparam Args Variadic template argument for types.
     * @param[in] src The binary data to be deserialized.
     * @param[in] size The size of the binary data.
     * @param[out] args The variables where the deserialized data items are stored.
     *
     * @throw std::out_of_range If not all data was deserialized.
     * @throw std::out_of_range If you read beyond the size of the stored data.

     * @warning The @a src parameter is a void pointer, so be careful.
     *
     * @note This function must always read all the data of the buffer.
     */
    template<typename... Args>
    static void fastDeserialization(void* src, size_t size, Args&... args);

    /**
     * @brief Serializes the given values into the binary stream.
     *
     * This function template writes the given values into the binary serializer, calculating the total size of
     * all arguments and then reserving space for them. It then forwards the call to a recursive write function
     * to handle the actual serialization.
     *
     * This generic write function is designed to work with all supported internal types as well as user-defined
     * types that inherit from the Serializable interface.
     *
     * Usage Example:
     *
     * @code{.cpp}
     *   BinarySerializer serializer;
     *   int x = 42;
     *   double y = 3.14;
     *   serializer.write(x, y);  // Serializes both x and y
     * @endcode
     *
     * @tparam T The type of the first value to be serialized.
     * @tparam Args The types of the remaining values to be serialized.
     *
     * @param value The first value to be serialized.
     * @param args The remaining values to be serialized.
     *
     * @return The total size in bytes that the values occupy after being serialized.
     *
     * @note The types being deserialized must meet the required conditions, such as being trivially copyable and
     * trivial, directly supported by this class, or being a subclass of the Serializable interface.
     */
    template<typename T, typename... Args, typename = std::enable_if_t<!trait_has_nullptr_t<T, Args...>::value>>
    size_t write(const T& value, const Args&... args);

    /**
     * @brief Serializes a file and its associated metadata into the binary stream.
     *
     * This function reads the content of the specified file, its name and size, and serializes them into the binary
     * serializer. It calculates the total size required for the serialized data and reserves the necessary space in
     * the serializer's buffer. The file content, size, and name are serialized in the order: size of filename,
     * filename, size of file content, file content.
     *
     * @param in_filenamepath The path to the file to be serialized (with the filename included).
     *
     * @return The total size in bytes that the serialized file and metadata occupy.
     *
     * @throw std::runtime_error If the file can't be opened for serialization.
     *
     * @note The function serializes the file content as binary data and does not perform any conversion or
     * transformation on the file content itself.
     */
    size_t writeFile(const std::string& in_filenamepath);

    /**
     * @brief Variadic template function to read multiple data types at once from the internal buffer.
     *
     * This function template reads the given values from the binary serializer's internal buffer and stores them in
     * the provided variables. It forwards the call to a recursive read function to handle the actual deserialization.
     *
     * Usage Example:
     *
     * @code{.cpp}
     *   BinarySerializer serializer;
     *   int x = 42;
     *   double y = 3.14;
     *   serializer.write(x, y);  // Serializes both x and y
     *   int read_x;
     *   double read_y;
     *   serializer.read(read_x, read_y);  // Deserializes to read_x and read_y
     * @endcode
     *
     * @tparam T The type of the first value to be deserialized.
     * @tparam Args The types of the remaining values to be deserialized.
     * @param[out] value The first lvalue reference where the read data should be stored.
     * @param[out] args The remaining lvalue references where the read data should be stored.
     *
     * @warning Make sure to read the values in the exact order and type they were written, otherwise undefined
     * behavior will occur. Also, this function should not be used with rvalue references.
     *
     * @throw std::out_of_range If you read beyond the size of the stored data.
     *
     * @note The types being deserialized must meet the required conditions, such as being trivially copyable and
     * trivial, directly supported by this class, or being a subclass of the Serializable interface.
     */
    template<typename T, typename... Args>
    void read(T& value, Args&... args);

    void readFile(const std::string& out_filepath);

private:

    // -----------------------------------------------------------------------------------------------------------------

    // Type traits.

    template <typename T>
    struct is_container : std::false_type {};

    template <typename... Args>
    struct is_container<std::vector<Args...>> : std::true_type {};

    template <typename T, std::size_t N>
    struct is_container<std::array<T, N>> : std::true_type {};

    // -----------------------------------------------------------------------------------------------------------------

    static Endianess determineEndianess();

    // Internal function to check if the type is trivially copiable.
    template<typename T>
    static void checkTriviallyCopyable();

    // Internal function to check if the type is trivial.
    template<typename T>
    static void checkTrivial();

    // Internal size calculator function.
    template<typename T>
    static std::uint64_t calcTotalSize(const T& data);

    // Internal binary serialization/deserialization helper function.
    template<typename T, typename C>
    static void binarySerializeDeserialize(const T* src, size_t data_size_bytes, C* dst, bool reverse);

    // Internal binary serialization helper function.
    template<typename T, typename C>
    void binarySerialize(const T* src, size_t data_size_bytes, C* dst);

    // Internal binary deserialization helper function.
    template<typename T, typename C>
    void binaryDeserialize(const T *src, size_t data_size_bytes, C *dst);

    // Recursive writing helper.
    template<typename T, typename... Args>
    size_t writeRecursive(const T& value, const Args&... args);

    // Recursive reaad helper.
    template<typename T, typename... Args>
    void readRecursive(T& value, Args&... args);

    // -----------------------------------------------------------------------------------------------------------------

    // Write data functions.

    // Generic writing.
    template<typename T>
    typename std::enable_if_t<
            !is_container<T>::value &&
            !std::is_base_of_v<Serializable, T> &&
            !std::is_same_v<std::nullptr_t &&, T> &&
            !std::is_pointer_v<T>, size_t>
    writeSingle(const T& obj);

    // For writing Serializable objects.
    size_t writeSingle(const Serializable& obj);

    // For writing strings.
    size_t writeSingle(const std::string& str);

    // For arrays of trivial types.
    template<typename T, size_t L>
    size_t writeSingle(const std::array<T, L>& arr);

    // For vectors of trivial types.
    template<typename T>
    size_t writeSingle(const std::vector<T>& v);



    // -----------------------------------------------------------------------------------------------------------------

    // Read data functions.

    // Generic reading.
    template<typename T>
    typename std::enable_if<
            !is_container<T>::value &&
            !std::is_base_of_v<Serializable, T> &&
            !std::is_same_v<std::nullptr_t &&, T> &&
            !std::is_pointer_v<T>,
        void>::type
    readSingle(T& value);

    // For read Serializable objects.
    void readSingle(Serializable& obj);

    // For read strings.
    void readSingle(std::string& str);

    // For arrays of trivial types.
    template<typename T, size_t L>
    void readSingle(std::array<T, L>& arr);

    // -----------------------------------------------------------------------------------------------------------------

    // Internal containers and variables.
    std::unique_ptr<std::byte[]> data_;      ///< Internal data pointer.
    std::atomic<size_t> size_;               ///< Current size of the data.
    std::atomic<size_t> capacity_;           ///< Current capacity.
    std::atomic<size_t> offset_;             ///< Offset when reading.
    Endianess endianess_;                    ///< Represent the endianess of the system.
    mutable std::mutex mtx_;                 ///< Mutex for thread safety
};

}} // END NAMESPACES.git s
// =====================================================================================================================

// TEMPLATES INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.tpp"
// =====================================================================================================================
