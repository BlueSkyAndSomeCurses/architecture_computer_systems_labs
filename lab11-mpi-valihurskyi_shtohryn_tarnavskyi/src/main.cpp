#include<iostream>

#include "plate_handler.hpp"
#include "check_file_reading.hpp"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Please specify the config_file_path" << std::endl;
    return 1;
  }

  std::string config_file = argv[1];

  auto [info_pair, start_temperatures_file] = check_file_reading(config_file);

  auto [x_y_steps, doubles] = info_pair;

  double t_step = doubles[0], t_snapshot = doubles[1],
    alpha = doubles[2], total_time = doubles[3];

  int x_step = x_y_steps[0];
  int y_step = x_y_steps[1];

  auto starting_points = get_start_points(start_temperatures_file);


  PlateHandler plateHandler = PlateHandler(x_step, y_step, t_step, t_snapshot, alpha, total_time, starting_points);


  auto loc_time = plateHandler.get_result(argc, argv);

  unsigned long long local = static_cast<unsigned long long>(loc_time);
  unsigned long long global_max;

  MPI_Reduce(&local, &global_max, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX, 0, MPI_COMM_WORLD);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    std::cout << "Total=" << global_max << std::endl;
  }

  return 0;
}
