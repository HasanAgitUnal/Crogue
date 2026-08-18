#pragma once
#include <filesystem>

namespace fs = std::filesystem;

fs::path get_data_dir();
fs::path get_cache_dir();

namespace package {

fs::path create_temp_dir();

void pack();

bool check_package(fs::path pack_dir, bool silent = false);

void install_plugins(const std::vector<std::string> &files, const std::vector<std::string> &repos, bool force);

void remove(const std::string &package, bool force);

void reset(const std::string &package, bool force);

void update(std::vector<std::string> packages);

void list(json *output = nullptr);

}  // namespace package
