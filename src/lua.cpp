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
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sol/sol.hpp>
#include <sstream>

#include "game.hpp"
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

        minilog::fdebugc("setup", logfile, "Cleaned Lua");
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


// function moved to end of file
// it was too long
void setup_lua();

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

void load_plugins() {
        minilog::fdebugc("lua", logfile, "Loading plugins");

        // load game settings
        fs::path settings_path = get_data_dir() / "settings.json";
        json game_settings = json::object();
        if (!fs::exists(settings_path)) {
                // if does not exits, fill with empty value
                std::ofstream settings_file(settings_path);

                if (!settings_file.is_open()) {
                        endwin();
                        minilog::fatal(1, "Cant't open file: \"" + settings_path.string() + "\"");
                }

                settings_file << "{}";
                settings_file.close();
        } else {
                std::ifstream settings_file(settings_path);

                if (!settings_file.is_open()) {
                        cleanup_lua();
                        endwin();
                        // TODO: create file empty
                        minilog::fatal(1, "Cant't open file: \"" + settings_path.string() + "\"");
                }

                try {
                        game_settings = json::parse(settings_file);
                } catch (json::parse_error &e) {
                        minilog::fdebug(logfile, minilog::msg::error, "While parsing settings.json: ", e.what());
                }
                minilog::fdebug(logfile, "loading settings.json");
        }

        // get enabled values
        for (auto &[key, value] : game_settings.items()) {
                game::settings::settings[key]["enabled"] = value;
        }

        // create plugins directory
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
                cleanup_lua();
                endwin();
                minilog::fatal(1, "Error while accessing plugins directory: ", e.what());
        }

        if (!fs::is_directory(plugins_directory)) {
                minilog::fdebugc("lua", logfile, minilog::msg::fatal, '"', plugins_directory, "\" is not a directory.");
                cleanup_lua();
                endwin();
                minilog::fatal(1, '"', plugins_directory, "\" is not a directory.");
        }

        // clean up non existing plugins

        std::vector<std::string> existing_plugins;
        for (const auto &entry : fs::directory_iterator(plugins_directory)) {
                if (entry.is_directory() && fs::exists(entry.path() / "init.lua")) {
                        existing_plugins.push_back(entry.path().filename().string());
                }
        }

        std::vector<std::string> to_remove;
        for (auto &[key, value] : game::settings::settings.items()) {
                if (std::find(existing_plugins.begin(), existing_plugins.end(), key) == existing_plugins.end()) {
                        to_remove.push_back(key);
                }
        }

        for (const auto &key : to_remove) {
                game::settings::settings.erase(key);
                game::settings::metadata.erase(key);
                game_settings.erase(key);
        }


        // load plugins
        for (const fs::path &subpath : fs::directory_iterator(plugins_directory)) {
                if (!fs::is_directory(subpath) || !fs::exists(subpath / "init.lua"))
                        continue;

                std::string pluginname = subpath.filename().string();

                // true by default
                bool is_enabled = game_settings.contains(pluginname) ? game_settings[pluginname].get<bool>() : true;

                // create pluginname key
                if (!game::settings::settings.contains(pluginname) ||
                    !game::settings::settings[pluginname].is_object()) {
                        game::settings::settings[pluginname] = json::object();
                }

                // set enabled
                game::settings::settings[pluginname]["enabled"] = is_enabled;

                if (is_enabled) {
                        load_plugin(subpath);
                }

                // plugin metadata
                // TODO: add a check_metadata function
                game::settings::metadata[pluginname] = safe_load(pluginname, subpath / "metadata.json");

                // plugin settings
                if (fs::exists(subpath / "settings.json")) {
                        json plugin_cfg = safe_load(pluginname, subpath / "settings.json");
                        if (plugin_cfg.is_object()) {
                                for (auto &[key, value] : plugin_cfg.items()) {
                                        if (key != "enabled") {  // do not override enabled
                                                game::settings::settings[pluginname][key] = value;
                                        }
                                }
                        }
                } else {
                        minilog::fdebugc("settings", logfile, "Generating default settings.json file for ", pluginname);
                        for (auto setting : game::settings::metadata[pluginname]["settings"]) {
                                game::settings::settings[pluginname][setting.at("option")] = setting.at("default");
                        }

                        std::ofstream file(subpath / "settings.json");
                        json settings = game::settings::settings[pluginname];
                        settings.erase("enabled");
                        file << settings.dump(4);
                        file.close();
                }
        }

        // write final version
        std::ofstream settings_file(settings_path);
        if (!settings_file.is_open()) {
                endwin();
                minilog::fatal(1, "Cant't open file: \"" + settings_path.string() + "\"");
        }

        settings_file << game_settings.dump(4);
        settings_file.close();
}
