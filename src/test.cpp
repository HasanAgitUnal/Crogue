#include "test.hpp"
#include "types.hpp"

void add_card(const std::string &name, std::function<int()> event) {
        game::card_set.push_back(std::make_shared<card_t>(card_t{name, std::move(event)}));
}

void setup_test() {
        add_card("Zombie", []() { return -1; });
        add_card("Zombie", []() { return -1; });
        add_card("Zombie", []() { return -1; });
        add_card("Zombie", []() { return -1; });
        add_card("Zombie", []() { return -1; });
        add_card("Zombie", []() { return -1; });
        add_card("Zombie", []() { return -1; });
        add_card("spider", []() { return -2; });
        add_card("spider", []() { return -2; });
        add_card("spider", []() { return -2; });
        add_card("spider", []() { return -2; });
        add_card("healing", []() { return +5; });
        add_card("healing", []() { return +5; });
        add_card("healing", []() { return +5; });
        add_card("healing", []() { return +5; });
        add_card("god", []() {
                game::player::hp = 0;
                return 0;
        });
}
