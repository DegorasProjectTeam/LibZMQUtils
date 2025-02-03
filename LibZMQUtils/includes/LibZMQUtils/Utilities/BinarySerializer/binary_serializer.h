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
#include <tuple>
#include <string>
#if __MINGW64_VERSION_MAJOR > 6
#include <filesystem>
#endif
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Global/libzmqutils_global.h"
// =====================================================================================================================

// Type trait to check if a type is a std::tuple
template<typename T>
struct is_tuple : std::false_type {};

template<typename... Args>
struct is_tuple<std::tuple<Args...>> : std::true_type {};

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace serializer{
// =====================================================================================================================

class BinarySerializer;

// Traits.
// ---------------------------------------------------------------------------------------------------------------------

template<typename... Ts>
struct trait_has_nullptr_t : std::disjunction<std::is_same<std::nullptr_t, Ts>...> {};

// ---------------------------------------------------------------------------------------------------------------------

using SizeUnit = std::uint64_t;               ///< Alias for the size unit.
using Bytes = std::byte[];                    ///< Type used for representing an array of std bytes,
using BytesDataPtr = std::unique_ptr<Bytes>;  ///< Type use for represent a unique pointer that contains bytes.

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
 * @see BinarySerializer
 */
class LIBZMQUTILS_EXPORT Serializable
{
public:

    Serializable() = default;

    Serializable(const Serializable&) = default;

    Serializable(Serializable&&) = default;

    Serializable& operator =(const Serializable&) = default;

    Serializable& operator =(Serializable&&) = default;

    /**
     * @brief Virtual destructor to allow proper cleanup of derived classes.
     */
    virtual ~Serializable();

    /**
     * @brief Method to serialize the object into a binary format.
     * @param serializer The BinarySerializer instance that will handle the serialization.
     * @return The size of the serialized data in bytes.
     */
    virtual SizeUnit serialize(BinarySerializer& serializer) const = 0;

    /**
     * @brief Method to deserialize the object from its binary representation.
     * @param serializer The BinarySerializer instance that will handle the deserialization.
     */
    virtual void deserialize(BinarySerializer& serializer) = 0;

    /**
     * @brief Returns the size of the object when serialized into binary format.
     * @return The size of the serialized object in bytes.
     */
    virtual SizeUnit serializedSize() const = 0;

protected:

    template<typename T, typename... Args>
    static SizeUnit calcSizeHelper(const T& value, const Args&... args);

};

// ---------------------------------------------------------------------------------------------------------------------

// TODO MOVE TO OTHER FILE

struct LIBZMQUTILS_EXPORT BinarySerializedData
{
    /**
     * @brief Default constructor for creating an empty serialized data.
     */
    BinarySerializedData();

    /**
     * @brief Checks if the serialized data is empty.
     * @return True if the struct no contains binary data or if the binary data size is zero, false otherwise.
     */
    bool isEmpty() const;

    /**
     * @brief Resets the serialized data to an empty state, clearing the internal buffer and setting size to zero.
     */
    void clear();

    // Struct data.
    BytesDataPtr bytes;  ///< Serialized request data parameters associated to the command as pointer of bytes.
    SizeUnit size;       ///< Total binary serialized data size.
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

    using BytesSmartPtr = std::unique_ptr<std::byte[]>;  ///< Alias for the bytes storage smart pointer.

    /// Enumeration representing the byte order (endianness) of data.
    enum class Endianess
    {
        LT_ENDIAN, ///< Little-endian byte order (LSB first).
        BG_ENDIAN     ///< Big-endian byte order (MSB first).
    };

    BinarySerializer(const BinarySerializer&) = delete;

    BinarySerializer(BinarySerializer&& other) :
        data_(std::move(other.data_)),
        size_(other.size_.load()),
        capacity_(other.capacity_.load()),
        offset_(other.offset_.load()),
        endianess_(std::move(other.endianess_))
    {}

    BinarySerializer& operator =(const Serializable&) = delete;

    BinarySerializer& operator =(BinarySerializer&&) = delete;

    /**
     * @brief Construct a new ´BinarySerializer´ object with a given capacity.
     * @param capacity The initial capacity of the serializer. Default is 1024.
     */
    BinarySerializer(SizeUnit capacity = 1024);

    /**
     * @brief Construct a new ´BinarySerializer´ object and load the given data.
     * @param src Pointer to the data source to load.
     * @param size Size of the data to load.
     * @warning The @a src parameter is a void pointer, so be careful.
     * @warning This constructor implies deep copy.
     */
    BinarySerializer(void* src, SizeUnit size);

    BinarySerializer(BytesSmartPtr&& src, SizeUnit size);


    /**
     * @brief Reserve memory for the serializer.
     * @param size The size of memory to reserve.
     * @warning This function implies deep copy if the class has data.
     */
    void reserve(SizeUnit size);

    /**
     * @brief Load data into the serializer.
     * @param data Pointer to the data to load.
     * @param size Size of the data to load.
     * @warning The @a data parameter is a void pointer, so be careful.
     * @warning This function implies deep copy.
     */
    void loadData(void *src, SizeUnit size);

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
    std::byte* release(SizeUnit& size);

    /**
     * @brief Move the unique pointer to the data held by the serializer and return it.
     * @param[out] out The smart pointer with the data.
     * @return The current size of the data.
     */
    SizeUnit moveUnique(BinarySerializer::BytesSmartPtr& out);

    /**
     * @brief Get the current size of the data held by the serializer.
     * @return The current size of the data.
     */
    SizeUnit getSize() const;

    /**
     * @brief Check whether all data has been read.
     * @return True if all data has been read, false otherwise.
     */
    bool allReaded() const;

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
     * @tparam Args Variadic template argument for types.
     * @param[out] out The unique pointer where the serialized data will be stored. The pointer will be allocated within the function.
     * @param[in] args The input data items to be serialized.
     * @return The size of the serialized data.
     */
    template<typename... Args>
    [[nodiscard]] static SizeUnit fastSerialization(BytesSmartPtr& out, const Args&... args);

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
    template<typename T, typename... Args,
        typename = std::enable_if_t<
            !is_tuple<T>::value &&
            !trait_has_nullptr_t<T, Args...>::value>>
    SizeUnit write(const T& value, const Args&... args);

    template<typename... Args>
    SizeUnit write(const std::tuple<Args...>& tup);

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
    SizeUnit writeFile(const std::string& in_filenamepath);

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
     * @return The filepath of the file read. It is a concatenation of out_filepath and file name. If file is empty
     *         the filepath is also empty.
     *
     * @throws std::out_of_range If there's not enough data left to read the size of the filename or file content.
     * @throws std::runtime_error If the file for can't be opened or if an empty filename is encountered.
     *
     * @note This function assumes that the serialized data in the internal buffer corresponds to a previously
     * serialized file created using the `writeFile` function.
     */
    std::string readFile(const std::string& out_filepath);

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
     * @note The types must meet the required conditions, such as being trivially copyable and trivial, directly
     * supported by this class, or being a subclass of the Serializable interface.
     */
    template<typename T, typename... Args>
    static SizeUnit serializedSize(const T& value, const Args&... args);

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
    static Endianess determineEndianess();

    // Internal function to check if the type is trivially copiable.
    template<typename T>
    static void checkTriviallyCopyable();

    // Internal function to check if the type is trivial.
    template<typename T>
    static void checkTrivial();

    // Internal binary serialization/deserialization helper function.
    template<typename T, typename C>
    static void binarySerializeDeserialize(const T* src, SizeUnit data_size_bytes, C* dst, bool reverse);

    // Internal binary serialization helper function.
    template<typename T, typename C>
    void binarySerialize(const T* src, SizeUnit data_size_bytes, C* dst);

    // Internal binary deserialization helper function.
    template<typename T, typename C>
    void binaryDeserialize(const T *src, SizeUnit data_size_bytes, C *dst);

    // Recursive size calculator helper.
    template<typename T, typename... Args>
    static SizeUnit serializedSizeRecursive(const T& value, const Args&... args);

    // Recursive write helper.
    template<typename T, typename... Args>
    void writeRecursive(const T& value, const Args&... args);

    // Recursive read helper.
    template<typename T, typename... Args>
    void readRecursive(T& value, Args&... args);

    // -----------------------------------------------------------------------------------------------------------------

    // Calc size functions.

    // Generic size calculator function.
    template<typename T>
    typename std::enable_if_t<
            !is_container<T>::value &&
            !std::is_same_v<std::nullptr_t &&, T> &&
            !std::is_pointer_v<T>, SizeUnit>
    static serializedSizeSingle(const T& data);

    // Size calculator function for vectors.
    template<typename T>
    static SizeUnit serializedSizeSingle(const std::vector<T>& data);

    // Size calculator function for vectors of vectors.
    template<typename T>
    static SizeUnit serializedSizeSingle(const std::vector<std::vector<T>>& data);

    // Size calculator function for arrays.
    template<typename T, size_t L>
    static SizeUnit serializedSizeSingle(const std::array<T, L>& data);

#if __MINGW64_VERSION_MAJOR > 6
    // Size calculator function for files.
    static SizeUnit serializedSizeSingle(const std::filesystem::path& data);
#endif

    // -----------------------------------------------------------------------------------------------------------------

    // Write data functions.

    // Generic writing.
    template<typename T>
    typename std::enable_if_t<
            !is_container<T>::value &&
            !std::is_base_of_v<Serializable, T> &&
            !std::is_same_v<std::nullptr_t &&, T> &&
#if __MINGW64_VERSION_MAJOR > 6
            !std::is_same_v<std::filesystem::path, T> &&
#endif
            !std::is_pointer_v<T>, void>
    writeSingle(const T& obj);

    // For writing Serializable objects.
    void writeSingle(const Serializable& obj);

    // For writing strings.
    void writeSingle(const std::string& str);

    // For arrays of trivial types.
    template<typename T, SizeUnit L>
    void writeSingle(const std::array<T, L>& arr);

    // For vectors of trivial types.
    template<typename T>
    void writeSingle(const std::vector<T>& v);

    // For vectors of vectors of trivial types.
    template<typename T>
    void writeSingle(const std::vector<std::vector<T>>& v);

#if __MINGW64_VERSION_MAJOR > 6
    // For write files using std::filesystem::path
    void writeSingle(const std::filesystem::path& file_path);
#endif

    // -----------------------------------------------------------------------------------------------------------------

    // Read data functions.

    // Generic reading.
    template<typename T>
    typename std::enable_if_t<
        !BinarySerializer::is_container<T>::value &&
        !std::is_base_of_v<Serializable, T> &&
        !std::is_same_v<std::nullptr_t &&, T> &&
#if __MINGW64_VERSION_MAJOR > 6
        !std::is_same_v<std::filesystem::path, T> &&
#endif
        !std::is_pointer_v<T>, void>
    readSingle(T& value);

    // For read Serializable objects.
    void readSingle(Serializable& obj);

    // For read strings.
    void readSingle(std::string& str);

#if __MINGW64_VERSION_MAJOR > 6
    // For read files using std::filesystem::path
    void readSingle(std::filesystem::path& file_path);
#endif

    // For arrays of trivial types.
    template<typename T, size_t L>
    void readSingle(std::array<T, L>& arr);

    // For vectors of trivial types.
    template<typename T>
    void readSingle(std::vector<T>& v);

    // For vector of vectors of trivial types.
    template<typename T>
    void readSingle(std::vector<std::vector<T>>& v);

    template<typename... Args>
    void readSingle(std::tuple<Args...>& tup);

    template<std::size_t I = 0, typename... Tp>
    typename std::enable_if<I == sizeof...(Tp), void>::type
    readTupleElements(std::tuple<Tp...>&);

    template<std::size_t I = 0, typename... Tp>
    typename std::enable_if<I < sizeof...(Tp), void>::type
    readTupleElements(std::tuple<Tp...>& t);

    // -----------------------------------------------------------------------------------------------------------------

    // Internal containers and variables.
    BytesSmartPtr data_;                  ///< Internal data pointer.
    std::atomic<SizeUnit> size_;          ///< Current size of the data.
    std::atomic<SizeUnit> capacity_;      ///< Current capacity.
    std::atomic<SizeUnit> offset_;        ///< Offset when reading.
    Endianess endianess_;                 ///< Represent the endianess of the system.
    mutable std::mutex mtx_;              ///< Mutex for thread safety

    // Specific class scope (for debug purposes).
    inline static const std::string kClassScope = "[LibDegorasBase,Serialization,BinarySerializer]";
};

}} // END NAMESPACES
// =====================================================================================================================

// TEMPLATES INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Utilities/BinarySerializer/binary_serializer.tpp"
// =====================================================================================================================
