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

#include <filesystem>
#include <sol/sol.hpp>

#include "cards.hpp"
#include "minilog.hpp"
#include "tui.hpp"
#include "types.hpp"

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

        // Force Lua garbage collection to clean up any remaining references
        game::lua.collect_garbage();
}  // namespace std::filesystem

/*
 * Getters and Setters
 */

int get_player_hp() {
        return game::player::hp;
}

void set_player_hp(int value) {
        game::player::hp = value;
}

int get_player_level() {
        return game::player::level;
}

void set_player_level(int value) {
        game::player::level = value;
}

std::string get_seed() {
        return std::to_string(game::seed);
}

int get_levelid() {
        return game::levelid;
}

void set_levelid(int value) {
        game::levelid = value;
}

void set_seed(const std::string &value) {
        try {
                game::seed = std::stoull(value);
        } catch (const std::invalid_argument &e) {
                throw sol::error::runtime_error(
                    "Invalid seed value, must be a string with numbers (characters will be ignored)");
        } catch (const std::out_of_range &e) {
                throw sol::error::runtime_error("Seed value is too big (max: 18446744073709551615).");
        }
}

/*
 * Setup
 */

void setup_lua() {
        game::lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::package, sol::lib::string, sol::lib::math);
        // clang-format off

        // main table
        sol::table crogue = game::lua.create_table();

        /*
         * Enums
         */

        crogue.new_enum("card_type",
                        "BASIC", card_type::BASIC,
                        "ITEM", card_type::ITEM,
                        "ENEMY", card_type::ENEMY,
                        "EXIT", card_type::EXIT);

        crogue.new_enum("log_type",
                        "NORMAL", log_type::NORMAL,
                        "WARN", log_type::WARN,
                        "IMPORTANT", log_type::IMPORTANT);

        /*
         * Types
         */

        sol::table objects = game::lua.create_table();

        objects.new_usertype<card_t>("card", sol::constructors<card_t>(),
                        "count", &card_t::count,
                        "name", &card_t::name,
                        "type", &card_t::type,
                        "level_ids", &card_t::level_ids,
                        "logmsg", &card_t::logmsg,
                        "ttl", &card_t::ttl,
                        "event", sol::property(
                                // getter
                                [](card_t& c) -> std::function<int()>& { return c.event; },
                                // setter
                                [](card_t& c, std::function<int()> f) { c.event = f; }
                                ));

        objects.new_usertype<card_slot_t>("card_slot", sol::constructors<card_slot_t>(),
                        "back", &card_slot_t::back,
                        "front", &card_slot_t::front,
                        "_lived", &card_slot_t::_lived);

        objects.new_usertype<level_t>("level", sol::constructors<level_t>(),
                        "name", &level_t::name,
                        "id", &level_t::id);

        objects.new_usertype<biome_t>("biome", 
                        sol::constructors<biome_t>(), 
                        "difficulty", &biome_t::difficulty,
                        "levels", &biome_t::levels
                        );

        objects.new_usertype<buff_t>("buff", sol::constructors<buff_t>(),
                        "name", &buff_t::name,
                        "event", sol::property(
                                [](buff_t& b) -> std::function<void(std::shared_ptr<buff_t>)>& { return b.event; },
                                [](buff_t& b, std::function<void(std::shared_ptr<buff_t>)> f) { b.event = f; }
                                ),
                        "level", &buff_t::level);

        crogue["obj"] = objects;

        // clang-format on

        /*
         * Functions
         */

        crogue["ask"] = &ask;
        crogue["ask_string"] = &ask_string;

        crogue["log"] = [](const std::string &msg, log_type type) { log(msg, type); };

        crogue["create_card"] = [](sol::table table) { return create_card(table); };
        crogue["create_buff"] = [](sol::table table) { return create_buff(table); };
        crogue["create_level"] = &create_level;
        crogue["create_biome"] = [](sol::table table) { return create_biome(table); };

        sol::table shared = game::lua.create_table();

        shared["card"] = [](const card_t &card) { return std::make_shared<card_t>(card); };
        shared["buff"] = [](const buff_t &buff) { return std::make_shared<buff_t>(buff); };
        shared["level"] = [](const level_t &level) { return std::make_shared<level_t>(level); };
        shared["biome"] = [](const biome_t &biome) { return std::make_shared<biome_t>(biome); };

        crogue["shared"] = shared;


        /*
         * Variables
         */

        sol::table stat = game::lua.create_table();

        stat["deck"] = std::ref(game::deck);
        stat["card_set"] = std::ref(game::card_set);
        stat["slot1"] = std::ref(game::slot1);
        stat["slot2"] = std::ref(game::slot2);
        stat["slot3"] = std::ref(game::slot3);
        stat["biomes"] = std::ref(game::biomes);
        stat["levels"] = std::ref(game::levels);
        stat["buffs"] = std::ref(game::buffs);
        stat["logs"] = std::ref(game::logs);

        stat["get_levelid"] = &get_levelid;
        stat["set_levelid"] = &set_levelid;
        stat["get_seed"] = &get_seed;
        stat["set_seed"] = &set_seed;

        crogue["stat"] = stat;


        sol::table player = game::lua.create_table();

        player["get_hp"] = &get_player_hp;
        player["set_hp"] = &set_player_hp;

        player["get_level"] = &get_player_level;
        player["set_level"] = &set_player_level;

        crogue["player"] = player;

        game::lua["cr"] = crogue;
}

void load_plugin(const fs::path &plugindir) {
        fs::path initfile = plugindir / "init.lua";

        // update path
        std::string original_path = game::lua["package"]["path"];
        std::string plugin_path = plugindir.string() + "/?.lua;" + original_path;
        game::lua["package"]["path"] = plugin_path;

        // call the init.lua and handle errors
        sol::protected_function_result result = game::lua.safe_script_file(initfile, &sol::script_pass_on_error);
        if (!result.valid()) {
                sol::error err = result;
                std::string pluginname = plugindir.filename().string();
                game::plugin_errors[pluginname] = "Lua script error: " + std::string(err.what());
                minilog::fdebugc("lua", logfile, minilog::msg::error, "In plugin: ", pluginname,
                                 " Error: ", err.what());
        }

        // reload original_path
        game::lua["package"]["path"] = original_path;
}

void load_plugins() {
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

        // load plugins
        for (const fs::path &subpath : fs::directory_iterator(plugins_directory)) {
                if (!fs::is_directory(subpath) || !fs::exists(subpath / "init.lua") ||
                    !fs::is_regular_file(subpath / "init.lua")) {
                        continue;
                }

                load_plugin(subpath);
        }
}
