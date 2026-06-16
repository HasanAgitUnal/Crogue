#pragma once
#include <functional>
#include <memory>
#include <string>

struct card_t {
        std::string name;
        std::function<int()> event;
};

struct card_slot_t {
        std::shared_ptr<card_t> back;
        std::shared_ptr<card_t> front;
};

namespace game {

inline std::vector<std::shared_ptr<card_t>> card_set;

inline card_slot_t slot1;
inline card_slot_t slot2;
inline card_slot_t slot3;

namespace player {

inline int hp = 100;

}  // namespace player
}  // namespace game
