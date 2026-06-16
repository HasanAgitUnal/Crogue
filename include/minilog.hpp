#pragma once
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace minilog {

namespace msg {

inline const std::string debug = "\033[34mD\033[0m: ";
inline const std::string info = "\033[35mI\033[0m: ";
inline const std::string warning = "\033[33mW\033[0m: ";
inline const std::string error = "\033[31mE: ";
inline const std::string fatal = "\033[31mFATAL: ";
inline const std::string success = "\033[32mSUCCES:\033[0m ";

}  // namespace msg

template <typename... MSG>
inline void out(const MSG &...msg) {
        std::stringstream ss;
        (ss << ... << msg);
        std::cout << ss.str() << "\n";
}

template <typename... MSG>
inline void err(const MSG &...msg) {
        std::stringstream ss;
        (ss << ... << msg);
        std::cerr << ss.str() << "\n";
}

template <typename... MSG>
inline void debug(const MSG &...msg) {
#ifdef DEBUG
        err(msg::debug, msg...);
#endif
}

template <typename... MSG>
inline void fatal(const char e_code, const MSG &...message) {
        err(msg::fatal, message...);
        exit(e_code);
}

template <typename... MSG>
inline void fout(const std::string &log_file, const MSG &...msg) {
        static std::ofstream lfile(log_file, std::ios::app);
        (lfile << ... << msg);
        lfile << std::endl;
}

template <typename... MSG>
inline void fdebug(const std::string &filename, const MSG &...msg) {
#ifdef DEBUG
        fout(filename, msg::debug, msg...);
#endif
}

template <typename... MSG>
inline void ffatal(const std::string &filename, const MSG &...message) {
        fout(filename, msg::fatal, message...);
        exit(1);
}

}  // namespace minilog
