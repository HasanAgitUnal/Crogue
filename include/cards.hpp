#pragma once

#include <functional>
#include <string>

void create_card(const int count, const std::string &name, std::function<int()> event);

void draw_cards();
