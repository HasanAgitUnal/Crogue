#pragma once

#include <functional>
#include <string>

#include "types.hpp"

// for now it just exits
int exit_gate();

void create_card(const int count, const std::string &name, const card_type &type, const std::vector<int> levelids,
                 std::function<int()> event);

uint64_t create_level(const int difficulty, const std::string name);

void generate_levels();

void draw_cards();

void handle_slot(card_slot_t &slot);

void draw_slots();

void basic_card_event(const std::shared_ptr<card_t> card);
