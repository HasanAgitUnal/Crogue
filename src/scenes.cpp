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
#include "lua.hpp"
#include "minilog.hpp"
#include "scenes.hpp"
#include "settings.hpp"
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
        setup_lua();
        load_plugins();
        plugin_errors();
        game::hooks::trigger(game::hooks::start);

        if (game::_skip_main_menu) {
                // play game for once and exit sliently
                minilog::fdebugc("cli", logfile, "Skipping main menu");
                game();
                return;
        }

        bool seed_changed = true;

        std::vector<std::string> main = {"Play", "Plugin Manager", "Reload All Plugins", "Quit"};
        std::vector<std::string> play = {"~~~ Start ~~~", "Seed:", "Random Seed", "Back"};

        std::vector<std::string> menu = main;
        int choice = 0;
        int key = 0;

        while (key != 'q') {
                init_pair(500, -1, 236);
                init_pair(501, -1, 234);
                print_menu(menu, choice);

                // on play menu show a warning if user does not changes seed.
                if (menu[0] != "Play" && !seed_changed) {
                        int max_y, max_x;
                        getmaxyx(stdscr, max_y, max_x);
                        attron(COLOR_PAIR(4));
                        mvprintw(
                            max_y - 1, 0,
                            "You are playing with same seed! May you want to change it before starting a new game.");
                        attroff(COLOR_PAIR(4));
                        refresh();
                }

                key = getch();

                switch (key) {
                        case 'k':
                        case KEY_UP:
                                if (choice > 0)
                                        choice--;
                                else
                                        choice = menu.size() - 1;
                                break;
                        case 'j':
                        case KEY_DOWN:
                                if (choice < (int)menu.size() - 1)
                                        choice++;
                                else
                                        choice = 0;
                                break;
                        case 10:
                        case 'l':
                        case KEY_ENTER:
                                if (menu[choice] == "~~~ Start ~~~") {
                                        minilog::fdebugc("setup", logfile, "Starting game.");
                                        game();
                                        game::hooks::trigger(game::hooks::game_end);

                                        menu = main;
                                        choice = 0;
                                        seed_changed = false;

                                } else if (menu[choice] == "Play") {
                                        menu = play;
                                        choice = 0;

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

                                        uint64_t last_seed = game::seed;
                                        handle_seed_input(menu_start_y + (choice * 2), max_x);
                                        if (game::seed != last_seed) {
                                                seed_changed = true;
                                        }

                                } else if (menu[choice] == "Random Seed") {
                                        uint64_t last_seed = game::seed;
                                        std::random_device rd;
                                        std::mt19937_64 gen(rd());
                                        std::uniform_int_distribution<uint64_t> dis;
                                        game::seed = dis(gen);

                                        if (game::seed != last_seed) {
                                                seed_changed = true;
                                        }

                                } else if (menu[choice] == "Back") {
                                        menu = main;
                                        choice = 0;

                                } else if (menu[choice] == "Reload All Plugins") {
                                        cleanup_lua();
                                        reset_game(true);
                                        setup_lua();
                                        load_plugins();
                                        plugin_errors();
                                        game::hooks::trigger(game::hooks::reload);

                                        int max_y, max_x;
                                        getmaxyx(stdscr, max_y, max_x);

                                        mvprintw(max_y - 2, 0, "Successfuly reloaded plugins!");
                                        press_enter_to_continue();

                                } else if (menu[choice] == "Plugin Manager") {
                                        settings();

                                } else if (menu[choice] == "Quit") {
                                        return;
                                }

                                break;
                }
        }
}

bool on_level_complete(int curr_level) {
        game::hooks::trigger(game::hooks::level_up, game::player::level);

        if (game::player::level == (int)game::levels.size()) {
                clear();
                attron(COLOR_PAIR(4));
                mvprintw(0, 0, "You found Amulet of Yendor!!!");
                mvprintw(1, 0, "And exiting from dungeon with your loot!");
                attroff(COLOR_PAIR(4));

                game::hooks::trigger(game::hooks::ending);

                refresh();
                getch();

                minilog::fdebugc("setup", logfile, "player reached last level");
                return true;
        }

        clear();

        attron(COLOR_PAIR(3));

        mvprintw(0, 0, "Congratulations, you completed level %d (%s). Next level is %s", curr_level,
                 game::levels[curr_level - 1]->name.c_str(), game::levels[curr_level]->name.c_str());

        attroff(COLOR_PAIR(3));

        press_enter_to_continue();

        // clear logs
        game::logs.clear();

        game::levelid = game::levels[game::player::level]->id;
        minilog::fdebugc("setup", logfile, "new level id: ", game::levelid);
        minilog::fdebugc("setup", logfile, "new level name: ", game::levels[game::player::level]->name);

        minilog::fdebugc("setup", logfile, "resetting the cards");
        game::card_set = {};

        draw_cards();
        minilog::fdebugc("setup", logfile, "card_set size: ", (int)game::card_set.size());

        draw_slots();

        game::slot1._lived = 0;
        game::slot2._lived = 0;
        game::slot3._lived = 0;

        log("You are now at level: " + game::levels[game::player::level]->name, WARN);

        return false;
}

void game() {
        if (game::settings::metadata.empty()) {
                // return if no plugin loaded
                int max_y, max_x;
                getmaxyx(stdscr, max_y, max_x);
                mvprintw(max_y - 1, 0, "No plugin loaded!");
                getch();
                return;
        }

        reset_game(false);

        minilog::fdebugc("setup", logfile, "Generating levels");
        generate_levels();

        if (game::levels.empty()) {
                int max_y, max_x;
                getmaxyx(stdscr, max_y, max_x);
                mvprintw(max_y - 1, 0, "No level created!");
                getch();
                return;
        }

        game::levelid = game::levels[0]->id;
        log("You are now at level: " + game::levels[game::player::level]->name, WARN);

        minilog::fdebugc("setup", logfile, "deck size: ", game::deck.size());
        draw_cards();

        minilog::fdebugc("setup", logfile, "card_set size: ", game::card_set.size());
        draw_slots();

        game::hooks::trigger(game::hooks::game_start);

        int key = 0;
        int last_level = -1;
        while (true) {
                handle_buffs();

                if (check_die()) {
                        break;
                }

                game::hooks::trigger(game::hooks::before_refresh);

                clear();
                print_ui();
                refresh();

                game::hooks::trigger(game::hooks::after_refresh);

                // keyboard handling
                key = getch();
                game::hooks::trigger(game::hooks::key, key);

                minilog::fdebugc("key", logfile, "pressed key: ", key);
                bool turn_taken = false;
                card_slot_t *acted_slot = nullptr;

                switch (key) {
                        case 'a': {
                                minilog::fdebugc("game", logfile, "Picked card slot1");
                                bool cancel = game::hooks::trigger_bool(game::hooks::slot, 1);
                                if (cancel) {
                                        break;
                                }

                                handle_slot(game::slot1);
                                acted_slot = &game::slot1;
                                turn_taken = true;
                                break;
                        }
                        case 'b': {
                                minilog::fdebugc("game", logfile, "Picked card slot2");
                                bool cancel = game::hooks::trigger_bool(game::hooks::slot, 2);
                                if (cancel) {
                                        break;
                                }

                                handle_slot(game::slot2);
                                acted_slot = &game::slot2;
                                turn_taken = true;
                                break;
                        }
                        case 'c': {
                                minilog::fdebugc("game", logfile, "Picked card slot3");
                                bool cancel = game::hooks::trigger_bool(game::hooks::slot, 3);
                                if (cancel) {
                                        break;
                                }

                                handle_slot(game::slot3);
                                acted_slot = &game::slot3;
                                turn_taken = true;
                                break;
                        }
                        case '0' ... '9': {
                                int index = key - '0';
                                minilog::fdebugc("inventory", logfile, "User tried to use item index: ", index);
                                if (index < (int)game::player::inventory.size()) {
                                        if (game::player::inventory[index]) {
                                                minilog::fdebugc("event", logfile, "Calling card event for item: ",
                                                                 game::player::inventory[index]->name);

                                                bool canceled = game::hooks::trigger_bool(
                                                    game::hooks::item, game::player::inventory[index]);
                                                if (canceled) {
                                                        continue;
                                                }

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
                                                game::hooks::trigger(game::hooks::game_quit);
                                                return;
                                        } else if (key == 'n') {
                                                break;
                                        }
                                }
                }

                // check if level changed
                if (last_level != game::player::level && last_level != -1) {
                        // return if amulet of yendor found
                        if (on_level_complete(game::player::level)) {
                                return;
                        }

                        last_level = game::player::level;
                        continue;
                }

                last_level = game::player::level;

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
                                        log(slot->front->name + " saw you!", log_type::WARN);
                                        handle_slot(*slot);
                                        slot->_lived = 0;
                                }
                        }
                }

                // check again
                if (last_level != game::player::level && last_level != -1) {
                        if (on_level_complete(game::player::level)) {
                                return;
                        }

                        last_level = game::player::level;
                        continue;
                }

                last_level = game::player::level;
        }
}

}  // namespace scene
