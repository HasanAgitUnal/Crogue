// CROGUE - Card-based rogulike TUI game
// Copyright (C) 2026  Hasan Agit Ünal
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <functional>
#include <sol/sol.hpp>
#include <string>

#include "types.hpp"

void reset_game(bool full);

void log(const std::string msg, const log_type type);

bool check_die();

int exit_gate();

void create_card(const int count, const std::string &name, const card_type &type, const std::vector<int> levelids,
                 const std::string &logmsg, int ttl, std::function<int()> event);

void create_card(sol::table table);

std::shared_ptr<level_t> create_level(const std::string name);

std::shared_ptr<buff_t> create_buff(const std::string name, std::function<void(std::shared_ptr<buff_t>)> event);
std::shared_ptr<buff_t> create_buff(sol::table table);

void create_biome(const std::string name, const int difficulty, const std::vector<std::shared_ptr<level_t>> &levels);
void create_biome(sol::table table);

void generate_levels();

void draw_cards();
void handle_slot(card_slot_t &slot);
void draw_slots();

void basic_card_event(const std::shared_ptr<card_t> card, const int extra);
void card_event(const std::shared_ptr<card_t> card, const int extra);
void handle_buffs();
