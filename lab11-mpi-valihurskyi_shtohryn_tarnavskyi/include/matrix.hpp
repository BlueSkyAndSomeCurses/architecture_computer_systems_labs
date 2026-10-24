#pragma once

#ifndef MATRIX_H
#define MATRIX_H

#include <cstddef>
#include <algorithm>


class Matrix {
public:
  size_t rows;
  size_t cols;
  double *matrix;

  Matrix(const size_t r, const size_t c);

  Matrix(const Matrix &other);


  Matrix();

  ~Matrix();

  Matrix &operator=(const Matrix &other);

  double &operator()(const size_t r, const size_t c);

  double operator()(const size_t r, const size_t c) const;

  double *operator[](const size_t r) const;

  size_t size();

  size_t get_rows() const;

  size_t get_columns() const;

  double *get_matrix() const;

  double get_matrix_copy();
};

#endif
