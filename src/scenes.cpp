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

#include <ncurses.h>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "cards.hpp"
#include "minilog.hpp"
#include "scenes.hpp"
#include "test.hpp"
#include "tui.hpp"

/*
 * Welcome to AI made shits file
 * This file is %70 AI generated && spagetti
 */

// Colored CROGUE word
std::string banner = R"(
[38;5;7m   [38;5;236m▒[38;5;242m██[38;5;239m█[38;5;22m█[38;5;236m▒[38;5;7m  [38;5;240m██[38;5;22m██[38;5;244m██[38;5;236m▒[38;5;7m    [38;5;2m░[38;5;22m█[38;5;28m█[38;5;242m██[38;5;240m░[38;5;7m     [38;5;236m▒[38;5;244m██[38;5;22m██[38;5;54m▒[38;5;7m  [38;5;22m██[38;5;7m    [38;5;55m█[38;5;54m█[38;5;7m  [38;5;55m█[38;5;56m█[38;5;93m███[38;5;56m█[38;5;55m██
[38;5;7m  [38;5;234m▓[38;5;240m██[38;5;28m██[38;5;239m█[38;5;28m█[38;5;2m  [38;5;236m█[38;5;238m██[38;5;236m████[38;5;22m▓[38;5;7m   [38;5;242m██[38;5;22m█[38;5;242m███[38;5;7m    [38;5;238m▓[38;5;240m██[38;5;235m█[38;5;238m█[38;5;242m█[38;5;55m█[38;5;7m  [38;5;238m█[38;5;240m█[38;5;7m    [38;5;93m█[38;5;55m█[38;5;7m  [38;5;56m█[38;5;93m██[38;5;55m█[38;5;93m██[38;5;56m█[38;5;93m█
[38;5;7m [38;5;236m▒[38;5;244m██[38;5;236m▒[38;5;7m  [38;5;236m░█[38;5;7m  [38;5;240m█[38;5;238m█[38;5;7m   [38;5;240m▒[38;5;28m██[38;5;7m  [38;5;242m▒█[38;5;238m█[38;5;242m  ██▒[38;5;7m  [38;5;238m▒[38;5;240m██▒[38;5;7m  [38;5;54m░[38;5;244m█[38;5;7m  [38;5;235m█[38;5;240m█[38;5;7m    [38;5;238m█[38;5;93m█[38;5;7m  [38;5;56m█[38;5;55m█[38;5;7m      
 [38;5;234m██[38;5;22m▓[38;5;7m       [38;5;242m█[38;5;236m█[38;5;7m    [38;5;242m██[38;5;7m  [38;5;236m██▒[38;5;7m  [38;5;236m▒█[38;5;28m█[38;5;7m  [38;5;240m██▒[38;5;7m       [38;5;238m█[38;5;93m█[38;5;7m    [38;5;235m█[38;5;55m█[38;5;7m  [38;5;55m█[38;5;238m█[38;5;7m      
 [38;5;238m██[38;5;2m░       [38;5;244m██[38;5;7m   [38;5;244m▒██[38;5;7m  [38;5;240m██[38;5;7m    [38;5;240m█[38;5;22m█[38;5;7m  [38;5;238m██[38;5;54m░[38;5;7m       [38;5;240m█[38;5;55m█[38;5;7m    [38;5;93m█[38;5;235m█[38;5;7m  [38;5;240m█[38;5;93m█[38;5;7m      
 [38;5;236m██ [38;5;7m       [38;5;242m██[38;5;238m█[38;5;236m█[38;5;242m█[38;5;238m█[38;5;242m█[38;5;22m▒[38;5;7m  [38;5;236m██[38;5;7m    [38;5;236m██[38;5;7m  [38;5;240m█[38;5;54m█[38;5;7m        [38;5;240m█[38;5;235m█[38;5;7m    [38;5;93m█[38;5;55m█[38;5;7m  [38;5;93m█[38;5;56m██[38;5;93m██[38;5;56m█[38;5;55m█[38;5;7m 
 [38;5;234m██ [38;5;7m       [38;5;240m█[38;5;242m█[38;5;244m█[38;5;242m██[38;5;240m█[38;5;28m▓[38;5;7m   [38;5;238m██[38;5;7m    [38;5;238m██[38;5;7m  [38;5;236m██[38;5;7m  [38;5;235m█[38;5;236m█[38;5;235m█[38;5;22m█[38;5;7m  [38;5;93m█[38;5;54m█[38;5;7m    [38;5;235m█[38;5;55m█[38;5;7m  [38;5;55m██[38;5;93m█[38;5;55m█[38;5;56m█[38;5;93m█[38;5;55m█[38;5;7m 
 [38;5;238m██[38;5;244m░[38;5;7m       [38;5;236m█[38;5;244m█[38;5;7m  [38;5;236m▓[38;5;234m█[38;5;242m█[38;5;236m░[38;5;7m  [38;5;238m█[38;5;242m█[38;5;7m    [38;5;242m██[38;5;7m  [38;5;238m██[38;5;54m░[38;5;7m [38;5;240m█[38;5;239m█[38;5;93m█[38;5;55m█[38;5;7m  [38;5;238m█[38;5;55m█[38;5;7m    [38;5;235m█[38;5;54m█[38;5;7m  [38;5;93m█[38;5;55m█[38;5;7m      
 [38;5;236m██[38;5;240m▓[38;5;7m       [38;5;238m█[38;5;244m█[38;5;7m   [38;5;242m█[38;5;240m█[38;5;236m▓[38;5;7m  [38;5;244m██▒[38;5;7m  [38;5;244m▒█[38;5;238m█[38;5;7m  [38;5;240m██▒[38;5;7m   [38;5;240m█[38;5;236m█[38;5;7m  [38;5;240m█[38;5;235m█[38;5;7m    [38;5;238m█[38;5;240m█[38;5;7m  [38;5;236m█[38;5;93m█[38;5;7m      
[38;5;234m ▒[38;5;28m██[38;5;240m▒[38;5;7m  [38;5;240m░█[38;5;7m  [38;5;242m██[38;5;7m   [38;5;242m▒[38;5;240m██[38;5;7m  [38;5;236m▒██  ██▒[38;5;7m  [38;5;240m▒█[38;5;238m█[38;5;240m▒[38;5;7m  [38;5;236m█[38;5;235m█[38;5;7m  [38;5;240m██[38;5;22m▓[38;5;7m  [38;5;54m▓[38;5;240m█[38;5;238m█[38;5;7m  [38;5;238m█[38;5;93m█[38;5;7m      
  [38;5;236m▓[38;5;244m██████[38;5;7m  [38;5;240m█[38;5;238m█[38;5;7m    [38;5;234m█[38;5;240m█▒[38;5;7m  [38;5;238m███[38;5;22m█[38;5;238m██[38;5;7m    [38;5;22m█[38;5;240m█[38;5;55m██[38;5;240m██[38;5;235m█[38;5;7m  [38;5;240m▒██[38;5;93m█[38;5;55m██[38;5;240m█[38;5;55m█[38;5;7m  [38;5;240m█[38;5;236m█[38;5;93m█[38;5;56m██[38;5;55m█[38;5;56m█[38;5;93m█
[38;5;7m   [38;5;234m▒[38;5;28m█[38;5;22m███[38;5;242m▒[38;5;7m  [38;5;236m█[38;5;238m█[38;5;7m    [38;5;234m█[38;5;236m██[38;5;7m  [38;5;2m░[38;5;22m█[38;5;28m█[38;5;22m█[38;5;240m█[38;5;22m░[38;5;7m     [38;5;22m▒█[38;5;235m█[38;5;236m██[38;5;54m░[38;5;7m   [38;5;22m▒[38;5;240m█[38;5;235m█[38;5;240m█[38;5;238m█[38;5;54m█[38;5;7m   [38;5;22m██[38;5;238m█[38;5;93m█[38;5;56m█[38;5;55m█[38;5;93m█[38;5;55m█
[0m
)";

namespace scene {

namespace {


attr_t parse_ansi_color(const std::string &params) {
        if (params == "0" || params == "" || params == "00")
                return A_NORMAL;

        std::regex color_pattern("38;5;([0-9]+)");
        std::smatch matches;

        if (std::regex_search(params, matches, color_pattern)) {
                int color_idx = std::stoi(matches[1].str());
                return COLOR_PAIR(color_idx + 1);
        }
        return A_NORMAL;
}

void print_ansi(const std::string &str) {
        bool in_escape = false;
        std::string params = "";
        std::string buffer = "";

        for (size_t i = 0; i < str.size(); ++i) {
                unsigned char ch = (unsigned char)str[i];

                if (ch == '\033') {
                        if (!buffer.empty()) {
                                printw("%s", buffer.c_str());
                                buffer = "";
                        }
                        in_escape = true;
                        if (i + 1 < str.size() && str[i + 1] == '[')
                                i++;
                        continue;
                }

                if (in_escape) {
                        if (ch == 'm') {
                                attrset(parse_ansi_color(params));
                                params = "";
                                in_escape = false;
                        } else {
                                params += (char)ch;
                        }
                } else {
                        buffer += (char)ch;
                }
        }
        if (!buffer.empty())
                printw("%s", buffer.c_str());
        attrset(A_NORMAL);
}

int get_real_size(const std::string &line) {
        bool in_escape = false;
        int size = 0;

        for (size_t i = 0; i < line.size(); ++i) {
                if (line[i] == '\033') {
                        in_escape = true;
                        if (i + 1 < line.size() && line[i + 1] == '[')
                                i++;
                        continue;
                }
                if (in_escape) {
                        if (line[i] == 'm')
                                in_escape = false;
                } else {
                        if ((line[i] & 0xc0) != 0x80)
                                size++;
                }
        }
        return size;
}

void draw_seed_item(int y, int max_x, bool selected) {
        int box_size = 20;
        int label_len = 5;  // "Seed:"
        int total_len = label_len + 1 + box_size;
        int start_x = (max_x - total_len) / 2;

        if (selected) {
                attron(COLOR_PAIR(19));
                mvprintw(y, start_x, "Seed:");
                attroff(COLOR_PAIR(19));
        } else {
                mvprintw(y, start_x, "Seed:");
        }

        mvaddch(y, start_x + label_len, ' ');

        attron(COLOR_PAIR(18));
        mvprintw(y, start_x + label_len + 1, "%-*s", box_size, std::to_string(game::seed).c_str());
        attroff(COLOR_PAIR(18));
}

void draw_menu(const std::vector<std::string> &menu, int choice) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        clear();

        // Banner'ı satırlara böl
        std::vector<std::string> banner_lines;
        std::string line;
        std::stringstream ss(banner);
        while (std::getline(ss, line)) {
                if (!line.empty())
                        banner_lines.push_back(line);
        }

        int banner_h = banner_lines.size();
        int menu_h = menu.size() + (menu.size() - 1);
        int spacing = 2;  // space between banner and menu
        int total_h = banner_h + spacing + menu_h;

        int start_y = (max_y - total_h) / 2;

        // print banner
        for (int i = 0; i < banner_h; ++i) {
                int x = (max_x - get_real_size(banner_lines[i])) / 2;
                move(start_y + i, x);
                print_ansi(banner_lines[i]);
        }

        // print menu
        int menu_start_y = start_y + banner_h + spacing;
        for (int i = 0; i < (int)menu.size(); ++i) {
                int current_y = menu_start_y + (i * 2);
                if (menu[i] == "Seed:") {
                        draw_seed_item(current_y, max_x, i == choice);
                } else {
                        int x = (max_x - (int)menu[i].length()) / 2;
                        if (i == choice) {
                                attron(COLOR_PAIR(19));
                                mvprintw(current_y, x, "%s", menu[i].c_str());
                                attroff(COLOR_PAIR(19));
                        } else {
                                mvprintw(current_y, x, "%s", menu[i].c_str());
                        }
                }
        }
        refresh();
}

void handle_seed_input(int y, int max_x) {
        curs_set(1);
        std::string seed_str = std::to_string(game::seed);
        int box_size = 20;
        int label_len = 5;
        int start_x = (max_x - (label_len + 1 + box_size)) / 2 + label_len + 1;

        auto redraw_input = [&]() {
                attron(COLOR_PAIR(20));
                mvprintw(y, start_x, "%-*s", box_size, seed_str.c_str());
                attroff(COLOR_PAIR(20));
                move(y, start_x + seed_str.length());
                refresh();
        };

        curs_set(2);
        redraw_input();

        int ch;
        while ((ch = getch()) != '\n' && ch != KEY_ENTER && ch != 10) {
                if (ch == 27)
                        break;
                if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                        if (!seed_str.empty())
                                seed_str.pop_back();
                } else if (isdigit(ch) && seed_str.length() < (size_t)box_size) {
                        seed_str += ch;
                }
                redraw_input();
        }

        if (!seed_str.empty()) {
                try {
                        game::seed = std::stoull(seed_str);
                } catch (...) {
                }
        }
        curs_set(0);
}

}  // anonymous namespace

void main_menu() {
        std::vector<std::string> menu = {"Play", "Seed:", "Random Seed"};
        int choice = 0;
        int key = 0;

        while (key != 'q') {
                draw_menu(menu, choice);
                key = getch();

                switch (key) {
                        case KEY_UP:
                                if (choice > 0)
                                        choice--;
                                break;
                        case KEY_DOWN:
                                if (choice < (int)menu.size() - 1)
                                        choice++;
                                break;
                        case 10:
                        case KEY_ENTER:
                                if (menu[choice] == "Play") {
                                        game();
                                } else if (menu[choice] == "Seed:") {
                                        int max_y, max_x;
                                        getmaxyx(stdscr, max_y, max_x);

                                        int banner_h = 0;
                                        std::stringstream ss(banner);
                                        std::string temp;
                                        while (std::getline(ss, temp)) {
                                                if (!temp.empty())
                                                        banner_h++;
                                        }

                                        int menu_h = menu.size() + (menu.size() - 1);
                                        int spacing = 2;
                                        int total_h = banner_h + spacing + menu_h;

                                        int start_y = (max_y - total_h) / 2;
                                        int menu_start_y = start_y + banner_h + spacing;

                                        handle_seed_input(menu_start_y + (choice * 2), max_x);

                                        /*
                                        int max_y, max_x;
                                        getmaxyx(stdscr, max_y, max_x);
                                        int total_height = menu.size() + (menu.size() - 1);
                                        int start_y = (max_y - total_height) / 2;
                                        handle_seed_input(start_y + (choice * 2), max_x);
                                        */

                                } else if (menu[choice] == "Random Seed") {
                                        std::random_device rd;
                                        std::mt19937_64 gen(rd());
                                        std::uniform_int_distribution<uint64_t> dis;
                                        game::seed = dis(gen);
                                }
                                break;
                }
        }
}

void game() {
        reset_game(true);
        //  TODO: remove this after adding plugin system
        setup_test();

        minilog::fdebugc("setup", logfile, "Generating levels");
        generate_levels();
        game::levelid = game::levels[0]->id;
        log("You are now at level: " + game::levels[game::player::level]->name, WARN);

        create_card(1, "~ Exit Gate ~", EXIT, {}, "", exit_gate);

        minilog::fdebugc("setup", logfile, "deck size: ", game::deck.size());
        draw_cards();

        minilog::fdebugc("setup", logfile, "card_set size: ", game::card_set.size());
        draw_slots();

        int key = 0;
        while (true) {
                handle_buffs();

                if (check_die()) {
                        break;
                }

                clear();
                print_ui();
                refresh();

                // keyboard handling
                key = getch();
                minilog::fdebugc("key", logfile, "pressed key: ", key);
                switch (key) {
                        case 'a':
                                minilog::fdebugc("game", logfile, "Picked card slot1");
                                handle_slot(game::slot1);
                                break;
                        case 'b':
                                minilog::fdebugc("game", logfile, "Picked card slot2");
                                handle_slot(game::slot2);
                                break;
                        case 'c':
                                minilog::fdebugc("game", logfile, "Picked card slot3");
                                handle_slot(game::slot3);
                                break;
                        case '0' ... '9': {
                                int index = key - '0';
                                minilog::fdebugc("inventory", logfile, "User tried to use item index: ", index);
                                if (index < (int)game::player::inventory.size()) {
                                        if (game::player::inventory[index]) {
                                                minilog::fdebugc("event", logfile, "Calling card event for item: ",
                                                                 game::player::inventory[index]->name);

                                                basic_card_event(game::player::inventory[index]);
                                                game::player::inventory[index] = nullptr;
                                        }
                                }
                                break;
                        }
                        case 'q':
                                while (true) {
                                        int key = ask("Realy quit? [y/n]: ");

                                        if (key == 'y') {
                                                return;
                                        } else if (key == 'n') {
                                                break;
                                        }
                                }
                }
        }
}

}  // namespace scene
