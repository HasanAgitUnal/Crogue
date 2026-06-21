#pragma once
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace minilog {

namespace msg {

inline const std::string debug = "\033[34mD\033[0m: ";
inline const std::string info = "\033[35mI\033[0m: ";
inline const std::string warning = "\033[33mW\033[0m: ";
inline const std::string error = "\033[31mE: ";
inline const std::string fatal = "\033[31mFATAL: ";
inline const std::string success = "\033[32mSUCCES:\033[0m ";

}  // namespace msg

inline std::unordered_map<std::string, std::string> categories;

// Helper to get formatted category string
inline std::string format_cat(const std::string &cat) {
        if (categories.find(cat) != categories.end()) {
                return "[" + std::string("\033[") + categories[cat] + cat + "\033[0m" + "] ";
        }
        return "";
}

/*
 * Normal
 */

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

/*
 * File
 */

template <typename... MSG>
inline void fout(const std::string &log_file, const MSG &...msg) {
        static std::ofstream lfile(log_file, std::ios::app);
        (lfile << ... << msg);
        lfile << std::endl;
        lfile.flush();
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

/*
 * Category
 */

template <typename... MSG>
inline void outc(std::string cat, const MSG &...msg) {
        std::stringstream ss;
        (ss << ... << msg);
        std::cout << format_cat(cat) << ss.str() << '\n';
}

template <typename... MSG>
inline void errc(const std::string cat, const MSG &...msg) {
        std::stringstream ss;
        (ss << ... << msg);
        std::cerr << format_cat(cat) << ss.str() << '\n';
}

template <typename... MSG>
inline void debugc(const std::string cat, const MSG &...msg) {
#ifdef DEBUG
        errc(cat, msg::debug, msg...);
#endif
}

template <typename... MSG>
inline void fatalc(const std::string cat, const char e_code, const MSG &...message) {
        errc(cat, msg::fatal, message...);
        exit(e_code);
}

/*
 * File & Category
 */

template <typename... MSG>
inline void foutc(const std::string &cat, const std::string &log_file, const MSG &...msg) {
        fout(log_file, format_cat(cat), msg...);
}

template <typename... MSG>
inline void fdebugc(const std::string &cat, const std::string &filename, const MSG &...msg) {
#ifdef DEBUG
        fout(filename, msg::debug, format_cat(cat), msg...);
#endif
}

template <typename... MSG>
inline void ffatalc(const std::string &cat, const std::string &filename, const MSG &...message) {
        fout(filename, msg::fatal, format_cat(cat), message...);
        exit(1);
}

}  // namespace minilog
