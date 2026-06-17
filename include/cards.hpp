#pragma once

#include <functional>
#include <string>

#include "types.hpp"

void create_card(const int count, const std::string &name, const card_type &type, std::function<int()> event);

void draw_cards();

void handle_slot(card_slot_t &slot);

void draw_slots();

void basic_card_event(const std::shared_ptr<card_t> card);
