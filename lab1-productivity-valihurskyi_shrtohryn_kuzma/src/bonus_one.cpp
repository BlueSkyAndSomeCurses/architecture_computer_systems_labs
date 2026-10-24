// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "bonus_one.hpp"

// Utility functions

bool is_number(const std::string &str) {
    return !str.empty() && std::find_if(str.begin(),str.end(),
    [](unsigned char c) { return !std::isdigit(c); }) == str.end();
}

template <typename T>
unsigned int letter_count(const T &data) {
    unsigned int count = 0;
    for (const auto &s : data) {
        count += s.size();
    }
    return count;
}

unsigned int sum_mod_64(const std::vector<unsigned int> &numbers) {
    unsigned int sum = 0;
    for (const auto &n : numbers) {
        sum += n;
        sum %= 64;
    }
    return sum;
}

// Test splitting a buffer into a vector of strings

void boost_split(const std::string &str, std::vector<std::string> &data) {
    boost::split(data, str, boost::is_any_of("\n"), boost::token_compress_on);
}

void qt_split(const QString &str, QStringList &data) {
    data = str.split("\n", Qt::SkipEmptyParts);
}

void absl_split(const std::string &str, std::vector<std::string> &data) {
    data = absl::StrSplit(str, '\n');
}

// Test converting split strings into numbers

void boost_convert(const std::vector<std::string> &data, std::vector<unsigned int> &numbers) {
    for (const auto &s : data) {
        numbers.push_back(boost::lexical_cast<unsigned int>(s));
    }
}

void qt_to_uint(const QStringList &data, std::vector<unsigned int> &numbers) {
    for (const auto &s : data) {
        numbers.push_back(s.toUInt());
    }
}

void absl_atoi(const std::vector<std::string> &data, std::vector<unsigned int> &numbers) {
    for (const auto &s : data) {
        unsigned int number;
        auto _ = absl::SimpleAtoi(s, &number);
        numbers.push_back(number);
    }
}

int main(int argc, char *argv[]) {
    command_line_options_t command_line_options_t{argc, argv};

    std::vector<std::string> cl_arguments = command_line_options_t.get_filenames();

    if (cl_arguments.size() != 2) {
        std::cerr << "неправильна кількість аргументів командного рядка" << std::endl;
        return 1;
    }


    if (!is_number(cl_arguments[1])) {
        std::cerr << "неправильний номер методу" << std::endl;
        return 2;
    }

    int method_number = std::stoi(cl_arguments[1]);
    if (method_number < 1 || method_number > 6) {
        std::cerr << "неправильний номер методу" << std::endl;
        return 2;
    }

    try {
        assert_file_exist(cl_arguments[0]);
    } catch (const std::invalid_argument &e) {
        std::cerr << "не вдалося відкрити файл із вхідними даними\n" << e.what() << std::endl;
        return 3;
    }

    // input
    std::ifstream file{cl_arguments[0]};
    std::stringstream ss;
    ss << file.rdbuf();
    std::string data = ss.str();
    data.pop_back(); // remove last newline
    QString qt_data = QString::fromStdString(data);

    // output
    std::vector<std::string> data_vector;
    std::vector<unsigned int> numbers;
    QStringList qt_data_vector;

    std::chrono::high_resolution_clock::time_point stage_one_time, stage_two_time;

    switch (method_number) {
        case 1:
            stage_one_time = get_current_time_fenced();
            boost_split(data, data_vector);
            stage_two_time = get_current_time_fenced();
            break;
        case 2:
            stage_one_time = get_current_time_fenced();
            qt_split(qt_data, qt_data_vector);
            stage_two_time = get_current_time_fenced();
            break;
        case 3:
            stage_one_time = get_current_time_fenced();
            absl_split(data, data_vector);
            stage_two_time = get_current_time_fenced();
            break;
        case 4:
            absl_split(data, data_vector);
            stage_one_time = get_current_time_fenced();
            boost_convert(data_vector, numbers);
            stage_two_time = get_current_time_fenced();
            break;
        case 5:
            qt_split(qt_data, qt_data_vector);
            stage_one_time = get_current_time_fenced();
            qt_to_uint(qt_data_vector, numbers);
            stage_two_time = get_current_time_fenced();
            break;
        case 6:
            absl_split(data, data_vector);
            stage_one_time = get_current_time_fenced();
            absl_atoi(data_vector, numbers);
            stage_two_time = get_current_time_fenced();
            break;
    }
    
    double elapsed_time_ms = std::chrono::duration<double, std::milli>(stage_two_time - stage_one_time).count();
    unsigned int elapsed_time_ms_ui = static_cast<unsigned int>(elapsed_time_ms);

    std::cout << elapsed_time_ms_ui << std::endl;
    if (method_number < 4) {
        unsigned int lc;
        if (method_number == 2) lc = letter_count(qt_data_vector);
        else lc = letter_count(data_vector); 
        std::cout << lc << std::endl;
    } else {
        auto number_sum = sum_mod_64(numbers);
        std::cout << number_sum << std::endl;
    }
}
