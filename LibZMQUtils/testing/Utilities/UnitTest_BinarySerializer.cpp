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

// C++ INCLUDES
// =====================================================================================================================
#include <iostream>
#include <vector>
#include <fstream>
#include <stdio.h>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include <LibZMQUtils/Utils>
#include <LibZMQUtils/Testing/UnitTest>
// =====================================================================================================================

// =====================================================================================================================
using zmqutils::utils::BinarySerializer;
using zmqutils::utils::Serializable;
// =====================================================================================================================

// Basic tests.
M_DECLARE_UNIT_TEST(BinarySerializer, Trivial)
M_DECLARE_UNIT_TEST(BinarySerializer, String)
M_DECLARE_UNIT_TEST(BinarySerializer, ArrayTrivial)
M_DECLARE_UNIT_TEST(BinarySerializer, VectorTrivial)
M_DECLARE_UNIT_TEST(BinarySerializer, Serializable)
M_DECLARE_UNIT_TEST(BinarySerializer, File)

// Other tests.
M_DECLARE_UNIT_TEST(BinarySerializer, TrivialIntensive)

// Implementations.

M_DEFINE_UNIT_TEST(BinarySerializer, Trivial)
{
    // Serializer.
    BinarySerializer serializer;

    // Hex result.
    const std::string result("00 00 00 00 00 00 00 08 41 13 1e 76 5c d4 66 f5 00 00 00 00 00 00 00 08 c0 8e c2 c5 "
                             "33 38 3b b1 00 00 00 00 00 00 00 04 ff ff ff de 00 00 00 00 00 00 00 04 00 00 00 05");

    // Data.
    const double n1 = 313245.590654;
    const double n2 = -984.3462891;
    const int n3 = -34;
    const unsigned n4 = 5;
    double r1, r2;
    int r3;
    unsigned r4;

    // Write, read.
    serializer.write(n1, n2, n3);
    serializer.write(n4);
    serializer.read(r1);
    serializer.read(r2, r3, r4);

    // Checking.
    M_EXPECTED_EQ(serializer.getDataHexString(), result)
    M_EXPECTED_EQ(serializer.getSize(), sizeof(double)*2 + sizeof(int) + sizeof(unsigned)
                                            + 4*sizeof(BinarySerializer::SizeUnit))
    M_EXPECTED_EQ(r1, n1)
    M_EXPECTED_EQ(r2, n2)
    M_EXPECTED_EQ(r3, n3)
    M_EXPECTED_EQ(r4, n4)

    // Clear test.
    serializer.clearData();
    M_EXPECTED_EQ(serializer.getSize(), size_t(0))

    // Clear out variables.
    r1 = r2 = 0.0L;
    r3 = r4 = 0;

    // Fast deserialization test.
    serializer.write(n1, n2, n3);
    serializer.write(n4);
    std::size_t size;
    std::byte* bytes = serializer.release(size);
    BinarySerializer::fastDeserialization(bytes, size, r1, r2, r3, r4);

    // Checking.
    M_EXPECTED_EQ(r1, n1)
    M_EXPECTED_EQ(r2, n2)
    M_EXPECTED_EQ(r3, n3)
    M_EXPECTED_EQ(r4, n4)
}

M_DEFINE_UNIT_TEST(BinarySerializer, String)
{
    // Serializer.
    BinarySerializer serializer;

    // Hex result.
    const std::string result("00 00 00 00 00 00 00 1b 2e 2e 2e 61 68 63 6e 61 4d 20 61 6c 20 65 64 20 72 61 67 75 6c "
                             "20 6e 75 20 6e 45 00 00 00 00 00 00 00 04 20 20 20 20 00 00 00 00 00 00 00 12 31 32 33 "
                             "2e 2e 2e 67 6e 69 72 74 73 2e 2e 2e 33 32 31 00 00 00 00 00 00 00 00");

    // Data.
    const std::string in1("En un lugar de la Mancha...");
    const std::string in2("    ");
    const std::string in3("123...string...321");
    const std::string in4("");
    std::string out1, out2, out3, out4;
    size_t size = in1.size() + in2.size() + in3.size() + in4.size() + sizeof(BinarySerializer::SizeUnit)*4;

    // Write, read.
    serializer.write(in1, in2, in3, in4);
    serializer.read(out1, out2, out3, out4);

    // Checking.
    M_EXPECTED_EQ(serializer.getDataHexString(), result)
    M_EXPECTED_EQ(serializer.getSize(), size)
    M_EXPECTED_EQ(in1, out1)
    M_EXPECTED_EQ(in2, out2)
    M_EXPECTED_EQ(in3, out3)
    M_EXPECTED_EQ(in4, out4)

    // Clear test.
    serializer.clearData();
    M_EXPECTED_EQ(serializer.getSize(), size_t(0))

    // Clear out variables.
    out1 = out2 = out3 = out4 = "";

    // Fast deserialization test.
    serializer.write(in1, in2, in3, in4);
    std::byte* bytes = serializer.release(size);
    BinarySerializer::fastDeserialization(bytes, size, out1, out2, out3, out4);

    // Checking.
    M_EXPECTED_EQ(in1, out1)
    M_EXPECTED_EQ(in2, out2)
    M_EXPECTED_EQ(in3, out3)
    M_EXPECTED_EQ(in4, out4)
}

M_DEFINE_UNIT_TEST(BinarySerializer, ArrayTrivial)
{
    // Serializer.
    BinarySerializer serializer;

    // Hex result.
    const std::string result("00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 01 12 34 56 78 90 ab cd ef fe dc ba 98 76 "
                             "54 32 10 00 00 00 00 00 00 00 14 00 00 00 00 00 00 00 08 c0 00 00 00 00 00 00 00 bf f0 "
                             "00 00 00 00 00 00 00 00 00 00 00 00 00 00 3f f1 99 99 99 99 99 9a 40 01 99 99 99 99 99 "
                             "9a 40 0a 66 66 66 66 66 66 40 10 00 00 00 00 00 00 40 14 00 00 00 00 00 00 40 18 00 00 "
                             "00 00 00 00 40 1c 00 00 00 00 00 00 40 20 00 00 00 00 00 00 40 22 00 00 00 00 00 00 40 "
                             "24 00 00 00 00 00 00 40 26 00 00 00 00 00 00 40 28 00 00 00 00 00 00 c0 2a 00 00 00 00 "
                             "00 00 c0 2c 00 00 00 00 00 00 c0 2e 00 00 00 00 00 00 c0 30 00 00 00 00 00 00 40 34 00 "
                             "00 00 00 00 00");

    // Data.
    const std::array<std::byte, 16> uuid = {
        std::byte(0x12), std::byte(0x34), std::byte(0x56), std::byte(0x78),
        std::byte(0x90), std::byte(0xAB), std::byte(0xCD), std::byte(0xEF),
        std::byte(0xFE), std::byte(0xDC), std::byte(0xBA), std::byte(0x98),
        std::byte(0x76), std::byte(0x54), std::byte(0x32), std::byte(0x10)};
    const std::array<double, 20> arr = {-2, -1, 0, 1.1, 2.2, 3.3, 4, 5, 6, 7 ,8 ,9 ,10, 11, 12, -13, -14, -15, -16, 20};
    std::array<std::byte, 16> r_uuid;
    std::array<double, 20> r_arr;

    // Write, read.
    serializer.write(uuid, arr);
    serializer.read(r_uuid, r_arr);

    // Checking.
    M_EXPECTED_EQ(serializer.getDataHexString(), result)
    M_EXPECTED_EQ(uuid, r_uuid)
    M_EXPECTED_EQ(arr, r_arr)
}

M_DEFINE_UNIT_TEST(BinarySerializer, VectorTrivial)
{
    // Serializer.
    BinarySerializer serializer;

    // Data.
    const std::vector<long double> v1 = {34.32315L, -423423785.434334534242L, 23.34L, -876.3L, 12345L};
    const std::vector<int> v2 = {-2, -1, 0, 1, 2, 3, 4, 5, 6, 7 ,8 ,9 ,10, 11, 12, -13, -14, -15, -16, 20};
    std::vector<long double> r1;
    std::vector<int> r2;
    size_t size1 = v1.size()*sizeof(long double) + sizeof(BinarySerializer::SizeUnit)*2;
    size_t size2 = v2.size()*sizeof(int) + sizeof(BinarySerializer::SizeUnit)*2;

    // Write, read.
    serializer.write(v1);
    serializer.write(v2);
    serializer.read(r1);
    serializer.read(r2);

    // Checking.
    M_EXPECTED_EQ(size1 + size2, serializer.getSize())
    M_EXPECTED_EQ(v1, r1)
    M_EXPECTED_EQ(v2, r2)
}

M_DEFINE_UNIT_TEST(BinarySerializer, Serializable)
{
    class TestSer : public Serializable
    {
    public:

        TestSer() = default;

        TestSer(double number, const std::string& str):
            number_(number), str_(str){}

        size_t serialize(zmqutils::utils::BinarySerializer& serializer) const final
        {
            return serializer.write(this->number_, this->str_);
        }

        void deserialize(zmqutils::utils::BinarySerializer& serializer) final
        {
            serializer.read(this->number_, this->str_);
        }

        size_t serializedSize() const final
        {
            return (sizeof(uint64_t)*2 + sizeof(double) + this->str_.length());
        }

        inline bool operator ==(const TestSer& other) const
        {
            static constexpr double epsilon = 1e-9;
            if (std::abs(number_ - other.number_) > epsilon) return false;
            return str_ == other.str_;
        }

    private:
        double number_;
        std::string str_;
    };

    // Serializer.
    BinarySerializer serializer;

    // Hex result.
    std::string result("00 00 00 00 00 00 00 08 c0 7c b5 58 e2 19 65 2c 00 00 00 00 00 00 00 1e 2e 2e 2e "
                       "6f 67 6e 65 76 20 6f 64 6e 61 6c 6f 76 20 79 20 79 6f 76 20 6f 64 6e 61 6c 6f 56");

    // Data.
    TestSer test_in(-459.3342, "Volando voy y volando vengo...");
    TestSer test_out;

    // Write, read.
    serializer.write(test_in);
    serializer.read(test_out);

    // Checking.
    M_EXPECTED_EQ(test_in, test_out)
    M_EXPECTED_EQ(result, serializer.getDataHexString())
}

M_DEFINE_UNIT_TEST(BinarySerializer, File)
{
    // Serializer.
    BinarySerializer serializer;

    // Hex result.
    const std::string result("00 00 00 00 00 00 00 12 74 78 74 2e 65 6c 69 66 5f 74 73 65 74 5f 70 6d 65 74 00 00 00 "
                             "00 00 00 00 3f 48 65 6c 6c 6f 20 64 61 72 6b 6e 65 73 73 20 6d 79 20 6f 6c 64 20 66 72 "
                             "69 65 6e 64 21 0a 49 27 76 65 20 63 6f 6d 65 20 74 6f 20 74 61 6c 6b 20 77 69 74 68 20 "
                             "79 6f 75 20 61 67 61 69 6e 2e");

    // Create a temporary file with some content.
    std::string filename = "temp_test_file.txt";
    std::string file_content = "Hello darkness my old friend!\nI've come to talk with you again.";
    std::ofstream temp_file(filename, std::ios::binary);
    temp_file.write(file_content.c_str(), static_cast<long long>(file_content.size()));
    temp_file.close();

    // Serialize the file.
    size_t ser_size = serializer.writeFile(filename);

    // Delete the file.
    remove(filename.c_str());

    // Deserialize the file.
    serializer.readFile("");

    // Open the file.
    std::ifstream output(filename);

    // Check the file.
    if(!output.is_open())
    {
        M_FORCE_FAIL()
        return;
    }

    // Read the deserialized content from the output stream.
    std::string deserialized_content((std::istreambuf_iterator<char>(output)),
                                     std::istreambuf_iterator<char>());

    // Verify the results.
    M_EXPECTED_EQ(serializer.getDataHexString(), result)
    M_EXPECTED_EQ(ser_size, sizeof(BinarySerializer::SizeUnit)*2 + file_content.size() + filename.size())
    M_EXPECTED_EQ(deserialized_content, file_content)

    // Delete the file.
    output.close();
    remove(filename.c_str());
}



M_DEFINE_UNIT_TEST(BinarySerializer, TrivialIntensive)
{
    // WARNING: This test is not efficient on purpose.

    BinarySerializer serializer;

    const size_t count = 2000;
    std::vector<double> original_numbers(count);
    std::vector<double> deserialized_numbers(count);

    // Fill the original numbers with random values
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1000000.0, 1000000.0);
    for (double &num : original_numbers)
        num = dis(gen);

    // Serialize all the numbers
    for (double num : original_numbers)
        serializer.write(num);

    // Deserialize the numbers
    for (double &num : deserialized_numbers)
        serializer.read(num);

    // Check that the deserialized numbers match the original
    for (size_t i = 0; i < count; i++) {
        M_EXPECTED_EQ(deserialized_numbers[i], original_numbers[i])
    }
}

int main()
{
    // Start of the session.
    M_START_UNIT_TEST_SESSION("LibZMQUtils BinarySerializer Session")

    // Register the tests.
    M_REGISTER_UNIT_TEST(BinarySerializer, Trivial)
    M_REGISTER_UNIT_TEST(BinarySerializer, String)
    M_REGISTER_UNIT_TEST(BinarySerializer, ArrayTrivial)
    M_REGISTER_UNIT_TEST(BinarySerializer, VectorTrivial)
    M_REGISTER_UNIT_TEST(BinarySerializer, Serializable)
    M_REGISTER_UNIT_TEST(BinarySerializer, File)
    M_REGISTER_UNIT_TEST(BinarySerializer, TrivialIntensive)

    // Run the unit tests.
    M_RUN_UNIT_TESTS()
}
