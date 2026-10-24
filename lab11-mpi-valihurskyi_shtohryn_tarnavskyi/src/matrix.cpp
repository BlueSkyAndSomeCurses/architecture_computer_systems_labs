#include "matrix.hpp"

#include <iostream>

Matrix::Matrix(const size_t r, const size_t c) : rows {r}, cols {c} {
    matrix = new double[r * c];
    std::fill_n(matrix, r * c, 0);
}

Matrix::Matrix()
  : rows(0),
    cols(0),
    matrix(nullptr)
{} 

Matrix::Matrix(const Matrix& other)
    : rows{ other.get_rows() }, cols{ other.get_columns() } {

    matrix = new double[rows * cols];

    std::copy(other.get_matrix(),
              other.get_matrix() + rows * cols,
              matrix);
}

Matrix::~Matrix() {
    delete[] matrix;
}

Matrix& Matrix::operator=(const Matrix& other) {
    if (this != &other) {

        delete[] matrix;

        rows = other.rows;
        cols = other.cols;

        matrix = new double[rows * cols];
        std::copy(other.matrix,
                  other.matrix + rows * cols,
                  matrix);
    }
    return *this;
}

double& Matrix::operator()(size_t r, size_t c) {

    if (r < rows && c < cols)
        return matrix[r * cols + c];

    throw std::out_of_range("Matrix index out of range");
    
}

double Matrix::operator()(const size_t r, const size_t c) const {

    if (r < rows && c < cols)
        return matrix[r * cols + c];

    throw std::out_of_range("Matrix index out of range");
}

double* Matrix::operator[](const size_t r) const {

    if (r < rows) {
        return matrix + r * cols;
    }

    throw std::out_of_range("Index out of range");
}

size_t Matrix::size() {
    return rows;
}

size_t Matrix::get_rows() const{
    return rows;
}

size_t Matrix::get_columns() const{
    return cols;
}

double* Matrix::get_matrix() const{
    return matrix;
}

double Matrix::get_matrix_copy() {
    return *matrix;
}
