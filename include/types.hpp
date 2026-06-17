#pragma once
#include <functional>
#include <memory>
#include <string>

const std::string logfile = "./build/debug.log";

enum card_type { BASIC, ITEM, ENEMY, EXIT };

struct card_t {
        std::string name;
        card_type type;
        std::function<int()> event;
};

struct card_slot_t {
        std::shared_ptr<card_t> back;
        std::shared_ptr<card_t> front;
};

namespace game {

inline std::vector<std::pair<int, std::shared_ptr<card_t>>> deck;  // count and card pairs

inline std::vector<std::shared_ptr<card_t>> card_set;

inline card_slot_t slot1;
inline card_slot_t slot2;
inline card_slot_t slot3;

namespace player {

inline int hp = 100;

inline std::vector<std::shared_ptr<card_t>> inventory;

}  // namespace player
}  // namespace game
