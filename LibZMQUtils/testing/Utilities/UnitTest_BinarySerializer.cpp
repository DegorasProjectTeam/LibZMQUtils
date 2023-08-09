
#include <cassert>
#include <iostream>

#include <LibDPSLR/Mathematics/Matrix>

using dpslr::math::Matrix;

void testMatrixIdentity()
{
    // Test identity property.
    Matrix<unsigned> identity2 = Matrix<unsigned>::I(2);
    Matrix<unsigned> identity3 = Matrix<unsigned>::I(3);
    Matrix<unsigned> identity4 = Matrix<unsigned>::I(4);
    Matrix<unsigned> identity5 = Matrix<unsigned>::I(5);

    // Check the identity matrices.
    bool is_iden_2 = identity2.isIdentity();
    bool is_iden_3 = identity3.isIdentity();
    bool is_iden_4 = identity4.isIdentity();
    bool is_iden_5 = identity5.isIdentity();

    // Output.
    std::cout<<"---------------------------------"<<std::endl;
    std::cout<<"testMatrixIdentity"<<std::endl<<std::endl;
    std::cout<<identity2.toString()<<std::endl;
    std::cout<<identity3.toString()<<std::endl;
    std::cout<<identity4.toString()<<std::endl;
    std::cout<<identity5.toString()<<std::endl;
    std::cout<<"---------------------------------"<<std::endl<<std::endl;

    // Asserts.
    assert(is_iden_2);
    assert(is_iden_3);
    assert(is_iden_4);
    assert(is_iden_5);
}

void testMatrixTranspose()
{
    // Create a matrix for testing
    Matrix<long double> matrix({{1.1, 2.2, 3.3},{4.4, 5.5, 6.6},{7.7, 8.8, 9.9},{10, 11, 12}});

    // Transpose the matrix
    auto transposed = matrix.transpose();

    // Output.
    std::cout<<"---------------------------------"<<std::endl;
    std::cout<<"testMatrixTranspose"<<std::endl<<std::endl;
    std::cout<<matrix.toString()<<std::endl;
    std::cout<<transposed.toString()<<std::endl;
    std::cout<<"---------------------------------"<<std::endl<<std::endl;

    // Check the dimensions
    assert(transposed.rowSize() == matrix.columnsSize());
    assert(transposed.columnsSize() == matrix.rowSize());

    // Check the values
    assert(transposed[0][0] == 1.1);
    assert(transposed[1][0] == 2.2);
    assert(transposed[2][0] == 3.3);
    assert(transposed[0][1] == 4.4);
    assert(transposed[1][1] == 5.5);
    assert(transposed[2][1] == 6.6);
    assert(transposed[0][2] == 7.7);
    assert(transposed[1][2] == 8.8);
    assert(transposed[2][2] == 9.9);
}

void testMatrixMultiplication()
{
    // Create matrices for testing
    Matrix<long double> mat1({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}});
    Matrix<long double> mat2({{2, 4, 6}, {1, 3, 5}, {8, 9, 7}});
    Matrix<long double> mat3({{1, 2, 3}, {4, 5, 6}});
    Matrix<long double> mat4({{6, 5, 12, 8}, {14, 13, 26, 16}, {22, 21, 40, 24.8}});
    Matrix<long double> bad1({{1, 2}, {3, 4, 5, 6}});
    Matrix<long double> scalar_m({{2, 0, 0}, {0, 2, 0}, {0, 0, 2}});

    // Perform matrix multiplication with scalar
    Matrix<long double> result1 = mat1 * 2;
    Matrix<long double> expected1({{2, 4, 6}, {8, 10, 12}, {14, 16, 18}});

    // Perform matrix multiplication with another matrix
    Matrix<long double> result2 = mat1 * mat2;
    Matrix<long double> expected2({{28, 37, 37}, {61, 85, 91}, {94, 133, 145}});

    // Perform matrix multiplication with scalar matrix
    Matrix<long double> result3 = mat1 * scalar_m;
    Matrix<long double> expected3({{2, 4, 6}, {8, 10, 12}, {14, 16, 18}});

    Matrix<long double> result4 = mat3 * mat4;
    Matrix<long double> expected4({{100, 94, 184, 114.4}, {226, 211, 418, 260.8}});

    // Invalid multiplication.
    Matrix<long double> result5 = bad1 * mat1;

    // Output.
    std::cout<<"---------------------------------"<<std::endl;
    std::cout<<"testMatrixMultiplication"<<std::endl<<std::endl;
    std::cout<<mat1.toString()<<std::endl;
    std::cout<<mat2.toString()<<std::endl;
    std::cout<<scalar_m.toString()<<std::endl;
    std::cout<<"mat1*2"<<std::endl<<std::endl;
    std::cout<<result1.toString()<<std::endl;
    std::cout<<"mat1*mat2"<<std::endl<<std::endl;
    std::cout<<result2.toString()<<std::endl;
    std::cout<<"mat1*scalar"<<std::endl<<std::endl;
    std::cout<<result3.toString()<<std::endl;
    std::cout<<"mat3*mat4"<<std::endl<<std::endl;
    std::cout<<result4.toString()<<std::endl;
    std::cout<<"---------------------------------"<<std::endl<<std::endl;

    // Asserts.
    assert(result1 == expected1);
    assert(result2 == expected2);
    assert(result3 == expected3);
    assert(result4 == expected4);
    assert(result5.isEmpty());
}

void testMatrixInverse()
{
    Matrix<long double> matrix(3, 3, 0);

    matrix(0, 0) = 1;
    matrix(0, 1) = 2;
    matrix(0, 2) = 3;
    matrix(1, 0) = 0;
    matrix(1, 1) = 0;
    matrix(1, 2) = 1;
    matrix(2, 0) = 0;
    matrix(2, 1) = 4;
    matrix(2, 2) = 0;


    Matrix<long double> inv = matrix.inverse();

    std::cout<<matrix.toString()<<std::endl;
    std::cout<<inv.toString()<<std::endl;


    assert(inv(0, 0) == -5.0);
    assert(inv(0, 1) == 3.0);
    assert(inv(1, 0) == 4.0);
    assert(inv(1, 1) == -2.0);
}

int main()
{
    std::cout << "Unit Test: Class dpslr::math::Matrix" << std::endl;

    testMatrixIdentity();
    testMatrixTranspose();
    testMatrixMultiplication();
    testMatrixInverse();

    // All assertions passed
    std::cout << "All tests passed!\n";

    return 0;
}
