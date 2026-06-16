#pragma once
#include <functional>
#include <memory>
#include <string>

namespace game {
namespace player {

inline int hp = 100;

}  // namespace player
}  // namespace game

struct card_t {
        std::string name;
        std::function<int()> event;
};

struct card_slot_t {
        std::shared_ptr<card_t> back;
        std::shared_ptr<card_t> front;
};
