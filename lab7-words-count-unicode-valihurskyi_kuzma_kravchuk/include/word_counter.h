#ifndef WORD_COUNTER_H
#define WORD_COUNTER_H
#include <string>
#include <unordered_map>
#include <archive.h>
#include <archive_entry.h>

#include "config_parser.h"

#ifdef USE_MAP
typedef std::map<std::string, int> word_count_t;
#else
#ifdef USE_ABSEIL_HASH_MAP
#include <absl/container/flat_hash_map.h>
typedef absl::flat_hash_map<std::string, int> word_count_t;
#else
typedef std::unordered_map<std::string, int> word_count_t;
#endif
#endif


class Archivator {
  struct archive *archive = nullptr;

public:
  Archivator();

  void process_files(const std::string &archive_path, Config &config, word_count_t &word_count, bool isMmap = false, bool isICU = false);

  void process_content(const std::string &buff, Config &config, word_count_t &word_count, bool isICU = false);

  ~Archivator();
};

void iterate_through_directory(Config &config, bool isMmap = false, bool isICU = false);

void write_results_to_file(const word_count_t &word_count, Config &config);

#endif //WORD_COUNTER_H
