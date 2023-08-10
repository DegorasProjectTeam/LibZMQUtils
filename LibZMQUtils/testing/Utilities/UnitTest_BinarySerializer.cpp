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
M_DECLARE_UNIT_TEST(BinarySerializer, VectorTrivial)
M_DECLARE_UNIT_TEST(BinarySerializer, Serializable)

// Other tests.

M_DECLARE_UNIT_TEST(BinarySerializer, TrivialIntensive)

// Implementations.

M_DEFINE_UNIT_TEST(BinarySerializer, Trivial)
{
    BinarySerializer serializer;
    std::string result("00 00 00 00 00 00 00 08 41 13 1e 76 5c d4 66 f5 00 00 00 00 00 00 00 08 c0 8e c2 c5 "
                       "33 38 3b b1 00 00 00 00 00 00 00 04 ff ff ff de 00 00 00 00 00 00 00 04 00 00 00 05");

    double n1, n2;
    int n3;
    unsigned n4;
    double r1, r2;
    int r3;
    unsigned r4;

    n1 = 313245.590654;
    n2 = -984.3462891;
    n3 = -34;
    n4 = 5;

    serializer.write(n1, n2, n3);
    serializer.write(n4);
    serializer.read(r1);
    serializer.read(r2, r3, r4);

    //M_EXPECTED_EQ(serializer.getDataHexString(), result)
    M_EXPECTED_EQ(serializer.getSize(), sizeof(double)*2 + sizeof(int) + sizeof(unsigned) + 4*sizeof(uint64_t))
    M_EXPECTED_EQ(r1, n1)
    M_EXPECTED_EQ(r2, n2)
    M_EXPECTED_EQ(r3, n3)
    M_EXPECTED_EQ(r4, n4)
}

M_DEFINE_UNIT_TEST(BinarySerializer, String)
{
    BinarySerializer serializer;
    std::string result("00 00 00 00 00 00 00 1b 2e 2e 2e 61 68 63 6e 61 4d 20 61 6c 20 65 64 20 72 61 67 75 6c 20 6e "
                       "75 20 6e 45 00 00 00 00 00 00 00 04 20 20 20 20 00 00 00 00 00 00 00 12 31 32 33 2e 2e 2e 67 "
                       "6e 69 72 74 73 2e 2e 2e 33 32 31 00 00 00 00 00 00 00 00");
    std::string in1("En un lugar de la Mancha...");
    std::string in2("    ");
    std::string in3("123...string...321");
    std::string in4("");
    std::string out1, out2, out3, out4;
    size_t size = in1.size() + in2.size() + in3.size() + in4.size() + sizeof(uint64_t)*4;

    serializer.write(in1, in2, in3, in4);
    serializer.read(out1, out2, out3, out4);

    M_EXPECTED_EQ(serializer.getDataHexString(), result)
    M_EXPECTED_EQ(serializer.getSize(), size)
    M_EXPECTED_EQ(in1, out1)
    M_EXPECTED_EQ(in2, out2)
    M_EXPECTED_EQ(in3, out3)
    M_EXPECTED_EQ(in4, out4)
}

M_DEFINE_UNIT_TEST(BinarySerializer, VectorTrivial)
{
    BinarySerializer serializer;
    std::string result("40 40 c0 00 00 00 00 00 40 8e c4 00 00 00 00 00");

    std::vector<long double> v1 = {34.32315L, 45L, 23.34L, -876.3L, 12345L};
    std::vector<long double> r1;

    //serializer.write(v1);
    //serializer.read(r1);


    M_EXPECTED_EQ(serializer.getDataHexString(), result)
    M_EXPECTED_EQ(v1, r1)
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

        bool operator ==(const TestSer& other) const
        {
            std::cout << "HEREEEE" << std::endl;
            static constexpr double epsilon = 1e-9;
            if (std::abs(number_ - other.number_) > epsilon) return false;
            return str_ == other.str_;
        }

    private:
        double number_;
        std::string str_;
    };

    BinarySerializer serializer;
    std::string result("00 00 00 00 00 00 00 08 c0 7c b5 58 e2 19 65 2c 00 00 00 00 00 00 00 1e 2e 2e 2e "
                       "6f 67 6e 65 76 20 6f 64 6e 61 6c 6f 76 20 79 20 79 6f 76 20 6f 64 6e 61 6c 6f 56");

    TestSer test_in(-459.3342, "Volando voy y volando vengo...");
    TestSer test_out;

    serializer.write(test_in);
    serializer.read(test_out);

    M_EXPECTED_EQ(test_in, test_out)
    M_EXPECTED_EQ(result, serializer.getDataHexString())
}

M_DEFINE_UNIT_TEST(BinarySerializer, TrivialIntensive)
{
    // WARNING: This test is not efficient on purpose.

    BinarySerializer serializer;

    const size_t count = 1000;
    std::vector<double> original_numbers(count);
    std::vector<double> deserialized_numbers(count);

    // Fill the original numbers with random values
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1000000.0, 1000000.0);
    for (double &num : original_numbers) {
        num = dis(gen);
    }

    // Serialize all the numbers
    for (double num : original_numbers) {
        serializer.write(num);
    }

    // Deserialize the numbers
    for (double &num : deserialized_numbers) {
        serializer.read(num);
    }

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

    M_REGISTER_UNIT_TEST(BinarySerializer, Serializable)

    M_REGISTER_UNIT_TEST(BinarySerializer, TrivialIntensive)


    // Run the unit tests.
    M_RUN_UNIT_TESTS()
}
