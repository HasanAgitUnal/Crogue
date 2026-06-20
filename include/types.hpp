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

namespace game {

inline std::vector<std::shared_ptr<biome_t>> biomes;  // unordered levels
inline std::vector<std::shared_ptr<level_t>> levels;  // ordered levels
inline int levelid;

inline std::vector<std::pair<int, std::shared_ptr<card_t>>> deck;  // count and card pairs
inline std::vector<std::shared_ptr<card_t>> card_set;

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
