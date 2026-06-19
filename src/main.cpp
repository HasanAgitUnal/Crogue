#include <ncurses.h>
#include <clocale>
#include <cstdlib>
#include <random>
#include <string>


#include "cards.hpp"
#include "minilog.hpp"
#include "types.hpp"

void setup_test() {
        // levels
        auto ef = create_level("Enterance I");
        auto es = create_level("Enterance II");

        create_biome("Enterance", 0,
                     {
                         ef,
                         es,
                     });

        auto mf = create_level("Mines I");
        auto ms = create_level("Mines II");

        create_biome("Mines", 10,
                     {
                         mf,
                         ms,
                     });

        // cards
        create_card(5, "Zombie", ENEMY, {}, []() { return -1; });
        create_card(3, "spider", ENEMY, {mf->id, ms->id}, []() { return -2; });
        create_card(4, "healing", BASIC, {}, []() { return 5; });
        /*
        create_card(1, "god", ENEMY, []() {
                game::player::hp = 0;
                return 0;
        });
        */
        create_card(5, "apple", ITEM, {}, []() { return 1; });
        create_card(1, "teleporter", ITEM, {ef->id, mf->id}, []() {
                // Skip 1 card from all slots
                for (card_slot_t *slot : {&game::slot1, &game::slot2, &game::slot3}) {
                        if (!slot->front || slot->front->name == "~ Exit Gate ~") {
                                continue;
                        }

                        // Update card slot
                        slot->front = slot->back;

                        if (game::card_set.empty()) {
                                slot->back = nullptr;
                        } else {
                                slot->back = game::card_set.back();
                                game::card_set.pop_back();
                                minilog::fdebug(logfile, "new card_set size: ", game::card_set.size());
                        }
                }

                return 0;
        });
}

void print_type(const std::shared_ptr<card_t> card, bool bold) {
        if (card == nullptr) {
                printw(" ");
                return;
        }

        char c;
        attr_t attr = bold ? A_BOLD : A_DIM;
        switch (card->type) {
                case BASIC:
                case ITEM:
                        c = '+';
                        attr |= COLOR_PAIR(2);
                        break;
                case ENEMY:
                        c = '-';
                        attr |= COLOR_PAIR(1);
                        break;
                case EXIT:
                        c = '#';
                        attr |= COLOR_PAIR(4);
                        break;
        }

        attron(attr);
        printw("%c ", c);
        attroff(attr);
}

void print_slot(const int line, const char c, const card_slot_t slot) {
        move(line, 0);
        printw("[");
        attron(COLOR_PAIR(5));
        printw("%c", c);
        attroff(COLOR_PAIR(5));
        printw("] ");

        print_type(slot.back, false);
        print_type(slot.front, true);

        const char *printname = slot.front != nullptr ? slot.front->name.c_str() : "---";
        printw("%s", printname);
}

void print_inventory(int start_line) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        int col_width = max_x / 5;

        for (int r = 0; r < 2; ++r) {               // 2 lines
                for (int c = 0; c < 5; ++c) {       // 5 cols
                        int index = (c * 2) + r;    // 0, 2, 4, 6, 8 (up) | 1, 3, 5, 7, 9 (down)
                        int x_pos = c * col_width;  // start x

                        move(start_line + r, x_pos);

                        printw("[");
                        attron(COLOR_PAIR(5));
                        printw("%d", index);
                        attroff(COLOR_PAIR(5));
                        printw("] ");

                        std::string name = "-";
                        if (index < game::player::inventory.size() && game::player::inventory[index] != nullptr) {
                                name = game::player::inventory[index]->name;
                        }

                        // cut the name
                        int available_space = col_width - 6;
                        if (available_space > 0) {
                                if (name.length() > (size_t)available_space) {
                                        name = name.substr(0, available_space - 2) + "..";
                                }
                                printw("%s", name.c_str());
                        }
                }
        }
}

/*
 * Main
 */

int main(int argc, char **argv) {
        setlocale(LC_ALL, "");
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        start_color();
        use_default_colors();
        init_pair(1, COLOR_RED, -1);
        init_pair(2, COLOR_GREEN, -1);
        init_pair(4, COLOR_BLUE, -1);
        init_pair(5, COLOR_MAGENTA, -1);
        refresh();
        minilog::fdebug(logfile, "started");

        //  TODO: add a cli option to override seed
        std::random_device rd;
        game::seed = rd();

        //  TODO: remove this after adding plugin system
        setup_test();

        minilog::fdebug(logfile, "Generating levels");
        generate_levels();
        game::levelid = game::levels[0]->id;
        game::message = "You are now at level: " + game::levels[game::player::level]->name;

        create_card(1, "~ Exit Gate ~", EXIT, {}, exit_gate);

        minilog::fdebug(logfile, "deck size: ", game::deck.size());
        draw_cards();

        minilog::fdebug(logfile, "card_set size: ", game::card_set.size());
        draw_slots();

        int key = 0;
        while (key != 'q') {
                clear();

                // ui
                mvprintw(0, 0, "%s", game::message.c_str());
                game::message = "";
                print_slot(1, 'a', game::slot1);
                print_slot(2, 'b', game::slot2);
                print_slot(3, 'c', game::slot3);
                mvprintw(4, 0, "HP: %d", game::player::hp);
                print_inventory(5);

                refresh();

                // keyboard handling
                key = getch();
                minilog::fdebug(logfile, "Pressed key: ", key);
                switch (key) {
                        case 'a':
                                minilog::fdebug(logfile, "picked card slot1");
                                handle_slot(game::slot1);
                                break;
                        case 'b':
                                minilog::fdebug(logfile, "picked card slot2");
                                handle_slot(game::slot2);
                                break;
                        case 'c':
                                minilog::fdebug(logfile, "picked card slot3");
                                handle_slot(game::slot3);
                                break;
                        case '0' ... '9':
                                int index = key - '0';
                                minilog::fdebug(logfile, "user tried to use item index: ", index);
                                if (index < game::player::inventory.size()) {
                                        if (game::player::inventory[index]) {
                                                minilog::fdebug(logfile, "calling card event for item: ",
                                                                game::player::inventory[index]->name);

                                                basic_card_event(game::player::inventory[index]);
                                                game::player::inventory[index] = nullptr;
                                        }
                                }
                                break;
                }

                if (game::player::hp == 0) {
                        minilog::fdebug(logfile, "player health is 0");
                        clear();
                        printw("You died stupid!");
                        getch();
                        break;
                }
        }

        endwin();

        minilog::fdebug(logfile, "Exiting with status: 0");
        return 0;
}
