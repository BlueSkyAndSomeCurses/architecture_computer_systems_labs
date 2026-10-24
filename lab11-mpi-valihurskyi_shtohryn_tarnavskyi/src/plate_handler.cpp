#include"plate_handler.hpp"
#include <Magick++.h>
#include "timer.h"


void create_heatmap_image(const Matrix &matrix, int snapshot_idx) {
  int w = matrix.get_columns();
  int h = matrix.get_rows();

  static constexpr double Vmin = 0.0;
  static constexpr double Vmax = 100.0;
  const double invRange = 1.0 / (Vmax - Vmin);

  Magick::Image image(Magick::Geometry(w, h), Magick::ColorRGB(0, 0, 0));
  image.type(Magick::TrueColorType);

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      double v = std::clamp(matrix(y, x), Vmin, Vmax);
      double t = (v - Vmin) * invRange;

      double r, g, b;

      if (t <= 0.25) {
        t = t / 0.25;
        r = 0.0;
        g = t;
        b = 1.0;
      } else if (t <= 0.5) {
        t = (t - 0.25) / 0.25;
        r = 0.0;
        g = 1.0;
        b = 1.0 - t;
      } else if (t <= 0.75) {
        t = (t - 0.5) / 0.25;
        r = t;
        g = 1.0;
        b = 0.0;
      } else {
        t = (t - 0.75) / 0.25;
        r = 1.0;
        g = 1.0 - t;
        b = 0.0;
      }

      image.pixelColor(x, h - y - 1, Magick::ColorRGB(r, g, b));
    }
  }

  std::string filename = "../data/pictures/snapshot_" + std::to_string(snapshot_idx) + ".png";
  image.write(filename);
}


PlateHandler::PlateHandler(
  int x_step,
  int y_step,
  double t_step,
  double t_snapshot,
  double alpha,
  double full_time,

  const Matrix &start_points) : x_step{x_step}, y_step{y_step}, t_step{t_step}, t_snapshot{t_snapshot},
                                alpha{alpha}, full_time{full_time} {
  heigh = start_points.get_rows();
  width = start_points.get_columns();

  field = start_points;
  buff_field = start_points;
};

size_t PlateHandler::get_result(int argc, char *argv[]) {

  auto start_t = get_current_time_fenced();
  env = std::make_unique<mpi::environment>(argc, argv);
  world = std::make_unique<mpi::communicator>();
  snap_shoter_rank = world->size() - 1;

  if (world->rank() == snap_shoter_rank) {
    make_snapshot();
    return 0;
  }

  auto limits = get_limits(world->rank());
  lower_limit = limits.first;
  upper_limit = limits.second;
#ifdef DEBUG
  std::cout << world->rank() << ' ' << lower_limit << ' ' << upper_limit << std::endl;
#endif


  iterate_over_time();

  auto end_t = get_current_time_fenced();

  return get_time(start_t, end_t);
}

std::pair<int, int> PlateHandler::get_limits(int rank) {
  int total_procs = world->size() - 1;

  int temp_height = heigh - 2;

  int chunk_size = temp_height / total_procs;


  int lower_limit = chunk_size * rank + 1;
  if (rank == 0) {
    --lower_limit;
  }

  int upper_limit;
  if (rank == total_procs - 1) {
    upper_limit = field.rows - 1;
  } else {
    upper_limit = chunk_size * rank + chunk_size;
  }

  return {lower_limit, upper_limit};
}

void PlateHandler::iterate_over_time() {
  int step_number = static_cast<int>(full_time / t_step);
  int snapshot_period = static_cast<int>(t_snapshot / t_step);

  for (int time = 0; time < step_number; time++) {
    pass_messages();
    exchange_info();
    update_inner_temperature();
    update_corner_temperature();
    update_buffer();

    if (time % snapshot_period == 0) {
      send_field_info();
    }
  }
}


void PlateHandler::update_corner_temperature() {
  int rank = world->rank();

  if (world->size() == 2) {
    return;
  }

  if (rank == 0) {
    update_row_temperature(upper_limit);
  }  if (rank == world->size() - 2) {
    update_row_temperature(lower_limit);
  } if (rank != 0 && rank != world->size() - 2) {
    update_row_temperature(upper_limit);
    update_row_temperature(lower_limit);
  }
}

void PlateHandler::update_inner_temperature() {
  int it_lower_limit = lower_limit + 1;
  int it_upper_limit = upper_limit - 1;
#ifdef  DEBUG
  std::cout << "iner temperature Rank" << world->rank() << ' ' << it_lower_limit << ' ' << it_upper_limit << std::endl;
#endif


  for (int i = it_lower_limit; i <= it_upper_limit; i++) {
    update_row_temperature(i);
  }
#ifdef DEBUG
  std::cout << "inner temperature Rank" << world->rank() << " success" << std::endl;
#endif
}

void PlateHandler::update_row_temperature(int row_idx) {
  for (int j = 1; j < width - 1; ++j) {
    field(row_idx, j) = get_point_temp(row_idx, j);
  }
}

void PlateHandler::pass_messages() {
  int rank = world->rank();
  constexpr int low_tag = 1;
  constexpr int up_tag = 2;

  if (world->size() == 2) {
    return;
  }

  if (rank == 0) {
    send_forward(rank, low_tag);
  } else if (rank == world->size() - 2) {
    send_back(rank, up_tag);
  } else {
    send_back(rank, up_tag);
    send_forward(rank, low_tag);
  }

}

void PlateHandler::exchange_info() {
  int rank = world->rank();
  constexpr int low_tag = 1;
  constexpr int up_tag = 2;

  if (world->size() == 2) {
    return;
  }

  int count = static_cast<int>(field.get_columns()) - 2;
  std::vector<double> recieved_row(count);


  if (rank == 0) {
    world->recv<double>(rank + 1, up_tag, recieved_row.data(), count);

    for (int i = 1; i < count; ++i) {
      buff_field[upper_limit + 1][i] = recieved_row[i-1];
    }
  } else if (rank == world->size() - 2) {
    world->recv<double>(rank - 1, low_tag, recieved_row.data(), count);

    for (int i = 1; i < count; ++i) {
      buff_field[lower_limit - 1][i] = recieved_row[i-1];
    }
  } else {
    world->recv<double>(rank - 1, low_tag, recieved_row.data(), count);
    for (int i = 1; i < count; ++i) {
      buff_field[lower_limit - 1][i] = recieved_row[i-1];
    }
    world->recv<double>(rank + 1, up_tag, recieved_row.data(), count);
    for (int i = 1; i < count; ++i) {
      buff_field[upper_limit + 1][i] = recieved_row[i-1];
    }
  }
}


void PlateHandler::send_forward(int rank, const int tag) {
  double *row_ptr = field[upper_limit] + 1;
  int count = static_cast<int>(field.get_columns()) - 2;

  world->isend<double>(rank + 1, tag, row_ptr, count);


#ifdef DEBUG
  std::cout << "send to rank" << rank + 1 << ":";
  for (auto i = 0; i < count; ++i) {
    std::cout << row_ptr[i] << " ";
  }
  std::cout << std::endl;
#endif
}

void PlateHandler::send_back(int rank, const int tag) {
  double *row_ptr = field[lower_limit] + 1;
  int count = static_cast<int>(field.get_columns()) - 2;

  world->isend<double>(rank - 1, tag, row_ptr, count);



#ifdef DEBUG
  std::cout << "send to rank" << rank - 1 << ":";
  for (auto i = 0; i < count; ++i) {
    std::cout << row_ptr[i] << " ";
  }
  std::cout << std::endl;
#endif
}

void PlateHandler::make_snapshot() {
  int step_number = static_cast<int>(full_time / t_step);
  int snapshot_period = static_cast<int>(t_snapshot / t_step);
  int num_snapshots = (step_number + snapshot_period - 1) / snapshot_period;

  for (int i = 0; i < num_snapshots; ++i) {

      for (auto rank = 0; rank < world->size() - 1; ++rank) {
        auto [lower, upper] = get_limits(rank);

        auto count = (upper - lower + 1) * field.cols;

        world->recv<double>(rank, rank, field[lower], count);
      }
      create_heatmap_image(field, i);
    }

}

void PlateHandler::send_field_info() {
  auto count = (upper_limit - lower_limit + 1) * field.cols;

  world->isend<double>(snap_shoter_rank, world->rank(), field[lower_limit], count);
}

void PlateHandler::update_buffer() {
  for (int i = lower_limit; i <= upper_limit; ++i) {
    for (int j = 1; j < width - 1; ++j) {
      buff_field(i, j) = field(i, j);
    }
  }
}

double PlateHandler::get_point_temp(int i, int j) const {
  double first_part = (buff_field[i - 1][j] - 2 * buff_field[i][j] + buff_field[i + 1][j]) / (x_step * x_step);
  double second_part = (buff_field[i][j - 1] - 2 * buff_field[i][j] + buff_field[i][j + 1]) / (y_step * y_step);

  return buff_field[i][j] + t_step * alpha * (first_part + second_part);
}
