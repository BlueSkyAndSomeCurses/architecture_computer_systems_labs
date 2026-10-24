#include "transfer_funcs_d.hpp"

double TransferD::s_scanf(const std::string& str) {

    double number;

    sscanf(str.c_str(), "%lf", &number);

    return number;
}

double TransferD::s_tod(const std::string& str) {

    return std::stod(str);
}


double TransferD::fr_chars(const std::string& str) {
    double val;
    std::from_chars(str.data(), str.data() + str.size(), val);
    return val;
}


double TransferD::a_toi(const std::string& str) {

    return static_cast<double>(atoi(str.c_str()));
}


double TransferD::str_stream(const std::string& str) {

    double number;
    std::stringstream sstr(str);

    sstr >> number;

    return number;
}
