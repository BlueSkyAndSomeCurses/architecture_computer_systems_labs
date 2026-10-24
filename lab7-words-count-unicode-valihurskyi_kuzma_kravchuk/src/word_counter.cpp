#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <boost/locale.hpp>
#include <unicode/unistr.h>
#include <unicode/brkiter.h>
#include <unicode/normalizer2.h>
#include <unicode/ucasemap.h>
#include <unicode/ustream.h>

#include "word_counter.h"
#include "config_parser.h"
#include "timer.h"

Archivator::Archivator() {
  archive = archive_read_new();
  archive_read_support_filter_all(archive);
  archive_read_support_format_all(archive);
}

Archivator::~Archivator() {
  archive_read_free(archive);
}

std::string read_file_into_buffer(const std::filesystem::path &file_path) {
  std::ifstream raw_file(file_path, std::ios::binary);
  if (!raw_file) {
    throw std::runtime_error("помилка читання архіву " + file_path.string());
  }

  auto buffer = [&raw_file]() {
    std::ostringstream ss{};
    ss << raw_file.rdbuf();
    return ss.str();
  }();
  // auto buffer = static_cast<std::ostringstream &>(
  // std::ostringstream{} << raw_file.rdbuf()).str();

  return buffer;
}

std::string read_file_mmap(const std::filesystem::path &file_path) {
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Cannot open file: " + file_path.string());
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        close(fd);
        throw std::runtime_error("fstat failed: " + file_path.string());
    }

    size_t length = sb.st_size;
    void *mapped = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        close(fd);
        throw std::runtime_error("mmap failed: " + file_path.string());
    }

    std::string result(reinterpret_cast<char *>(mapped), length);

    munmap(mapped, length);
    close(fd);

    return result;
}

void process_text_with_boost(const std::string &text, word_count_t &word_count) {
  boost::locale::boundary::ssegment_index word_index(
      boost::locale::boundary::word, text.begin(), text.end());
  word_index.rule(boost::locale::boundary::word_letter);

  for (auto it = word_index.begin(); it != word_index.end(); ++it) {
    std::string word = *it;
    word = boost::locale::fold_case(boost::locale::normalize(word));
    ++word_count[word];
  }
}

void process_text_with_icu(const std::string &text, word_count_t &word_count) {
  UErrorCode status = U_ZERO_ERROR;

  icu::UnicodeString utext = icu::UnicodeString::fromUTF8(text);
  std::unique_ptr<icu::BreakIterator> wordBreaker(
      icu::BreakIterator::createWordInstance(icu::Locale::getUS(), status));
  if (U_FAILURE(status)) throw std::runtime_error("Failed to create BreakIterator");

  wordBreaker->setText(utext);

  const icu::Normalizer2 *normalizer = icu::Normalizer2::getNFKCInstance(status);
  if (U_FAILURE(status)) throw std::runtime_error("Failed to get Normalizer2");

  int32_t start = wordBreaker->first();
  for (int32_t end = wordBreaker->next(); end != icu::BreakIterator::DONE; start = end, end = wordBreaker->next()) {
    icu::UnicodeString word = utext.tempSubStringBetween(start, end).trim();

    if (word.isEmpty()) continue;

    if (word == ":") continue;

    icu::UnicodeString normWord = normalizer->normalize(word, status);
    normWord.toLower();

    std::string utf8word;
    normWord.toUTF8String(utf8word);
    ++word_count[utf8word];
  }
}


void Archivator::process_content(const std::string &buff, Config &config, word_count_t &word_count, bool isICU) {
  if (archive_read_open_memory(archive, buff.c_str(), buff.size()) != ARCHIVE_OK) {
    return;
  }

  struct archive_entry *entry;
  while ((archive_read_next_header(archive, &entry)) == ARCHIVE_OK) {
    const char *entry_name = archive_entry_pathname(entry);
    const std::filesystem::path entry_path = entry_name;
    const auto file_type = archive_entry_filetype(entry);

    auto is_archive = config.archives_extensions.contains(entry_path.extension());
    auto is_textfile = config.indexing_extensions.contains(entry_path.extension());

    if (!is_archive && !is_textfile) {
      archive_read_data_skip(archive);
      continue;
    }
    if (is_textfile && static_cast<size_t>(archive_entry_size(entry)) >= config.max_file_size) {
      archive_read_data_skip(archive);
      continue;
    }


    std::string content;
    if (file_type == AE_IFREG) {
      char buffer[4096];
      la_ssize_t bytes_read;

      while ((bytes_read = archive_read_data(archive, buffer, sizeof(buffer))) > 0) {
        content.append(buffer, bytes_read);
      }
    }

    if (is_textfile) {
      if (isICU) {
        process_text_with_icu(content, word_count);
      } else {
        process_text_with_boost(content, word_count);
      }
    }

    if (is_archive) {
      Archivator().process_content(content, config, word_count);
    }
  }
}

void Archivator::process_files(const std::string &archive_path, Config &config, word_count_t &word_count, bool isMmap, bool isICU) {
  std::string buff;
  if (isMmap) {
    buff = read_file_mmap(archive_path);
  } else {
    buff = read_file_into_buffer(archive_path);
  }

  process_content(buff, config, word_count, isICU);
}


void iterate_through_directory(Config &config, bool isMmap, bool isICU) {
  auto total_start = get_current_time_fenced();

  boost::locale::generator gen;
  std::locale loc = gen("en_US.UTF-8");
  std::locale::global(loc);

  word_count_t word_count;
  for (const auto &entry: std::filesystem::recursive_directory_iterator(config.indir)) {
    if (!std::filesystem::is_regular_file(entry)) {
      continue;
    }

    auto arch = Archivator();

    if (config.archives_extensions.contains(entry.path().extension())) {
      arch.process_files(entry.path(), config, word_count, isMmap, isICU);
    }
    if (config.indexing_extensions.contains(entry.path().extension())) {

      std::string content;
      if (isMmap) {
        content = read_file_mmap(entry.path());
      } else {
        content = read_file_into_buffer(entry.path());
      }

      if (isICU) {
        process_text_with_icu(content, word_count);
      } else {
        process_text_with_boost(content, word_count);
      }
    }
  }

  auto write_start = get_current_time_fenced();  // Writing starts here
  write_results_to_file(word_count, config);
  auto end = get_current_time_fenced();          // Everything ends here

  using namespace std::chrono;
  auto total_duration = duration_cast<milliseconds>(end - total_start).count();
  auto writing_duration = duration_cast<milliseconds>(end - write_start).count();

  std::cout << "Total=" << total_duration << std::endl;
  std::cout << "Writing=" << writing_duration << std::endl;
}


void write_results_to_file(const word_count_t &word_count, Config &config) {
  // Save in alphabetical order
  std::ofstream output_file(config.out_by_a);
  if (!output_file) {
    throw std::runtime_error("помилка запису у вихідний файл: " + config.out_by_n);
  }

#ifndef USE_MAP
  std::vector<std::pair<std::string, int>> word_count_vector(word_count.begin(), word_count.end());
  std::sort(word_count_vector.begin(), word_count_vector.end(), [](const auto &a, const auto &b) {
    return a.first < b.first;
  });
  for (const auto &pair : word_count_vector) {
    output_file << pair.first << ": " << pair.second << std::endl;
  }
#else
  for (const auto &pair : word_count) {
    output_file << pair.first << ": " << pair.second << std::endl;
  }
#endif
  output_file.close();

  // Save in descending order
#ifdef USE_MAP
  std::vector<std::pair<std::string, int>> word_count_vector(word_count.begin(), word_count.end());
#endif
  std::sort(word_count_vector.begin(), word_count_vector.end(), [](const auto &a, const auto &b) {
    if (a.second != b.second)
      return a.second > b.second;
    return a.first < b.first;
  });

  std::ofstream output_file2(config.out_by_n);
  if (!output_file2) {
    throw std::runtime_error("помилка запису у вихідний файл: " + config.out_by_a);
  }

  for (const auto &pair : word_count_vector) {
    output_file2 << pair.first << ": " << pair.second << std::endl;
  }
  output_file2.close();
}
