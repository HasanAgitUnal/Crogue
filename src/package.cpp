// CROGUE - Card-based rogulike TUI game
// Copyright (C) 2026  Hasan Agit Ünal
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/*
 * crogue install [-f PACKAGENAME.tar.gz | -g GITREPO]
 * crogue remove PACKAGENAME
 * crogue reset PACKAGENAME
 *
 * -g features will be added on future
 *
 */

// clang-format off
#include <minizip-ng/mz.h>
#include "minizip-ng/mz_strm.h"
#include <minizip-ng/mz_zip.h>
#include <minizip-ng/mz_zip_rw.h>
// clang-format on
#include <filesystem>
#include <fstream>
#include <random>
#include <string>

#include "lua.hpp"
#include "minilog.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

fs::path get_data_dir() {
        fs::path base_path;

#if defined(_WIN32)
        const char *appdata = std::getenv("APPDATA");
        if (appdata) {
                base_path = fs::path(appdata);
        } else {
                base_path = fs::path(std::getenv("USERPROFILE")) / "AppData" / "Roaming";
        }
#else
        const char *xdg_data = std::getenv("XDG_DATA_HOME");
        if (xdg_data && *xdg_data) {
                base_path = fs::path(xdg_data);
        } else {
                base_path = fs::path(std::getenv("HOME")) / ".local" / "share";
        }
#endif

        return base_path / "crogue";
}

fs::path get_cache_dir() {
        fs::path base_path;

#if defined(_WIN32)
        const char *localappdata = std::getenv("LOCALAPPDATA");
        if (localappdata) {
                base_path = fs::path(localappdata);
        } else {
                base_path = fs::path(std::getenv("USERPROFILE")) / "AppData" / "Local";
        }
#else
        const char *xdg_cache = std::getenv("XDG_CACHE_HOME");
        if (xdg_cache && *xdg_cache) {
                base_path = fs::path(xdg_cache);
        } else {
                base_path = fs::path(std::getenv("HOME")) / ".cache";
        }
#endif

        return base_path / "crogue";
}

namespace package {

static std::string trim(const std::string &str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos)
                return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
}

void extract_zip(const std::string &zip_path, const std::string &target_dir) {
        void *reader = mz_zip_reader_create();
        if (!reader)
                minilog::fatal(1, "Not enough memory avaible.");

        if (mz_zip_reader_open_file(reader, zip_path.c_str()) != MZ_OK) {
                mz_zip_reader_delete(&reader);
                minilog::fatal(1, "Can't open or read zip archive: ", zip_path);
        }

        mz_zip_reader_save_all(reader, target_dir.c_str());
        mz_zip_reader_close(reader);

        mz_zip_reader_delete(&reader);
}

fs::path create_temp_dir() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;

        std::stringstream ss;
        ss << "crogue_pkg_" << std::hex << std::setfill('0') << std::setw(16) << dis(gen);

        fs::path temp_dir = fs::temp_directory_path() / ss.str();

        std::error_code ec;
        fs::create_directories(temp_dir, ec);
        if (ec) {
                minilog::fatal(1, "Failed to create temp directory: ", ec.message());
        }

        return temp_dir;
}

void check_package(fs::path pack_dir) {
        minilog::out("\033[32m==>\033[0m Checking init.lua...");

        fs::path init_file = pack_dir / "init.lua";
        if (!fs::exists(init_file) || !fs::is_regular_file(init_file)) {
                minilog::fatal(1, "init.lua does not exists or not a file");
        }

        minilog::out("\033[32m==>\033[0m Checking metadata.json...");

        fs::path metadata_file = pack_dir / "metadata.json";
        if (!fs::exists(metadata_file) || !fs::is_regular_file(metadata_file)) {
                minilog::fatal(1, "metadata.json does not exists or not a file");
        }

        std::ifstream file(metadata_file);
        if (!file.is_open()) {
                minilog::fatal(1, "Can't read metadata.json");
        }

        json metadata;
        try {
                file >> metadata;
        } catch (json::exception &e) {
                minilog::fatal(1, "Invalid JSON: ", e.what());
        }

        file.close();

        std::string error = check_metadata(metadata);
        if (error != "") {
                minilog::fatal(1, "Invalid metadata: ", error);
        };

        minilog::out("\033[32m==>\033[0m Checking pack_name.txt...");

        fs::path pack_name_file = pack_dir / "pack_name.txt";
        if (!fs::exists(pack_name_file) || !fs::is_regular_file(pack_name_file)) {
                minilog::fatal(1, "pack_name.txt does not exists or not a file");
        }
}

bool confirm(const std::string msg) {
        std::cout << msg << std::flush;

        std::string input;
        std::getline(std::cin, input);

        std::string trimmed_input = trim(input);

        char confirm = trimmed_input.empty() ? 'y' : std::tolower(trimmed_input[0]);

        if (confirm != 'y') {
                return false;
        }
        return true;
}

void pack() {
        fs::path cwd = fs::current_path();

        check_package(cwd);

        fs::path pack_name_file = cwd / "pack_name.txt";
        std::ifstream file(pack_name_file);
        if (!file.is_open()) {
                minilog::fatal(1, "Can't read pack_name.txt.");
        }

        std::string raw_name((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string zip_basename = trim(raw_name);

        if (zip_basename.empty()) {
                minilog::fatal(1, "pack_name.txt can't be empty.");
        }


        minilog::out("\033[32m==>\033[0m Compressing...");
        fs::path absolute_target_zip = fs::absolute(cwd / (zip_basename + ".zip"));

        fs::path temp_zip = fs::absolute(cwd / (zip_basename + ".tmp"));

        void *zip_writer = mz_zip_writer_create();
        if (!zip_writer) {
                minilog::fatal(1, "Not enough memory avaible.");
        }

        mz_zip_writer_set_compress_level(zip_writer, MZ_COMPRESS_LEVEL_BEST);

        int32_t err = mz_zip_writer_open_file(zip_writer, temp_zip.string().c_str(), 0, 0);
        if (err != MZ_OK) {
                mz_zip_writer_delete(&zip_writer);
                minilog::fatal(1, "Can't create or open .zip file: ", absolute_target_zip.string(),
                               ". Error code: ", std::to_string(err));
        }

        for (const auto &entry : fs::recursive_directory_iterator(cwd)) {
                fs::path current_entry_path = fs::absolute(entry.path());

                // dont compress .zip or .tmp file
                if (current_entry_path == absolute_target_zip || current_entry_path == temp_zip) {
                        continue;
                }

                fs::path relative_path = fs::relative(current_entry_path, cwd);
                std::string zip_entry_name = relative_path.generic_string();

                if (fs::is_regular_file(entry.status())) {
                        mz_zip_writer_add_file(zip_writer, current_entry_path.string().c_str(), zip_entry_name.c_str());
                }
        }

        mz_zip_writer_close(zip_writer);
        mz_zip_writer_delete(&zip_writer);

        std::error_code ec;
        fs::rename(temp_zip, absolute_target_zip, ec);
        if (ec) {
                minilog::fatal(1, "Failed to rename zip file: ", ec.message());
        }

        minilog::out("\033[32m==>\033[0m Successfully generated: ", zip_basename, ".zip");
}

void install(const fs::path &directory, const std::string &plugin_name, bool force) {
        fs::path plugins_dir = get_data_dir() / "plugins";

        try {
                fs::create_directories(plugins_dir);
        } catch (const fs::filesystem_error &e) {
                minilog::fatal(1, "Can't create plugin directory: ", e.what());
        }

        fs::path plugin_dir = plugins_dir / plugin_name;

        if (fs::exists(plugin_dir)) {
                if (force) {
                        fs::remove_all(plugin_dir);
                        goto install;
                }

                minilog::out("\033[33m==>\033[0m A plugin with this name already exists.");
                if (!confirm("\033[32m==>\033[0m Remove it and continue to installation? [Y/n]: ")) {
                        minilog::out("\033[33m==>\033[0m Canceled...");
                        fs::remove_all(directory);
                        exit(0);
                }

                fs::remove_all(plugin_dir);
        }

install:
        minilog::out("\033[32m==>\033[0m Installing...");
        std::error_code ec;
        fs::rename(directory, plugin_dir, ec);

        if (ec) {
                ec.clear();

                fs::copy(directory, plugin_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
                if (ec) {
                        minilog::fatal(1, "Copy failed: ", ec.message());
                }

                // no error handling because its not important
                fs::remove_all(directory);
        }
        minilog::out("\033[32m==>\033[0m Installation Successfull");
}

void install_file(const std::string &path, bool force) {
        if (!fs::exists(path)) {
                minilog::fatal(1, "File doesn't exitst: ", path);
        }

        fs::path temp_dir = create_temp_dir();

        extract_zip(path, temp_dir);

        check_package(temp_dir);

        std::ifstream file(temp_dir / "pack_name.txt");
        std::string raw_name((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        std::string plugin_name = trim(raw_name);
        if (plugin_name.empty()) {
                fs::remove_all(temp_dir);
                minilog::fatal(1, "pack_name.txt is empty.");
        }

        install(temp_dir, plugin_name, force);
}

void install_git(const std::string &repo_url, bool force) {

};

void remove(const std::string &package, bool force) {
        fs::path pack(get_data_dir() / "plugins" / package);

        if (force) {
                goto remove;
        }

        if (!confirm("\033[32m==>\033[0m Are you sure? [Y/n]: ")) {
                minilog::out("\033[33m==>\033[0m Canceled...");
                exit(0);
        }

remove:
        if (!fs::exists(pack)) {
                minilog::fatal(1, "Plugin does not exist.");
        }

        fs::remove_all(pack);
}

void reset(const std::string &package) {
        fs::path settings(get_data_dir() / "plugins" / package / "settings.json");

        // do not matter exists or not
        fs::remove(settings);
}

void list() {
        fs::path plugins_dir(get_data_dir() / "plugins");
        std::cout << "\033[35m";
        for (const auto &entry : std::filesystem::directory_iterator(plugins_dir)) {
                fs::path path = entry.path();
                if (fs::is_directory(path)) {
                        minilog::out(path.filename().string());
                }
        }
        std::cout << "\033[0m";
}

}  // namespace package
