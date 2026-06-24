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

#include <ncurses.h>
#include <string>
#include <vector>

void setup_colors();

attr_t parse_ansi_color(const std::string &params);
void print_ansi(const std::string &str);
int get_real_size(const std::string &line);

/*
 * Main Menu
 */

void print_seed_item(int y, int max_x, bool selected);
void print_menu(const std::vector<std::string> &menu, int choice);

/*
 * Game
 */

int ask(std::string what);
std::string ask_string(std::string what);
int print_slots(int line);
void print_stats(int line);
void print_buffs(int line);
int print_logs(int line);
int print_inventory(int line);
void print_ui();
