#pragma once
#ifndef PLATE_HANDLER_H
#define PLATE_HANDLER_H

#include<iostream>
#include<vector>
#include <cmath>
#include <unordered_map>
#include <boost/mpi.hpp>
#include <boost/functional/hash.hpp>
#include <boost/serialization/vector.hpp>

#include <cstddef>

#include "matrix.hpp"

namespace mpi = boost::mpi;

class PlateHandler {
public:
  PlateHandler(
    int x_step,
    int y_step,
    double t_step,
    double t_snapshot,
    double alpha,
    double full_time,

    const Matrix &start_points
  );

  size_t get_result(int argc, char *argv[]);

  void send_field_info();

  double get_point_temp(int i, int j) const;

  std::pair<int, int> get_limits(int rank);

  void iterate_over_time();

  void update_corner_temperature();

  void update_row_temperature(int row_idx);

  void update_inner_temperature();

  void exchange_info();

  void pass_messages();

  void update_buffer();

  void send_forward(int rank, const int tag);

  void send_back(int rank, const int tag);

  void make_snapshot();

private :
  int x_step;
  int y_step;
  double t_step;
  double t_snapshot;
  int heigh;
  int width;
  double alpha;


  double full_time;
  Matrix field;
  Matrix buff_field;
  int lower_limit;
  int upper_limit;
  int snap_shoter_rank;

  std::unique_ptr<mpi::environment> env;
  std::unique_ptr<mpi::communicator> world;
};

#endif
