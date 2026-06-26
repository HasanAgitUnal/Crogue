#include <sol/sol.hpp>

#include "cards.hpp"
#include "minilog.hpp"
#include "tui.hpp"
#include "types.hpp"

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

void setup_lua() {
        game::lua.open_libraries(sol::lib::base, sol::lib::table);

        // clang-format off

        /*
         * Enums
         */

        game::lua.new_enum("CardType",
                        "BASIC", card_type::BASIC,
                        "ITEM", card_type::ITEM,
                        "ENEMY", card_type::ENEMY,
                        "EXIT", card_type::EXIT);

        game::lua.new_enum("LogType",
                        "NORMAL", log_type::NORMAL,
                        "WARN", log_type::WARN,
                        "IMPORTANT", log_type::IMPORTANT);

        /*
         * Types
         */

        game::lua.new_usertype<card_t>("Card", sol::constructors<card_t>(),
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

        game::lua.new_usertype<card_slot_t>("CardSlot", sol::constructors<card_slot_t>(),
                        "back", &card_slot_t::back,
                        "front", &card_slot_t::front,
                        "_lived", &card_slot_t::_lived);

        game::lua.new_usertype<level_t>("Level", sol::constructors<level_t>(),
                        "name", &level_t::name,
                        "id", &level_t::id);

        game::lua.new_usertype<biome_t>("Biome", sol::constructors<biome_t>(),
                        "name", &biome_t::name,
                        "difficulty", &biome_t::difficulty,
                        "levels", &biome_t::levels);

        game::lua.new_usertype<buff_t>("Buff", sol::constructors<buff_t>(),
                        "name", &buff_t::name,
                        "event", sol::property(
                                [](buff_t& b) -> std::function<void(std::shared_ptr<buff_t>)>& { return b.event; },
                                [](buff_t& b, std::function<void(std::shared_ptr<buff_t>)> f) { b.event = f; }
                                ),
                        "level", &buff_t::level);

        // clang-format on

        /*
         * Functions
         */

        game::lua.set_function<char(std::string)>("ask", ask);
        game::lua.set_function<std::string(std::string)>("ask_string", ask_string);

        game::lua.set_function<void(std::string, log_type)>("log", log);

        game::lua.set_function<void(sol::table)>("create_card", create_card);
        game::lua.set_function<std::shared_ptr<level_t>(std::string)>("create_level", create_level);
        game::lua.set_function<std::shared_ptr<buff_t>(sol::table)>("create_buff", create_buff);
        game::lua.set_function<void(sol::table)>("create_biome", create_biome);

        /*
         * Variables
         */

        sol::table crogue = game::lua.create_table();

        sol::table stat = game::lua.create_table();

        stat["biomes"] = std::ref(game::biomes);
        stat["levels"] = std::ref(game::levels);
        stat["levelid"] = std::ref(game::levelid);
        stat["deck"] = std::ref(game::deck);
        stat["card_set"] = std::ref(game::card_set);
        stat["buffs"] = std::ref(game::buffs);
        stat["slot1"] = std::ref(game::slot1);
        stat["slot2"] = std::ref(game::slot2);
        stat["slot3"] = std::ref(game::slot3);
        stat["logs"] = std::ref(game::logs);
        stat["seed"] = std::ref(game::seed);

        sol::table player = game::lua.create_table();

        player["get_hp"] = &get_player_hp;
        player["set_hp"] = &set_player_hp;

        player["get_level"] = &get_player_level;
        player["set_level"] = &set_player_level;

        crogue["stat"] = stat;
        crogue["player"] = player;

        game::lua["crogue"] = crogue;
}

void load_plugins() {
        // TODO: add a plugins directory and load plugins from here
        // TODO: remove this after finishing plugins

        sol::protected_function_result result = game::lua.safe_script_file("test.lua", &sol::script_pass_on_error);
        if (!result.valid()) {
                sol::error err = result;
                log("Lua script error: " + std::string(err.what()), log_type::WARN);
                minilog::fdebugc("lua", logfile, minilog::msg::error, "Error in lua: ", err.what());
        }
}
