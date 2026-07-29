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

#include <ncurses.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sol/sol.hpp>
#include <sstream>

#include "cards.hpp"
#include "game.hpp"
#include "minilog.hpp"
#include "package.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;

void cleanup_lua() {
        // Clear all vectors that contain objects with Lua references
        game::buffs.clear();
        game::deck.clear();
        game::card_set.clear();
        game::biomes.clear();
        game::levels.clear();
        game::player::inventory.clear();

        // Clear card slots which also contain card_t with Lua references
        game::slot1.back = nullptr;
        game::slot1.front = nullptr;
        game::slot2.back = nullptr;
        game::slot2.front = nullptr;
        game::slot3.back = nullptr;
        game::slot3.front = nullptr;

        // Cleaanup package.laoded
        game::lua.script(
            "for k in pairs(package.loaded) do "
            "  if k ~= '_G' and k ~= 'package' and k ~= 'table' and k ~= 'string' and k ~= 'math' then "
            "    package.loaded[k] = nil "
            "  end "
            "end");

        // Force Lua garbage collection to clean up any remaining references
        game::lua.collect_garbage();

        minilog::fdebugc("lua", logfile, "Cleaned Lua");
}

void end_program() {
        cleanup_lua();
        endwin();
}

/*
 * Main Job
 */

json load_json(const fs::path &path) {
        std::ifstream metadata_file(path);

        if (!fs::exists(path)) {
                throw std::runtime_error("File doesn't exits: " + path.string());
        }

        if (!metadata_file.is_open()) {
                throw std::runtime_error("Can't open file: " + path.string());
        }

        std::stringstream ss;
        ss << metadata_file.rdbuf();

        // if empty create an empty json
        if (ss.str().empty()) {
                std::ofstream file(path);
                file << "{}";
                file.close();
                return json::object();
        }

        return json::parse(ss.str());
}

// clang-format off
json safe_load(const std::string pluginname, const fs::path path) {
        try {
                return load_json(path);

        } catch (json::parse_error &e) {
                game::plugin_errors[pluginname] = "E: While parsing \"" + path.string() +  "\": " + std::string(e.what());

        } catch (std::exception &e) {
                game::plugin_errors[pluginname] = "E: While loading \"" + path.string() +  "\": " + std::string(e.what());
        }

        return json{};
}

// clang-format on

std::string check_metadata(const json &metadata) {
        if (!metadata.contains("name")) {
                return "does not contains \"name\" key";
        }

        if (!metadata.contains("description")) {
                return "does not contains \"description\" key";
        }

        if (!metadata["name"].is_string()) {
                return "\"name\" key is not string";
        }

        if (!metadata["description"].is_string()) {
                return "\"description\" key is not string";
        }

        if (metadata.contains("settings")) {
                if (!metadata["settings"].is_array()) {
                        return "\"settings\" key is not array";
                }

                if (metadata["settings"].empty()) {
                        return "\"settings\" key is empty";
                }

                for (auto s : metadata["settings"]) {
                        std::vector<std::string> required_keys = {"type", "name", "description", "option"};

                        for (auto key : required_keys) {
                                if (!s.contains(key) || !s[key].is_string()) {
                                        return "\"key\" is not found / has invalid type on a setting";
                                }
                        }

                        if (!s.contains("default")) {
                                return "a setting does not contains \"default\" key";
                        }

                        std::string type = s["type"].get<std::string>();
                        if (type == "switch") {
                                if (!s["default"].is_boolean()) {
                                        return "default value type of a switch setting is wrong";
                                }

                        } else if (type == "input") {
                                if (!s["default"].is_string()) {
                                        return "default value type of a input setting is wrong";
                                }

                        } else if (type == "choose") {
                                if (!s["default"].is_string()) {
                                        return "default value type of a choose setting is wrong";
                                }

                                if (!s.contains("values") || !s["values"].is_array()) {
                                        return "a choose setting does not contains \"values\" key";
                                }

                                for (auto v : s["values"]) {
                                        if (!v.is_string()) {
                                                return "a value of choose setting is not a string";
                                        }
                                }

                                bool found = false;
                                for (const auto &val : s["values"]) {
                                        if (val == s["default"]) {
                                                found = true;
                                                break;
                                        }
                                }

                                if (!found) {
                                        return "default value of a choose setting is not found in its values array";
                                }
                        } else {
                                return "a setting has a invalid type";
                        }
                }
        }

        return "";
}

std::string check_settings(const json &settings, const json &metadata) {
        if (!settings.is_object()) {
                for (auto [key, value] : metadata.items()) {
                        if (!metadata.contains(key)) {
                                if (metadata[key]["type"].get<std::string>() == "switch") {
                                        if (!value.is_boolean()) {
                                                return "value of " + key + "is not boolean";
                                        }

                                } else if (metadata[key]["type"].get<std::string>() == "input") {
                                        if (!value.is_string()) {
                                                return "value of " + key + "is not string";
                                        }

                                } else if (metadata[key]["type"].get<std::string>() == "choose") {
                                        if (!value.is_string()) {
                                                return "value of " + key + "is not string";
                                        }

                                        if (!metadata[key]["values"].contains(value)) {
                                                return "value of " + key + "is not one of the values in metadata.json";
                                        }
                                }
                        }
                }
        }

        return "";
}

void load_plugin(const fs::path &plugindir) {
        std::string pluginname = plugindir.filename().string();

        // load lua script
        fs::path initfile = plugindir / "init.lua";
        minilog::fdebugc("lua", logfile, "Loading plugin: ", pluginname);

        // update path
        std::string original_path = game::lua["package"]["path"];
        std::string plugin_path = plugindir.string() + "/?.lua;" + original_path;
        game::lua["package"]["path"] = plugin_path;

        // call the init.lua and handle errors
        sol::protected_function_result result = game::lua.safe_script_file(initfile, &sol::script_pass_on_error);
        if (!result.valid()) {
                sol::error err = result;
                game::plugin_errors[pluginname] = "E: " + std::string(err.what());
                minilog::fdebugc("lua", logfile, minilog::msg::error, "In plugin: ", pluginname,
                                 " Error: ", err.what());
        }

        // reload original_path
        game::lua["package"]["path"] = original_path;
}

fs::path create_plugins_dir() {
        fs::path plugins_directory;

        try {
                plugins_directory = get_data_dir() / "plugins";

                if (!fs::exists(plugins_directory)) {
                        minilog::fdebugc("lua", logfile, "Creating plugins directory: ", plugins_directory);
                        fs::create_directories(plugins_directory);
                }

        } catch (const std::exception &e) {
                minilog::fdebugc("lua", logfile, minilog::msg::fatal,
                                 "Error while accessing plugins directory: ", e.what());
                end_program();
                minilog::fatal(1, "Error while accessing plugins directory: ", e.what());
        }

        if (!fs::is_directory(plugins_directory)) {
                minilog::fdebugc("lua", logfile, minilog::msg::fatal, '"', plugins_directory, "\" is not a directory.");
                end_program();
                minilog::fatal(1, '"', plugins_directory, "\" is not a directory.");
        }

        return plugins_directory;
}

fs::path get_settings_path() {
        return get_data_dir() / "settings.json";
}

void load_settings() {
        fs::path settings_path = get_settings_path();

        if (!fs::exists(settings_path)) {
                std::ofstream settings_file(settings_path);
                if (!settings_file.is_open()) {
                        end_program();
                        minilog::fatal(1, "Can't open file: \"" + settings_path.string() + "\"");
                }
                settings_file << "{}";
                return;
        }

        std::ifstream settings_file(settings_path);
        if (!settings_file.is_open()) {
                end_program();
                minilog::fatal(1, "Can't open file: \"" + settings_path.string() + "\"");
        }

        try {
                json parsed = json::parse(settings_file);
                for (auto &[key, value] : parsed.items()) {
                        game::settings::settings[key]["enabled"] = value;
                }
                minilog::fdebugc("settings", logfile, "loading settings.json");
        } catch (json::parse_error &e) {
                minilog::fdebug(logfile, minilog::msg::error, "While parsing settings.json: ", e.what());
        }
}

void save_settings() {
        fs::path settings_path = get_settings_path();
        std::ofstream settings_file(settings_path);

        if (!settings_file.is_open()) {
                end_program();
                minilog::fatal(1, "Can't open file: \"" + settings_path.string() + "\"");
        }

        json settings_to_write = json::object();
        for (auto &[plugin_name, config] : game::settings::settings.items()) {
                if (config.contains("enabled")) {
                        settings_to_write[plugin_name] = config["enabled"];
                }
        }

        settings_file << settings_to_write.dump(4);
}

void cleanup_orphaned_settings(const fs::path &plugins_directory) {
        std::vector<std::string> existing_plugins;
        for (const auto &entry : fs::directory_iterator(plugins_directory)) {
                if (entry.is_directory() && fs::exists(entry.path() / "init.lua")) {
                        existing_plugins.push_back(entry.path().filename().string());
                }
        }

        std::vector<std::string> to_remove;
        for (auto &[key, _] : game::settings::settings.items()) {
                if (std::find(existing_plugins.begin(), existing_plugins.end(), key) == existing_plugins.end()) {
                        to_remove.push_back(key);
                }
        }

        for (const auto &key : to_remove) {
                game::settings::settings.erase(key);
                game::settings::metadata.erase(key);
        }
}

void load_plugin_config(const std::string &pluginname, const fs::path &subpath) {
        fs::path plugin_settings_path = subpath / "settings.json";

        if (fs::exists(plugin_settings_path)) {
                json plugin_cfg = safe_load(pluginname, plugin_settings_path);
                if (plugin_cfg.is_object()) {
                        for (auto &[key, value] : plugin_cfg.items()) {
                                if (key != "enabled") {
                                        game::settings::settings[pluginname][key] = value;
                                }
                        }
                }
        } else {
                minilog::fdebugc("settings", logfile, "Generating default settings.json file for ", pluginname);
                for (const auto &setting : game::settings::metadata[pluginname]["settings"]) {
                        game::settings::settings[pluginname][setting.at("option")] = setting.at("default");
                }

                std::ofstream file(plugin_settings_path);
                json settings = game::settings::settings[pluginname];
                settings.erase("enabled");
                file << settings.dump(4);
        }
}

void process_single_plugin(const fs::path &subpath) {
        if (!fs::is_directory(subpath) || !fs::exists(subpath / "init.lua"))
                return;

        std::string pluginname = subpath.filename().string();

        // 1. Metadata Validation
        json metadata = safe_load(pluginname, subpath / "metadata.json");
        std::string error = check_metadata(metadata);
        if (!error.empty()) {
                game::plugin_errors[pluginname] = "E: metadata.json: " + error;
                game::settings::settings.erase(pluginname);
                game::settings::metadata.erase(pluginname);
                return;
        }

        game::settings::metadata[pluginname] = metadata;

        load_plugin_config(pluginname, subpath);

        bool is_enabled = true;
        if (game::settings::settings.contains(pluginname) && game::settings::settings[pluginname].contains("enabled")) {
                is_enabled = game::settings::settings[pluginname]["enabled"].get<bool>();
        }

        if (!game::settings::settings.contains(pluginname) || !game::settings::settings[pluginname].is_object()) {
                game::settings::settings[pluginname] = json::object();
        }

        game::settings::settings[pluginname]["enabled"] = is_enabled;

        if (is_enabled) {
                load_plugin(subpath);
        }
}

// --- Main Logic ---

void load_plugins() {
        minilog::fdebugc("lua", logfile, "Loading plugins");

        create_card(1, "~ Exit Gate ~", EXIT, {}, "", 0, 0, exit_gate);

        load_settings();

        fs::path plugins_directory = create_plugins_dir();

        cleanup_orphaned_settings(plugins_directory);

        for (const fs::path &subpath : fs::directory_iterator(plugins_directory)) {
                process_single_plugin(subpath);
        }

        save_settings();
}
