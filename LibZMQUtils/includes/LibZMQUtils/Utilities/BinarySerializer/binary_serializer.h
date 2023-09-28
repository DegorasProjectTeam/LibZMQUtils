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
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
// =====================================================================================================================
#include <mutex>
#include <vector>
#include <atomic>
#include <memory>
#include <cstddef>
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

    Serializable() = default;

    Serializable(const Serializable&) = default;

    Serializable& operator =(const Serializable&) = default;

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
class BinarySerializer
{
public:

    using SizeUnit = std::uint64_t;                      ///< Alias for the size unit.
    using BytesSmartPtr = std::unique_ptr<std::byte[]>;  ///< Alias for the bytes storage smart pointer.

    /// Enumeration representing the byte order (endianness) of data.
    enum class Endianess
    {
        LITTLE_ENDIAN, ///< Little-endian byte order (LSB first).
        BIG_ENDIAN     ///< Big-endian byte order (MSB first).
    };

    /**
     * @brief Construct a new ´BinarySerializer´ object with a given capacity.
     * @param capacity The initial capacity of the serializer. Default is 1024.
     */
    LIBZMQUTILS_EXPORT BinarySerializer(SizeUnit capacity = 1024);

    /**
     * @brief Construct a new ´BinarySerializer´ object and load the given data.
     * @param src Pointer to the data source to load.
     * @param size Size of the data to load.
     * @warning The @a src parameter is a void pointer, so be careful.
     * @warning This constructor implies deep copy.
     */
    LIBZMQUTILS_EXPORT BinarySerializer(void* src, SizeUnit size);

    LIBZMQUTILS_EXPORT BinarySerializer(BytesSmartPtr&& src, SizeUnit size);


    /**
     * @brief Reserve memory for the serializer.
     * @param size The size of memory to reserve.
     * @warning This function implies deep copy if the class has data.
     */
    LIBZMQUTILS_EXPORT void reserve(SizeUnit size);

    /**
     * @brief Load data into the serializer.
     * @param data Pointer to the data to load.
     * @param size Size of the data to load.
     * @warning The @a data parameter is a void pointer, so be careful.
     * @warning This function implies deep copy.
     */
    LIBZMQUTILS_EXPORT void loadData(void *src, SizeUnit size);



    /**
     * @brief Clear the data held by the serializer.
     */
    LIBZMQUTILS_EXPORT void clearData();

    /**
     * @brief Reset the internal read offset.
     */
    LIBZMQUTILS_EXPORT void resetReading();

    /**
     * @brief Release the data held by the serializer and return a raw pointer to it.
     * @return Raw pointer to the data.
     */
    LIBZMQUTILS_EXPORT std::byte* release();

    /**
     * @brief Release the data held by the serializer, return a raw pointer to it, and set the size variable.
     * @param[out] size The size of the data.
     * @return Raw pointer to the data.
     */
    LIBZMQUTILS_EXPORT std::byte* release(SizeUnit& size);

    /**
     * @brief Move the unique pointer to the data held by the serializer and return it.
     * @param[out] out The smart pointer with the data.
     * @return The current size of the data.
     */
    LIBZMQUTILS_EXPORT SizeUnit moveUnique(BinarySerializer::BytesSmartPtr& out);

    /**
     * @brief Get the current size of the data held by the serializer.
     * @return The current size of the data.
     */
    LIBZMQUTILS_EXPORT SizeUnit getSize() const;

    /**
     * @brief Check whether all data has been read.
     * @return True if all data has been read, false otherwise.
     */
    LIBZMQUTILS_EXPORT bool allReaded() const;

    /**
     * @brief Convert the internal state of the ´BinarySerializer´ to a JSON-formatted string.
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
    LIBZMQUTILS_EXPORT std::string toJsonString() const;

    /**
     * @brief Get a hex string representation of the data held by the serializer.
     * @return Hex string representation of the data.
     */
    LIBZMQUTILS_EXPORT std::string getDataHexString() const;

    /**
     * @brief A static function that serializes multiple input data items into binary data.
     *
     * This function takes multiple input data items and serializes them into binary data, storing the resulting binary
     * data in the provided unique pointer. It first checks that all input data types are both trivially copyable and
     * trivial, then serializes each data item in order, consolidating the binary data.
     *
     * @tparam Args Variadic template argument for types.
     * @param[out] out The unique pointer where the serialized data will be stored. The pointer will be allocated within the function.
     * @param[in] args The input data items to be serialized.
     * @return The size of the serialized data.
     */
    template<typename... Args>
    static SizeUnit fastSerialization(BytesSmartPtr& out, const Args&... args);

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
     * @warning This function implies deep copy.
     * @note This function must always read all the data of the buffer.
     */
    template<typename... Args>
    static void fastDeserialization(void* src, SizeUnit size, Args&... args);

    template<typename... Args>
    static void fastDeserialization(BytesSmartPtr&& src, SizeUnit size, Args&... args);

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
    SizeUnit write(const T& value, const Args&... args);

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
    LIBZMQUTILS_EXPORT SizeUnit writeFile(const std::string& in_filenamepath);

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
     * @throw std::logic_error If the serialized value size is greater than type for storage.
     *
     * @note The types being deserialized must meet the required conditions, such as being trivially copyable and
     * trivial, directly supported by this class, or being a subclass of the Serializable interface.
     */
    template<typename T, typename... Args>
    void read(T& value, Args&... args);

    /**
     * @brief Deserializes and writes the content of a previously serialized file to a new file.
     *
     * This function reads the serialized data from the internal buffer and deserializes it to reconstruct the content
     * of a previously serialized file. It then writes the deserialized content to a new file in the specified path.
     *
     * @param out_filepath The path of the new file to be created.
     *
     * @throws std::out_of_range If there's not enough data left to read the size of the filename or file content.
     * @throws std::runtime_error If the file for can't be opened or if an empty filename is encountered.
     *
     * @note This function assumes that the serialized data in the internal buffer corresponds to a previously
     * serialized file created using the `writeFile` function.
     */
    LIBZMQUTILS_EXPORT void readFile(const std::string& out_filepath);

protected:

    // -----------------------------------------------------------------------------------------------------------------

    // Type traits.

    template <typename T>
    struct is_container : std::false_type {};

    template <typename... Args>
    struct is_container<std::vector<Args...>> : std::true_type {};

    template <typename T, SizeUnit N>
    struct is_container<std::array<T, N>> : std::true_type {};

    // -----------------------------------------------------------------------------------------------------------------

    // Internal function to determine the endianess of the system.
    LIBZMQUTILS_EXPORT static Endianess determineEndianess();

    // Internal function to check if the type is trivially copiable.
    template<typename T>
    static void checkTriviallyCopyable();

    // Internal function to check if the type is trivial.
    template<typename T>
    static void checkTrivial();

    // Internal size calculator function.
    template<typename T>
    static SizeUnit calcTotalSize(const T& data);

    // Internal size calculator function for vectors.
    template<typename T>
    static SizeUnit calcTotalSize(const std::vector<T>& data);

    // Internal size calculator function for arrays.
    template<typename T, size_t L>
    static SizeUnit calcTotalSize(const std::array<T, L>& data);

    // Internal binary serialization/deserialization helper function.
    template<typename T, typename C>
    static void binarySerializeDeserialize(const T* src, SizeUnit data_size_bytes, C* dst, bool reverse);

    // Internal binary serialization helper function.
    template<typename T, typename C>
    void binarySerialize(const T* src, SizeUnit data_size_bytes, C* dst);

    // Internal binary deserialization helper function.
    template<typename T, typename C>
    void binaryDeserialize(const T *src, SizeUnit data_size_bytes, C *dst);

    // Recursive writing helper.
    template<typename T, typename... Args>
    void writeRecursive(const T& value, const Args&... args);

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
            !std::is_pointer_v<T>, void>
    writeSingle(const T& obj);

    // For writing Serializable objects.
    LIBZMQUTILS_EXPORT void writeSingle(const Serializable& obj);

    // For writing strings.
    LIBZMQUTILS_EXPORT void writeSingle(const std::string& str);

    // For arrays of trivial types.
    template<typename T, SizeUnit L>
    void writeSingle(const std::array<T, L>& arr);

    // For vectors of trivial types.
    template<typename T>
    void writeSingle(const std::vector<T>& v);

    // -----------------------------------------------------------------------------------------------------------------

    // Read data functions.

    // Generic reading.
    template<typename T>
    typename std::enable_if_t<
            !is_container<T>::value &&
            !std::is_base_of_v<Serializable, T> &&
            !std::is_same_v<std::nullptr_t &&, T> &&
            !std::is_pointer_v<T>, void>
    readSingle(T& value);

    // For read Serializable objects.
    LIBZMQUTILS_EXPORT void readSingle(Serializable& obj);

    // For read strings.
    LIBZMQUTILS_EXPORT void readSingle(std::string& str);

    // For arrays of trivial types.
    template<typename T, size_t L>
    void readSingle(std::array<T, L>& arr);

    // For vectors of trivial types.
    template<typename T>
    void readSingle(std::vector<T>& v);

    // -----------------------------------------------------------------------------------------------------------------

    // Internal containers and variables.
    BytesSmartPtr data_;                  ///< Internal data pointer.
    std::atomic<SizeUnit> size_;          ///< Current size of the data.
    std::atomic<SizeUnit> capacity_;      ///< Current capacity.
    std::atomic<SizeUnit> offset_;        ///< Offset when reading.
    Endianess endianess_;                 ///< Represent the endianess of the system.
    mutable std::mutex mtx_;              ///< Mutex for thread safety
};

}} // END NAMESPACES
// =====================================================================================================================

// TEMPLATES INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.tpp"
// =====================================================================================================================
