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

#include "cards.hpp"
#include "minilog.hpp"
#include "scenes.hpp"
#include "test.hpp"
#include "tui.hpp"

namespace scene {

void main_menu() {
        std::vector<std::string> menu = {"Play", "Seed:"};
        int menu_choice = 0;

        int key = 0;
        while (key != 'q') {
                int max_y, max_x;
                getmaxyx(stdscr, max_y, max_x);

                clear();

                int total_menu_height = menu.size() + (menu.size() - 1);
                int start_y = (max_y - total_menu_height) / 2;

                for (int i = 0; i < (int)menu.size(); ++i) {
                        int current_y = start_y + (i * 2);

                        if (menu[i] == "Seed:") {
                                int box_size = 20;
                                int label_len = std::string("Seed:").length();
                                int total_len = label_len + 1 + box_size;  // +1 for space
                                int start_x = (max_x - total_len) / 2;

                                if (i == menu_choice) {
                                        attron(COLOR_PAIR(18));
                                        mvprintw(current_y, start_x, "Seed:");
                                        attroff(COLOR_PAIR(18));
                                } else {
                                        mvprintw(current_y, start_x, "Seed:");
                                }

                                // Normal space (no highlight)
                                mvaddch(current_y, start_x + label_len, ' ');

                                attron(COLOR_PAIR(17));
                                mvprintw(current_y, start_x + label_len + 1, "%-*s", box_size,
                                         std::to_string(game::seed).c_str());
                                attroff(COLOR_PAIR(17));
                        } else {
                                int x = (max_x - menu[i].length()) / 2;
                                if (i == menu_choice) {
                                        attron(COLOR_PAIR(18));
                                        mvprintw(current_y, x, "%s", menu[i].c_str());
                                        attroff(COLOR_PAIR(18));
                                } else {
                                        mvprintw(current_y, x, "%s", menu[i].c_str());
                                }
                        }
                }

                refresh();

                key = getch();
                switch (key) {
                        case KEY_UP:
                                if (menu_choice != 0)
                                        menu_choice--;
                                break;

                        case KEY_DOWN:
                                if (menu_choice != (int)menu.size() - 1)
                                        menu_choice++;
                                break;
                        case 10:  // ENTER
                        case KEY_ENTER: {
                                if (menu[menu_choice] == "Play") {
                                        game();
                                } else if (menu[menu_choice] == "Seed:") {
                                        curs_set(1);
                                        std::string seed_str = std::to_string(game::seed);
                                        int box_size = 20;
                                        int label_len = std::string("Seed:").length();
                                        int start_x = (max_x - (label_len + 1 + box_size)) / 2 + label_len + 1;
                                        int current_y = start_y + (menu_choice * 2);

                                        int ch;
                                        // Initial draw and cursor position
                                        attron(COLOR_PAIR(19));
                                        mvprintw(current_y, start_x, "%-*s", box_size, seed_str.c_str());
                                        attroff(COLOR_PAIR(19));
                                        move(current_y, start_x + seed_str.length());
                                        curs_set(2);  // High visibility / Block cursor
                                        refresh();

                                        while ((ch = getch()) != '\n' && ch != KEY_ENTER && ch != 10) {
                                                if (ch == 27)
                                                        break;
                                                if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                                                        if (!seed_str.empty())
                                                                seed_str.pop_back();
                                                } else if (isdigit(ch) && seed_str.length() < (size_t)box_size) {
                                                        seed_str += ch;
                                                }

                                                attron(COLOR_PAIR(19));  // Active Input Box color
                                                mvprintw(current_y, start_x, "%-*s", box_size, seed_str.c_str());
                                                attroff(COLOR_PAIR(19));
                                                move(current_y, start_x + seed_str.length());
                                                refresh();
                                        }

                                        if (!seed_str.empty()) {
                                                try {
                                                        game::seed = std::stoull(seed_str);
                                                } catch (...) {
                                                }
                                        }
                                        curs_set(0);
                                }
                                break;
                        }
                }
        }
}

void game() {
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
