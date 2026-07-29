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
json safe_load(const std::string plugin_name, const fs::path path) {
        try {
                return load_json(path);

        } catch (json::parse_error &e) {
                game::plugin_errors[plugin_name] = "E: While parsing \"" + path.string() +  "\": " + std::string(e.what());

        } catch (std::exception &e) {
                game::plugin_errors[plugin_name] = "E: While loading \"" + path.string() +  "\": " + std::string(e.what());
        }

        return json::object();
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
                                        return "\"" + key + "\" is not found / has invalid type on a setting";
                                }
                        }

                        if (s["option"] == "enabled") {
                                return "a settings can't use \"enabled\" as option name";
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
                return "settings is not a JSON object";
        }

        // check settings enabled
        if (!metadata.contains("settings") || !metadata["settings"].is_array()) {
                return "";
        }

        const auto &meta_settings = metadata["settings"];

        for (const auto &s : meta_settings) {
                if (!s.contains("option") || !s["option"].is_string()) {
                        continue;
                }

                std::string option_key = s["option"].get<std::string>();

                if (!settings.contains(option_key)) {
                        return "missing setting option: \"" + option_key + "\"";
                }

                const auto &val = settings[option_key];
                std::string type = s["type"].get<std::string>();

                // check type and value
                if (type == "switch") {
                        if (!val.is_boolean()) {
                                return "value of switch option \"" + option_key + "\" must be boolean";
                        }
                } else if (type == "input") {
                        if (!val.is_string()) {
                                return "value of input option \"" + option_key + "\" must be string";
                        }
                } else if (type == "choose") {
                        if (!val.is_string()) {
                                return "value of choose option \"" + option_key + "\" must be string";
                        }

                        std::string val_str = val.get<std::string>();
                        bool is_valid_choice = false;

                        if (s.contains("values") && s["values"].is_array()) {
                                for (const auto &v : s["values"]) {
                                        if (v == val_str) {
                                                is_valid_choice = true;
                                                break;
                                        }
                                }
                        }

                        if (!is_valid_choice) {
                                return "value \"" + val_str + "\" for choose option \"" + option_key +
                                       "\" is not in metadata values list";
                        }
                }
        }

        return "";
}

void load_plugin(const fs::path &plugindir) {
        std::string plugin_name = plugindir.filename().string();

        // load lua script
        fs::path initfile = plugindir / "init.lua";
        minilog::fdebugc("lua", logfile, "Loading plugin: ", plugin_name);

        // update path
        std::string original_path = game::lua["package"]["path"];
        std::string plugin_path = plugindir.string() + "/?.lua;" + original_path;
        game::lua["package"]["path"] = plugin_path;

        // call the init.lua and handle errors
        sol::protected_function_result result = game::lua.safe_script_file(initfile, &sol::script_pass_on_error);
        if (!result.valid()) {
                sol::error err = result;
                game::plugin_errors[plugin_name] = "E: " + std::string(err.what());
                minilog::fdebugc("lua", logfile, minilog::msg::error, "In plugin: ", plugin_name,
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

void load_settings() {
        fs::path settings_path = get_data_dir() / "settings.json";

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
        fs::path settings_path = get_data_dir() / "settings.json";
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

// remove invalid/removed plugin's settings
void cleanup_orphaned_settings(const fs::path &plugins_directory) {
        std::vector<std::string> existing_plugins;
        for (const auto &entry : fs::directory_iterator(plugins_directory)) {
                if (package::check_package(entry, true)) {
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

void generate_default_settings(const std::string &plugin, const fs::path &path) {
        fs::path plugin_settings_path = path / "settings.json";

        minilog::fdebugc("settings", logfile, "Generating default settings.json file for ", plugin);

        // Objenin ilklendirildiğinden emin ol
        if (!game::settings::settings.contains(plugin) || !game::settings::settings[plugin].is_object()) {
                game::settings::settings[plugin] = json::object();
        }

        // metadata içinde "settings" dizisi var mı kontrol et
        if (game::settings::metadata[plugin].contains("settings") &&
            game::settings::metadata[plugin]["settings"].is_array()) {
                for (const auto &setting : game::settings::metadata[plugin]["settings"]) {
                        if (setting.contains("option") && setting.contains("default")) {
                                game::settings::settings[plugin][setting.at("option")] = setting.at("default");
                        }
                }
        }

        std::ofstream file(plugin_settings_path);
        json settings = game::settings::settings[plugin];

        // Sadece geçerli bir json objesiyse ve "enabled" içeriyorsa sil
        if (settings.is_object() && settings.contains("enabled")) {
                settings.erase("enabled");
        }

        file << settings.dump(4);
}

// WARN: must be runned after metadata loaded
void load_plugin_config(const std::string &plugin_name, const fs::path &subpath) {
        fs::path plugin_settings_path = subpath / "settings.json";

        if (fs::exists(plugin_settings_path)) {
                json plugin_cfg = safe_load(plugin_name, plugin_settings_path);

                std::string error = check_settings(plugin_cfg, game::settings::metadata[plugin_name]);
                if (!error.empty()) {
                        generate_default_settings(plugin_name, subpath);
                }

                plugin_cfg = safe_load(plugin_name, plugin_settings_path);

                // copy plugin settings to game::settings::settings without overriding enabled key
                if (plugin_cfg.is_object()) {
                        for (auto &[key, value] : plugin_cfg.items()) {
                                if (key != "enabled") {
                                        game::settings::settings[plugin_name][key] = value;
                                }
                        }
                }
        } else {
                generate_default_settings(plugin_name, subpath);
        }
}

void process_single_plugin(const fs::path &subpath) {
        if (!fs::is_directory(subpath) || !fs::exists(subpath / "init.lua"))
                return;

        std::string plugin_name = subpath.filename().string();

        // Metadata Validation
        json metadata = safe_load(plugin_name, subpath / "metadata.json");
        std::string error = check_metadata(metadata);
        if (!error.empty()) {
                game::plugin_errors[plugin_name] = "E: metadata.json: " + error;
                game::settings::settings.erase(plugin_name);
                game::settings::metadata.erase(plugin_name);
                return;
        }

        game::settings::metadata[plugin_name] = metadata;

        // Loading Settings
        load_plugin_config(plugin_name, subpath);

        bool is_enabled = true;
        if (game::settings::settings.contains(plugin_name) &&
            game::settings::settings[plugin_name].contains("enabled")) {
                is_enabled = game::settings::settings[plugin_name]["enabled"].get<bool>();
        }

        if (!game::settings::settings.contains(plugin_name) || !game::settings::settings[plugin_name].is_object()) {
                game::settings::settings[plugin_name] = json::object();
        }

        game::settings::settings[plugin_name]["enabled"] = is_enabled;

        // Load actual lua code
        if (is_enabled) {
                load_plugin(subpath);
        }
}

// Main Logic

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
