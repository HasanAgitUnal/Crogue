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
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <string>

const std::string logfile = "./build/debug.log";

enum card_type { BASIC, ITEM, ENEMY, EXIT };

enum log_type { NORMAL, WARN, IMPORTANT };

struct card_t {
        std::string name;
        card_type type;
        std::vector<int> level_ids;
        std::function<int()> event;
        std::string logmsg;
};

struct card_slot_t {
        std::shared_ptr<card_t> back;
        std::shared_ptr<card_t> front;
};

struct level_t {
        std::string name;
        int id;
};

struct biome_t {
        std::string name;
        int difficulty;
        std::vector<std::shared_ptr<level_t>> levels;
};

struct buff_t {
        std::string name;
        std::function<void(std::shared_ptr<buff_t>)> event;
        int level;
};

namespace game {

inline std::vector<std::shared_ptr<biome_t>> biomes;  // unordered levels
inline std::vector<std::shared_ptr<level_t>> levels;  // ordered levels
inline int levelid;

inline std::vector<std::pair<int, std::shared_ptr<card_t>>> deck;  // count and card pairs
inline std::vector<std::shared_ptr<card_t>> card_set;

inline std::vector<std::shared_ptr<buff_t>> buffs;

inline card_slot_t slot1;
inline card_slot_t slot2;
inline card_slot_t slot3;

inline std::deque<std::pair<log_type, std::string>> logs;

// seed used for random things
inline uint64_t seed;

namespace player {

inline int hp = 100;
inline int level = 0;

inline std::vector<std::shared_ptr<card_t>> inventory;

}  // namespace player
}  // namespace game

namespace settings {

inline bool custom_seed = false;

}
