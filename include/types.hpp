#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

const std::string logfile = "./build/debug.log";

enum card_type { BASIC, ITEM, ENEMY, EXIT };

struct card_t {
        std::string name;
        card_type type;
        std::vector<int> level_ids;
        std::function<int()> event;
};

struct card_slot_t {
        std::shared_ptr<card_t> back;
        std::shared_ptr<card_t> front;
};

struct level_t {
        std::string name;
        int difficulty;
        int id;
};

namespace game {

inline std::vector<std::shared_ptr<level_t>> level_registry;  // unordered levels
inline std::vector<std::shared_ptr<level_t>> levels;          // ordered levels
inline int levelid;

inline std::vector<std::pair<int, std::shared_ptr<card_t>>> deck;  // count and card pairs
inline std::vector<std::shared_ptr<card_t>> card_set;

inline card_slot_t slot1;
inline card_slot_t slot2;
inline card_slot_t slot3;

// the message will be displayed for one time
inline std::string message;

// seed used for random things
inline uint64_t seed;

namespace player {

inline int hp = 100;
inline int level = 0;

inline std::vector<std::shared_ptr<card_t>> inventory;

}  // namespace player
}  // namespace game
