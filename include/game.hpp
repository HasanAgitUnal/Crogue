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

#pragma once
#define SOL_ALL_SAFETIES_ON 1
#include <ncurses.h>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <sol/sol.hpp>
#include <string>

using json = nlohmann::json;

const std::string logfile = "./build/debug.log";

enum card_type { BASIC, ITEM, ENEMY, EXIT };

enum log_type { NORMAL, WARN, IMPORTANT };

struct card_t {
        int count = 0;
        std::string name = "";
        card_type type = BASIC;
        std::vector<int> level_ids;
        std::string logmsg = "";
        int ttl = 0;    // time-to-live
        int power = 0;  // power of the card (used for extra damage)
        std::function<int()> event = nullptr;
};

struct card_slot_t {
        std::shared_ptr<card_t> back = nullptr;
        std::shared_ptr<card_t> front = nullptr;
        int _lived = 0;
};

struct level_t {
        std::string name = "";
        int id = 0;
};

struct biome_t {
        int difficulty = 0;
        std::vector<std::shared_ptr<level_t>> levels;
};

struct buff_t {
        std::string name = "";
        std::function<void(std::shared_ptr<buff_t>)> event = nullptr;
        int level = 0;
};

namespace game {

inline bool _skip_main_menu = false;
inline bool _plugins_changed = false;  // used to store is current save's plugins are changed (save is broken)
inline std::string _curr_save_loaded = "";

inline std::vector<std::shared_ptr<biome_t>> biomes;  // unordered levels
inline std::vector<std::shared_ptr<level_t>> levels;  // ordered levels
inline int levelid;

inline std::vector<std::shared_ptr<card_t>> deck;
inline std::vector<std::shared_ptr<card_t>> card_set;

inline std::vector<std::shared_ptr<buff_t>> buffs;

inline card_slot_t slot1;
inline card_slot_t slot2;
inline card_slot_t slot3;

inline std::deque<std::pair<log_type, std::string>> logs;

inline uint64_t seed;

inline sol::state lua;
inline std::map<std::string, std::string> plugin_errors;

inline int current_scene_id;

namespace player {

inline int hp = 100;
inline int level = 0;

inline std::vector<std::shared_ptr<card_t>> inventory;

}  // namespace player

namespace hooks {

inline std::vector<std::function<void(void)>> after_refresh, before_refresh, start, game_start, game_end, game_quit,
    reload, die, draw, level_gen, ending;

// arg: key
inline std::vector<std::function<void(int)>> key;

// arg: level
inline std::vector<std::function<void(int)>> level_up;

// arg: slot num (1, 2, 3)
// return is_canceled
inline std::vector<std::function<bool(int)>> slot;

// arg: item card
// return: is_canceled
inline std::vector<std::function<bool(std::shared_ptr<card_t>)>> item;

// arg: card, extra
// return: is_canceled?
inline std::vector<std::function<bool(std::shared_ptr<card_t>, int)>> card_event;

template <typename... Args>
inline void trigger(std::vector<std::function<void(Args...)>> &hooks, Args... args) {
        for (auto &hook : hooks) {
                hook(args...);
        }
}

template <typename... Args>
inline bool trigger_bool(std::vector<std::function<bool(Args...)>> &hooks, Args... args) {
        bool canceled = false;
        for (auto &hook : hooks) {
                if (hook(args...)) {
                        canceled = true;
                }
        }
        return canceled;
}

}  // namespace hooks

namespace settings {

inline json settings = json::object();
inline json metadata = json::object();

}  // namespace settings

}  // namespace game

struct scene_t {
        std::function<void(void)> ui_refresh;
        std::function<bool(int)> key_handler;
        int exit_key = 'q';

        void run() {
                int key = 0;
                while (key != this->exit_key) {
                        this->ui_refresh();

                        key = getch();

                        // if scene ends key_handler returns true
                        if (this->key_handler(key))
                                break;
                }
        };
};
