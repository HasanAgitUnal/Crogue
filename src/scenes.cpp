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
#include <sstream>
#include <string>
#include <vector>

#include "cards.hpp"
#include "minilog.hpp"
#include "scenes.hpp"
#include "test.hpp"
#include "tui.hpp"

namespace scene {

void handle_seed_input(int y, int max_x) {
        curs_set(1);
        std::string seed_str = std::to_string(game::seed);
        int box_size = 20;
        int label_len = 5;
        int start_x = (max_x - (label_len + 1 + box_size)) / 2 + label_len + 1;

        auto redraw_input = [&]() {
                mvprintw(y, start_x, "%-*s", box_size, seed_str.c_str());
                mvchgat(y, start_x, box_size, A_NORMAL, 501, NULL);
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

void main_menu() {
        if (game::_skip_main_menu) {
                // play game for once and exit sliently
                game();
                return;
        }

        std::vector<std::string> menu = {"Play", "Random Seed", "Seed:"};
        int choice = 0;
        int key = 0;

        while (key != 'q') {
                init_pair(500, -1, 236);
                init_pair(501, -1, 234);
                print_menu(menu, choice);
                key = getch();

                switch (key) {
                        case 'k':
                        case KEY_UP:
                                if (choice > 0)
                                        choice--;
                                break;
                        case 'j':
                        case KEY_DOWN:
                                if (choice < (int)menu.size() - 1)
                                        choice++;
                                break;
                        case 10:
                        case 'l':
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

        create_card(1, "~ Exit Gate ~", EXIT, {}, "", 0, exit_gate);

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
                bool turn_taken = false;
                card_slot_t *acted_slot = nullptr;

                switch (key) {
                        case 'a':
                                minilog::fdebugc("game", logfile, "Picked card slot1");
                                handle_slot(game::slot1);
                                acted_slot = &game::slot1;
                                turn_taken = true;
                                break;
                        case 'b':
                                minilog::fdebugc("game", logfile, "Picked card slot2");
                                handle_slot(game::slot2);
                                acted_slot = &game::slot2;
                                turn_taken = true;
                                break;
                        case 'c':
                                minilog::fdebugc("game", logfile, "Picked card slot3");
                                handle_slot(game::slot3);
                                acted_slot = &game::slot3;
                                turn_taken = true;
                                break;
                        case '0' ... '9': {
                                int index = key - '0';
                                minilog::fdebugc("inventory", logfile, "User tried to use item index: ", index);
                                if (index < (int)game::player::inventory.size()) {
                                        if (game::player::inventory[index]) {
                                                minilog::fdebugc("event", logfile, "Calling card event for item: ",
                                                                 game::player::inventory[index]->name);

                                                basic_card_event(game::player::inventory[index], 0);
                                                game::player::inventory[index] = nullptr;
                                                turn_taken = true;
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

                // time-to-live
                if (turn_taken) {
                        for (card_slot_t *slot : {&game::slot1, &game::slot2, &game::slot3}) {
                                if (!slot->front || slot == acted_slot) {
                                        continue;
                                }

                                if (slot->front->ttl == 0)
                                        continue;

                                slot->_lived++;

                                if (slot->front->ttl == slot->_lived) {
                                        minilog::fdebugc("game", logfile, "time-to-live expired for a card");
                                        handle_slot(*slot);
                                        slot->_lived = 0;
                                }
                        }
                }
        }
}

}  // namespace scene
