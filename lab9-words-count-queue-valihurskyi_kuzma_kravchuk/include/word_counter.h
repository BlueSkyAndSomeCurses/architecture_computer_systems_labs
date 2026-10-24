#ifndef WORD_COUNTER_H
#define WORD_COUNTER_H
#include <string>
#include <unordered_map>
#include <archive.h>
#include <oneapi/tbb.h>
#include <archive_entry.h>

#include "config_parser.h"

// Taken from book Pro TBB, C++ Parallel Programming with Threading Building Blocks by Michael Voss, Rafael Asenjo, James Reinders

typedef std::unordered_map<std::string, int> word_count_t;


class Archivator {
  struct archive *archive = nullptr;

public:
  Archivator();

  void process_files(const std::string &archive_path, Config &config, word_count_t &word_count);

  void process_content(const std::string &buff, Config &config, word_count_t *word_count);

  ~Archivator();
};


void iterate_through_directory(Config &config);

void write_results_to_file(const word_count_t &word_count, Config &config);

void start_hell_machine(Config &config);

#endif //WORD_COUNTER_H
