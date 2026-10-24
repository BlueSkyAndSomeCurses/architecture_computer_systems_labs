#include "config_parser.h"

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>

std::string trim_quotes(const std::string &s) {
  std::string temp = s;
  boost::algorithm::trim_if(temp, [](char c) {
    return c == '"' || c == '\'';
  });
  return temp;
}

Config::Config(const std::string &file_name) {
  po::options_description desc;
  desc.add_options()
      ("indir", po::value<std::string>()->required())
      ("out_by_a", po::value<std::string>()->required())
      ("out_by_n", po::value<std::string>()->required())
      ("indexing_extensions", po::value<std::vector<std::string> >()->multitoken(), "Indexing file extensions")
      ("archives_extensions", po::value<std::vector<std::string> >()->multitoken(), "Archive file extensions")
      ("max_file_size", po::value<size_t>()->required())
      ("indexing_threads", po::value<int>()->default_value(1))
      ("merging_threads", po::value<int>()->default_value(1))
      ("filenames_queue_size", po::value<int>()->default_value(10000), "Size of filenames queue")
      ("raw_files_queue_size", po::value<int>()->default_value(1000), "Size of raw files queue")
      ("dictionaries_queue_size", po::value<int>()->default_value(1000), "Size of dictionary queue")
      ("measure_indexing_time", po::value<bool>()->default_value(false), "Measure indexing phase time");


  po::variables_map vm;

  try {
    std::ifstream config_file(file_name);
    if (!config_file) throw std::runtime_error("помилка відкриття файлу конфігурації");

    po::store(po::parse_config_file(config_file, desc, true), vm);
    po::notify(vm);

    indir = trim_quotes(vm["indir"].as<std::string>());
    out_by_a = trim_quotes(vm["out_by_a"].as<std::string>());
    out_by_n = trim_quotes(vm["out_by_n"].as<std::string>());


    for (const auto &ext: vm["indexing_extensions"].as<std::vector<std::string> >()) {
      std::vector<std::string> tokens;
      boost::split(tokens, ext, boost::is_any_of(" "), boost::token_compress_on);
      for (const auto &token: tokens) {
        if (!token.empty())
          indexing_extensions.insert(boost::trim_copy(token));
      }
    }

    for (const auto &ext: vm["archives_extensions"].as<std::vector<std::string> >()) {
      std::vector<std::string> tokens;
      boost::split(tokens, ext, boost::is_any_of(" "), boost::token_compress_on);
      for (const auto &token: tokens) {
        if (!token.empty())
          archives_extensions.insert(boost::trim_copy(token));
      }
    }

    max_file_size = vm["max_file_size"].as<size_t>();

    indexing_threads = vm["indexing_threads"].as<int>();
    merging_threads = vm["merging_threads"].as<int>();
    filenames_queue_size = vm["filenames_queue_size"].as<int>();
    raw_files_queue_size = vm["raw_files_queue_size"].as<int>();
    dictionaries_queue_size = vm["dictionaries_queue_size"].as<int>();


    if (vm.contains("measure_indexing_time"))
      measure_indexing_time = vm["measure_indexing_time"].as<bool>();
  } catch (const std::exception &e) {
    std::cerr << "помилка обробки файлу конфігурації" << e.what() << "\n";
    exit(5);
  }
}
