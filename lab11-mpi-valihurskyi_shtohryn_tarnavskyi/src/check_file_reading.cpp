#include "check_file_reading.hpp"

Matrix get_start_points(const std::string &path) {
  std::ifstream infile(path);
  if (!infile.is_open()) {
    throw std::runtime_error{"Could not open file: " + path};
  }

  std::string line;
  std::vector<double> flat_data;
  size_t rows = 0;
  size_t cols = 0;

  while (std::getline(infile, line)) {
    if (line.empty()) continue;

    std::vector<std::string> tokens;
    boost::split(tokens, line, boost::is_any_of(", "), boost::token_compress_on);
    cols = tokens.size();
    for (const auto& token : tokens) {
      flat_data.push_back(std::stod(token));
    }

    ++rows;
  }

  Matrix result{rows, cols};


  for (size_t r = 0; r < rows; ++r) {
    for (size_t c = 0; c < cols; ++c) {
      result(r, c) = flat_data[r * cols + c];
    }
  }

  return result;
}

std::unordered_map<int, std::unordered_map<std::pair<int, int>, double, boost::hash<std::pair<int, int> > > >
get_hot_points(std::string path, double total_time, double t_step) {
  std::ifstream infile(path);

  if (!infile.is_open()) {
    throw std::runtime_error{"Could not open file: " + path};
  }

  std::unordered_map<int, std::unordered_map<std::pair<int, int>, double, boost::hash<std::pair<int, int> > > > result;
  std::unordered_map<std::pair<int, int>, double, boost::hash<std::pair<int, int> > > temp;

  std::string line;
  std::vector<std::string> parts;
  int x, y;
  double temperature, time;
  int tick;

  int ticks = static_cast<int>(total_time / t_step);

  while (std::getline(infile, line)) {
    boost::split(parts, line, boost::is_any_of(","));

    time = std::stod(parts[0]);
    x = std::stoi(parts[1]);
    y = std::stoi(parts[2]);
    temperature = std::stod(parts[3]);

    tick = static_cast<int>(ticks * (time / total_time));

    if (!result.contains(tick)) {
      temp.clear();
      result[tick] = temp;
    }

    result[tick][{x, y}] = temperature;
  }

  return result;
}

std::pair<std::pair<std::vector<int>, std::vector<double> >, std::string > check_file_reading(
  const std::string& file_name) {
  toml::table tbl;
  try {
    tbl = toml::parse_file(file_name);
  } catch (const toml::parse_error &err) {
    std::cerr << "не вдалося відкрити файл із конфігурацією" << err << "\n";
    std::exit(3);
  }

  int x_step, y_step;
  double t_step, t_snapshot, alpha, total_time;
  std::string start_temperatures;

  try {
    x_step = tbl["x_step"].value<int>().value();
    y_step = tbl["y_step"].value<int>().value();
    t_step = tbl["t_step"].value<double>().value();
    total_time = tbl["total_time"].value<double>().value();
    t_snapshot = tbl["t_snapshot"].value<double>().value();
    alpha = tbl["alpha"].value<double>().value();

    start_temperatures = tbl["start_temperatures"].value<std::string>().value();
  } catch (const std::bad_optional_access &err) {
    std::cerr << "помилка читання файлу конфігурації, або помилка його вмісту" << "\n";
    std::exit(5);
  }

  if (t_step > std::pow(std::max(x_step, y_step), 2) / (4 * alpha)) {
    std::cerr << "Порушення умов стабільності за критерієм фон Неймана" << std::endl;
    std::exit(65);
  }

  std::ifstream infile1(start_temperatures);

  if (!infile1.is_open() || !infile1.good()) {
    std::cerr << "файл з даними відсутній або недоступний" << std::endl;
    std::exit(26);
  }

  infile1.close();

  std::ifstream infile2(start_temperatures);

  if (!infile2.is_open() || !infile2.good()) {
    std::cerr << "файл з даними відсутній або недоступний" << std::endl;
    std::exit(26);
  }

  infile2.close();

  return {
    {{x_step, y_step}, {t_step, t_snapshot, alpha, total_time}},
    start_temperatures
  };
}
