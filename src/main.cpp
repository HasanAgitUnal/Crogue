#include <ncurses.h>
#include <clocale>
#include <cstdlib>
#include <string>

#include "cards.hpp"
#include "minilog.hpp"
#include "types.hpp"

void setup_test() {
        create_card(5, "Zombie", ENEMY, []() { return -1; });
        create_card(3, "spider", ENEMY, []() { return -2; });
        create_card(4, "healing", BASIC, []() { return +5; });
        /*
        create_card(1, "god", ENEMY, []() {
                game::player::hp = 0;
                return 0;
        });
        */
}

// for now it just exits
int exit_gate() {
        clear();
        printw("You are exiting from dungeon with your loot!");
        getch();
        endwin();
        exit(0);
        return 0;
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

        setup_test();
        create_card(1, "~ Exit Gate ~", EXIT, exit_gate);
        minilog::fdebug(logfile, "deck size: ", game::deck.size());
        draw_cards();
        minilog::fdebug(logfile, "card_set size: ", game::card_set.size());
        draw_slots();

        int key = 0;
        while (key != 'q') {
                clear();

                // ui
                print_slot(0, 'a', game::slot1);
                print_slot(1, 'b', game::slot2);
                print_slot(2, 'c', game::slot3);
                mvprintw(3, 0, "HP: %d", game::player::hp);

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
