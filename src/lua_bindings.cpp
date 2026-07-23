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

#include <sol/sol.hpp>

#include "cards.hpp"
#include "game.hpp"
#include "lua.hpp"
#include "minilog.hpp"
#include "tui.hpp"

// clang-format off
#define SHARED_PROPERTY(self_type, type, member)\
        #member, sol::property(\
                        [](self_type& self) -> std::shared_ptr<type> { return self.member; },\
                        [](self_type& self, sol::optional<std::shared_ptr<type>> value) {\
                                self.member = value.value_or(nullptr);\
                        }\
                )
// clang-format on

/*
 * Wrapper Things
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

std::shared_ptr<scene_t> create_scene(sol::table table) {
        try {
                scene_t new_scene;
                new_scene.ui_refresh = table.get<std::function<void(void)>>("ui_refresh");
                new_scene.key_handler = table.get<std::function<bool(int)>>("key_handler");
                new_scene.exit_key = table.get_or<int>("exit_key", 'q');

                return std::make_shared<scene_t>(new_scene);
        } catch (const sol::error &e) {
                minilog::fdebug(logfile, minilog::msg::error, "Error in plugin", e.what());
                throw sol::error::runtime_error("Invalid table");
        }

        return nullptr;
}

sol::table get_settings(const std::string plugin) {
        sol::table table = game::lua.create_table();

        minilog::fdebug(logfile, "plugin name: '", plugin, "'");

        if (!game::settings::settings.contains(plugin)) {
                throw sol::error::runtime_error("Invalid plugin name");
        }

        auto plugin_settings = game::settings::settings[plugin];
        for (auto [key, value] : plugin_settings.items()) {
                if (value.is_boolean()) {
                        bool v = value.get<bool>();
                        table[key] = v;
                } else if (value.is_string()) {
                        std::string v = value.get<std::string>();
                        table[key] = v;
                }
        }

        return table;
}

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
                        "power", &card_t::power,
                        // TODO: this may be buggy check this
                        "event", sol::property(
                                // getter
                                [](card_t& c) -> std::function<int()>& { return c.event; },
                                // setter
                                [](card_t& c, std::function<int()> f) { c.event = f; }
                                ));

        objects.new_usertype<card_slot_t>("card_slot", sol::constructors<card_slot_t>(),
                        SHARED_PROPERTY(card_slot_t, card_t, back),
                        SHARED_PROPERTY(card_slot_t, card_t, front),
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

        objects.new_usertype<scene_t>("scene", sol::constructors<scene_t>(),
                        "exit_key", &scene_t::exit_key,
                        "ui_refresh", &scene_t::ui_refresh,
                        "key_handler", &scene_t::key_handler,
                        "run", &scene_t::run);

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
        crogue["create_scene"] = &create_scene;

        crogue["reset_game"] = &reset_game;
        crogue["generate_levels"] = &generate_levels;

        crogue["draw_cards"] = &draw_cards;
        crogue["draw_slots"] = &draw_slots;

        crogue["handle_slot"] = &handle_slot;
        crogue["handle_buffs"] = &handle_buffs;

        crogue["basic_card_event"] = &basic_card_event;
        crogue["card_event"] = &card_event;

        crogue["settings"] = &get_settings;

        crogue["hook"] = [](std::string event, sol::function func) {
                try {
                        if (event == "before_refresh") {
                                game::hooks::before_refresh.push_back(func.as<std::function<void(void)>>());

                        } else if (event == "after_refresh") {
                                game::hooks::after_refresh.push_back(func.as<std::function<void(void)>>());

                        } else if (event == "start") {
                                game::hooks::start.push_back(func.as<std::function<void(void)>>());

                        } else if (event == "end") {
                                game::hooks::end.push_back(func.as<std::function<void(void)>>());

                        } else if (event == "reload") {
                                game::hooks::reload.push_back(func.as<std::function<void(void)>>());

                        } else if (event == "key") {
                                game::hooks::key.push_back(func.as<std::function<void(int)>>());

                        } else if (event == "level_up") {
                                game::hooks::level_up.push_back(func.as<std::function<void(int)>>());

                        } else if (event == "slot") {
                                game::hooks::slot.push_back(func.as<std::function<bool(int)>>());

                        } else if (event == "item") {
                                game::hooks::item.push_back(func.as<std::function<bool(std::shared_ptr<card_t>)>>());

                        } else if (event == "draw") {
                                game::hooks::draw.push_back(func.as<std::function<void(void)>>());

                        } else if (event == "level_gen") {
                                game::hooks::level_gen.push_back(func.as<std::function<void(void)>>());

                        } else if (event == "card_event") {
                                game::hooks::card_event.push_back(
                                    func.as<std::function<bool(std::shared_ptr<card_t>, int)>>());
                        } else {
                                throw sol::error::runtime_error("Invalid hook name!: " + event);
                        }

                        minilog::fdebugc("lua", logfile, "Added a ", event, " hook");
                } catch (sol::error &e) {
                        throw sol::error::runtime_error("Invalid hook function type");
                }
        };

        // Shared

        sol::table shared = game::lua.create_table();

        shared["card"] = [](const card_t &card) { return std::make_shared<card_t>(card); };
        shared["buff"] = [](const buff_t &buff) { return std::make_shared<buff_t>(buff); };
        shared["level"] = [](const level_t &level) { return std::make_shared<level_t>(level); };
        shared["biome"] = [](const biome_t &biome) { return std::make_shared<biome_t>(biome); };

        crogue["shared"] = shared;

        // TUI

        sol::table tui = game::lua.create_table();

        tui["print_ansi"] = &print_ansi;
        tui["print_line"] = &print_line;
        tui["print_slots"] = &print_slots;
        tui["print_stats"] = &print_stats;
        tui["print_buffs"] = &print_buffs;
        tui["print_logs"] = &print_logs;
        tui["print_inventory"] = &print_inventory;
        tui["print_all"] = &print_ui;

        crogue["tui"] = tui;

        // Curses

        sol::table curses = game::lua.create_table();
        curses.new_usertype<attr_t>("attr_t", sol::constructors<attr_t>());

        curses["ansi2attr"] = &parse_ansi_color;

        curses["move"] = &move;
        curses["printw"] = &printw;
        curses["mvprintw"] = &mvprintw;
        curses["clear"] = &clear;
        curses["refresh"] = &refresh;
        curses["getch"] = &getch;

        curses["attron"] = &attron;
        curses["attrset"] = &attrset;
        curses["attroff"] = &attroff;

        curses["curs_set"] = &curs_set;

        curses["getmaxyx"] = []() {
                int y, x;
                getmaxyx(stdscr, y, x);
                return game::lua.create_table_with("y", y, "x", x);
        };

        // KEY_*
        curses["KEY_CODE_YES"] = KEY_CODE_YES;
        curses["KEY_MIN"] = KEY_MIN;
        curses["KEY_BREAK"] = KEY_BREAK;
        curses["KEY_SRESET"] = KEY_SRESET;
        curses["KEY_RESET"] = KEY_RESET;
        curses["KEY_DOWN"] = KEY_DOWN;
        curses["KEY_UP"] = KEY_UP;
        curses["KEY_LEFT"] = KEY_LEFT;
        curses["KEY_RIGHT"] = KEY_RIGHT;
        curses["KEY_HOME"] = KEY_HOME;
        curses["KEY_BACKSPACE"] = KEY_BACKSPACE;
        curses["KEY_F0"] = KEY_F(0);
        curses["KEY_F"] = [](int n) { return KEY_F(n); };
        curses["KEY_DL"] = KEY_DL;
        curses["KEY_IL"] = KEY_IL;
        curses["KEY_DC"] = KEY_DC;
        curses["KEY_IC"] = KEY_IC;
        curses["KEY_EIC"] = KEY_EIC;
        curses["KEY_CLEAR"] = KEY_CLEAR;
        curses["KEY_EOS"] = KEY_EOS;
        curses["KEY_EOL"] = KEY_EOL;
        curses["KEY_SF"] = KEY_SF;
        curses["KEY_SR"] = KEY_SR;
        curses["KEY_NPAGE"] = KEY_NPAGE;
        curses["KEY_PPAGE"] = KEY_PPAGE;
        curses["KEY_STAB"] = KEY_STAB;
        curses["KEY_CTAB"] = KEY_CTAB;
        curses["KEY_CATAB"] = KEY_CATAB;
        curses["KEY_ENTER"] = KEY_ENTER;
        curses["KEY_PRINT"] = KEY_PRINT;
        curses["KEY_LL"] = KEY_LL;
        curses["KEY_A1"] = KEY_A1;
        curses["KEY_A3"] = KEY_A3;
        curses["KEY_B2"] = KEY_B2;
        curses["KEY_C1"] = KEY_C1;
        curses["KEY_C3"] = KEY_C3;
        curses["KEY_BTAB"] = KEY_BTAB;
        curses["KEY_BEG"] = KEY_BEG;
        curses["KEY_CANCEL"] = KEY_CANCEL;
        curses["KEY_CLOSE"] = KEY_CLOSE;
        curses["KEY_COMMAND"] = KEY_COMMAND;
        curses["KEY_COPY"] = KEY_COPY;
        curses["KEY_CREATE"] = KEY_CREATE;
        curses["KEY_END"] = KEY_END;
        curses["KEY_EXIT"] = KEY_EXIT;
        curses["KEY_FIND"] = KEY_FIND;
        curses["KEY_HELP"] = KEY_HELP;
        curses["KEY_MARK"] = KEY_MARK;
        curses["KEY_MESSAGE"] = KEY_MESSAGE;
        curses["KEY_MOVE"] = KEY_MOVE;
        curses["KEY_NEXT"] = KEY_NEXT;
        curses["KEY_OPEN"] = KEY_OPEN;
        curses["KEY_OPTIONS"] = KEY_OPTIONS;
        curses["KEY_PREVIOUS"] = KEY_PREVIOUS;
        curses["KEY_REDO"] = KEY_REDO;
        curses["KEY_REFERENCE"] = KEY_REFERENCE;
        curses["KEY_REFRESH"] = KEY_REFRESH;
        curses["KEY_REPLACE"] = KEY_REPLACE;
        curses["KEY_RESTART"] = KEY_RESTART;
        curses["KEY_RESUME"] = KEY_RESUME;
        curses["KEY_SAVE"] = KEY_SAVE;
        curses["KEY_SBEG"] = KEY_SBEG;
        curses["KEY_SCANCEL"] = KEY_SCANCEL;
        curses["KEY_SCOMMAND"] = KEY_SCOMMAND;
        curses["KEY_SCOPY"] = KEY_SCOPY;
        curses["KEY_SCREATE"] = KEY_SCREATE;
        curses["KEY_SDC"] = KEY_SDC;
        curses["KEY_SDL"] = KEY_SDL;
        curses["KEY_SELECT"] = KEY_SELECT;
        curses["KEY_SEND"] = KEY_SEND;
        curses["KEY_SEOL"] = KEY_SEOL;
        curses["KEY_SEXIT"] = KEY_SEXIT;
        curses["KEY_SFIND"] = KEY_SFIND;
        curses["KEY_SHELP"] = KEY_SHELP;
        curses["KEY_SHOME"] = KEY_SHOME;
        curses["KEY_SIC"] = KEY_SIC;
        curses["KEY_SLEFT"] = KEY_SLEFT;
        curses["KEY_SMESSAGE"] = KEY_SMESSAGE;
        curses["KEY_SMOVE"] = KEY_SMOVE;
        curses["KEY_SNEXT"] = KEY_SNEXT;
        curses["KEY_SOPTIONS"] = KEY_SOPTIONS;
        curses["KEY_SPREVIOUS"] = KEY_SPREVIOUS;
        curses["KEY_SPRINT"] = KEY_SPRINT;
        curses["KEY_SREDO"] = KEY_SREDO;
        curses["KEY_SREPLACE"] = KEY_SREPLACE;
        curses["KEY_SRIGHT"] = KEY_SRIGHT;
        curses["KEY_SRSUME"] = KEY_SRSUME;
        curses["KEY_SSAVE"] = KEY_SSAVE;
        curses["KEY_SSUSPEND"] = KEY_SSUSPEND;
        curses["KEY_SUNDO"] = KEY_SUNDO;
        curses["KEY_SUSPEND"] = KEY_SUSPEND;
        curses["KEY_UNDO"] = KEY_UNDO;
        curses["KEY_MOUSE"] = KEY_MOUSE;
        curses["KEY_RESIZE"] = KEY_RESIZE;
        curses["KEY_MAX"] = KEY_MAX;

        crogue["curses"] = curses;

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
