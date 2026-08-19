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

// clang-format off
#include <minizip-ng/mz.h>
#include "minizip-ng/mz_strm.h"
#include <minizip-ng/mz_zip.h>
#include <minizip-ng/mz_zip_rw.h>
// clang-format on
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <minilog.hpp>
#include <random>
#include <string>

#include "game.hpp"
#include "lua.hpp"
#include "utils.hpp"

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

static void extract_zip(const std::string &zip_path, const std::string &target_dir) {
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
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;

        std::stringstream ss;
        ss << "crogue_" << std::hex << std::setfill('0') << std::setw(16) << dis(gen);

        fs::path temp_dir = fs::temp_directory_path() / ss.str();

        std::error_code ec;
        fs::create_directories(temp_dir, ec);
        if (ec) {
                minilog::fatal(1, "Failed to create temp directory: ", ec.message());
        }

        return temp_dir;
}

bool check_package(fs::path pack_dir, bool silent = false) {
        if (!silent) {
                minilog::out("\033[32m==>\033[0m Checking package");
        }

        fs::path init_file = pack_dir / "init.lua";
        if (!fs::exists(init_file) || !fs::is_regular_file(init_file)) {
                if (!silent) {
                        minilog::fatal(1, "init.lua does not exists or not a file");
                }
                return false;
        }

        fs::path metadata_file = pack_dir / "metadata.json";
        if (!fs::exists(metadata_file) || !fs::is_regular_file(metadata_file)) {
                if (!silent) {
                        minilog::fatal(1, "metadata.json does not exists or not a file");
                }
                return false;
        }

        std::ifstream file(metadata_file);
        if (!file.is_open()) {
                if (!silent) {
                        minilog::fatal(1, "Can't read metadata.json");
                }
                return false;
        }

        json metadata;
        try {
                file >> metadata;
        } catch (json::exception &e) {
                if (!silent) {
                        minilog::fatal(1, "Invalid JSON: ", e.what());
                }
                return false;
        }

        file.close();

        std::string error = check_metadata(metadata);
        if (error != "") {
                if (!silent) {
                        minilog::fatal(1, "Invalid metadata: ", error);
                }
                return false;
        }

        fs::path pack_name_file = pack_dir / "pack_name.txt";
        if (!fs::exists(pack_name_file) || !fs::is_regular_file(pack_name_file)) {
                if (!silent) {
                        minilog::fatal(1, "pack_name.txt does not exists or not a file");
                }
                return false;
        }

        std::ifstream pnfile(pack_name_file);

        std::string raw_name((std::istreambuf_iterator<char>(pnfile)), std::istreambuf_iterator<char>());
        std::string name = trim(raw_name);
        if (name.empty()) {
                if (!silent) {
                        minilog::fatal(1, "pack_name.txt is empty");
                }
                return false;
        }

        return true;
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

static bool check_git() {
#if defined(_WIN32)
        int status = std::system("where git > nul 2>&1");
#else
        int status = std::system("command -v git > /dev/null 2>&1");
#endif
        return (status == 0);
}

static void clone_git(const std::string &repo_url, const fs::path &target_dir) {
        minilog::out("\033[32m==>\033[0m Cloning ", repo_url, "...");
        std::string cmd =
            "git clone --depth 1 \"" + repo_url + "\" \"" + target_dir.string() + "\" 2>&1 | grep -v 'remote:'";
        int ret = std::system(cmd.c_str());
        if (ret != 0) {
                minilog::fatal(1, "Failed to clone: ", repo_url);
        }
}

static bool is_repo_exists(const std::string &repo) {
#ifdef WIN32
        return std::system(("git ls-remote --exit-code " + repo + " HEAD > nul 2>&1").c_str()) == 0;
#else
        return std::system(("git ls-remote --exit-code " + repo + " HEAD >/dev/null 2>&1").c_str()) == 0;
#endif
}

static std::string get_git_commit(const fs::path &repo_dir) {
        std::string cmd = "git -C " + repo_dir.string() + " rev-parse HEAD 2>/dev/null";

        std::array<char, 128> buffer;
        std::string result;

        using PipeDeleter = int (*)(FILE *);
        std::unique_ptr<FILE, PipeDeleter> pipe(popen(cmd.c_str(), "r"), pclose);

        if (!pipe) {
                return "";
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
        }

        return trim(result);
}

static bool check_git_update(const std::string &repo_url, const std::string &local_commit) {
        if (!check_git()) {
                return false;
        }

        std::string cmd = "git ls-remote " + repo_url + " HEAD 2>/dev/null";

        std::array<char, 128> buffer;
        std::string result;

        using PipeDeleter = int (*)(FILE *);
        std::unique_ptr<FILE, PipeDeleter> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
                return false;
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
        }

        std::stringstream ss(result);
        std::string remote_commit;
        ss >> remote_commit;

        remote_commit = trim(remote_commit);

        if (remote_commit.empty() || local_commit.empty()) {
                return false;
        }

        return (remote_commit != local_commit);
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

static void install(const fs::path &directory, const std::string &plugin_name, bool force) {
        fs::path plugins_dir = game::_data_directory / "plugins";

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
                if (!confirm(
                        "\033[32m==>\033[0m Remove it and continue to installation? [Y/n]: \n\033[32m==>\033[0m ")) {
                        minilog::out("\033[33m==>\033[0m Canceled...");
                        fs::remove_all(directory);
                        exit(0);
                }

                fs::remove_all(plugin_dir);
        }

install:
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
}

static std::string read_pack_name(const fs::path &dir) {
        std::ifstream file(dir / "pack_name.txt");
        std::string raw_name((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return trim(raw_name);
}

static std::pair<fs::path, std::string> prepare_file(const std::string &path, bool force) {
        fs::path temp_dir = create_temp_dir();

        extract_zip(path, temp_dir);

        check_package(temp_dir);

        std::ifstream file(temp_dir / "pack_name.txt");
        std::string raw_name((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        std::string plugin_name = read_pack_name(temp_dir);
        if (plugin_name.empty()) {
                fs::remove_all(temp_dir);
                minilog::fatal(1, "pack_name.txt is empty.");
        }

        return {temp_dir, plugin_name};
}

static std::pair<fs::path, std::string> prepare_git(const std::string &repo_url, bool force) {
        fs::path temp_dir = create_temp_dir();
        clone_git(repo_url, temp_dir);

        check_package(temp_dir);

        std::ifstream file(temp_dir / "pack_name.txt");
        std::string raw_name((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        std::string plugin_name = read_pack_name(temp_dir);
        if (plugin_name.empty()) {
                fs::remove_all(temp_dir);
                minilog::fatal(1, "pack_name.txt is empty.");
        }

        // save repo url & git commit to update later
        std::ofstream repo_file(temp_dir / "git_repo.json");
        json repo_info = json::object();
        repo_info["url"] = repo_url;
        repo_info["commit"] = get_git_commit(temp_dir);
        repo_file << repo_info.dump();
        repo_file.close();

        // remove .git directory
        fs::remove_all(temp_dir / ".git");

        return {temp_dir, plugin_name};
};

void install_plugins(const std::vector<std::string> &files, const std::vector<std::string> &repos, bool force) {
        for (const std::string &repo : repos) {
                if (!is_repo_exists(repo)) {
                        minilog::fatal(1, "Git repository doesn't exists: ", repo);
                };
        }

        for (const std::string &file : files) {
                if (!fs::exists(file) || !fs::is_regular_file(file)) {
                        minilog::fatal(1, "File doesn't exists or not a file: ", file);
                }
        }

        minilog::out("\033[34m::\033[0m Installing plugins: ");
        if (!repos.empty()) {
                minilog::out("    From git:");
        }
        for (const std::string &repo : repos) {
                minilog::out("      \033[90m->\033[0m ", repo);
        }

        if (!files.empty()) {
                minilog::out("    From file:");
        }
        for (const std::string &file : files) {
                minilog::out("      \033[90m->\033[0m ", file);
        }

        if (!force && !confirm("\033[32m==>\033[0m Continue to installation [Y/n]: \n\033[32m==>\033[0m ")) {
                exit(0);
        }

        minilog::out("\033[34m::\033[0m Preparing plugins...");
        std::vector<std::pair<fs::path, std::string>> prepared;
        for (const std::string &file : files) {
                minilog::out("\033[32m==>\033[0m Preparing file: ", file);
                prepared.push_back(prepare_file(file, force));
        }

        for (const std::string &repo : repos) {
                minilog::out("\033[32m==>\033[0m Preparing git: ", repo);
                prepared.push_back(prepare_git(repo, force));
        }

        minilog::out("\033[34m::\033[0m Installing plugins...");
        for (const auto p : prepared) {
                minilog::out("\033[32m==>\033[0m Installing: ", p.second);
                install(p.first, p.second, force);
        }
}

void update(std::vector<std::string> &packages) {
        fs::path plugins_dir(game::_data_directory / "plugins");
        if (packages.empty()) {
                // update all if no package given
                for (const auto &entry : std::filesystem::directory_iterator(plugins_dir)) {
                        fs::path path = entry.path();
                        if (fs::is_directory(path)) {
                                packages.push_back(path.filename().string());
                        }
                }
        }

        minilog::out("\033[34m::\033[0m Checking for updates...");
        bool no_update = true;
        std::vector<std::string> updates;
        for (auto pack : packages) {
                fs::path pack_dir = plugins_dir / pack;
                if (!fs::exists(pack_dir)) {
                        minilog::fatal(1, "Plugin \"", pack, "\" doesn't exists.");
                }

                if (!fs::exists(pack_dir / "git_repo.json")) {
                        minilog::out("\033[33m==> Plugin \"", pack, "\" is not installed via git -> ignoring\033[0m");
                        continue;
                }

                json repo_info;
                std::ifstream repo_info_file(pack_dir / "git_repo.json");
                repo_info_file >> repo_info;

                if (repo_info.contains("url") && repo_info.contains("commit")) {
                        if (check_git_update(repo_info.at("url"), repo_info.at("commit"))) {
                                minilog::out("\033[32m==>\033[0m Updating \"", pack, "\"");
                                updates.push_back(repo_info.at("url"));
                                no_update = false;
                        } else {
                                minilog::out("\033[32m==>\033[0m " + pack + " is at newest version");
                        }
                } else {
                        minilog::out("\033[33m==> Can't check updates for plugin \"" + pack + "\".\"" +
                                     (pack_dir / "git_repo.json").string() + "\" is invalid\033[0m");
                }
        }

        if (no_update) {
                minilog::out("\033[32m==>\033[0m Everything is up to date");
                return;
        }

        minilog::out("\033[34m::\033[0m Installing found updates...");
        install_plugins({}, updates, true);
}

void remove(const std::string &package, bool force) {
        fs::path pack(game::_data_directory / "plugins" / package);

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

void reset(const std::string &package, bool force) {
        fs::path settings(game::_data_directory / "plugins" / package / "settings.json");
        if (!confirm("\033[32m==>\033[0m Are you sure? [Y/n]\n\033[32m==>\033[0m")) {
                minilog::out("\033[33m==>\033[0m Canceled...");
                exit(0);
        }

        // do not matter exists or not
        minilog::out("\033[34m::\033[0m Resetting...");
        fs::remove(settings);
}

void list(json *output = nullptr) {
        fs::path plugins_dir(game::_data_directory / "plugins");
        for (const auto &entry : std::filesystem::directory_iterator(plugins_dir)) {
                fs::path path = entry.path();
                if (fs::is_directory(path)) {
                        std::string source = "local";

                        if (!check_package(path, true))
                                continue;

                        if (fs::exists(path / "git_repo.json")) {
                                try {
                                        std::ifstream file(path / "git_repo.json");
                                        json info;
                                        file >> info;
                                        file.close();
                                        source = info["url"].get<std::string>();
                                } catch (std::exception &e) {
                                        source = "git-unknown";
                                        continue;
                                }
                        }

                        if (output) {
                                (*output)[path.filename().string()] = source;
                        } else {
                                minilog::out("\033[36m" + path.filename().string());
                                minilog::out(" \033[90m-> \033[35m", source, "\033[0m");
                        }
                }
        }
}

}  // namespace package
