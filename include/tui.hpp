#pragma once
#include "types.hpp"

void print_type(const std::shared_ptr<card_t> card, bool bold);
void print_slot(const int line, const char c, const card_slot_t slot);
void print_inventory(int start_line);
