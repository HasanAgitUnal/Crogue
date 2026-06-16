#include <ncurses.h>
#include <memory>
#include <string>

#include "minilog.hpp"
#include "test.hpp"
#include "types.hpp"

const std::string logfile = "./build/debug.log";

void handle_slot(card_slot_t &slot) {
        if (!slot.front) {
                return;
        }

        int result = slot.front->event();
        game::player::hp += result;
        minilog::fdebug(logfile, "game::player::hp=", game::player::hp);
        minilog::fdebug(logfile, "card event result=", result);

        // Update card slot
        slot.front = slot.back;

        if (game::card_set.empty()) {
                slot.back = nullptr;
        } else {
                slot.back = game::card_set.back();
                game::card_set.pop_back();
                minilog::fdebug(logfile, "new game::card_set size: ", game::card_set.size());
        }
}

void draw_cards() {
        game::slot1 = {*(&game::card_set.back() - 1), game::card_set.back()};
        game::slot2 = {*(&game::card_set.back() - 3), *(&game::card_set.back() - 2)};
        game::slot3 = {*(&game::card_set.back() - 5), *(&game::card_set.back() - 4)};

        for (int i = 0; i < 6; i++) {
                game::card_set.pop_back();
        }
}

/*
 * Main
 */

int main(int argc, char **argv) {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        refresh();
        minilog::fdebug(logfile, "started");

        setup_test();
        game::card_set = test::cards;
        draw_cards();

        int key;
        while (key != 'q') {
                clear();

                // ui
                mvprintw(0, 0, "[a] %s", game::slot1.front != nullptr ? game::slot1.front->name.c_str() : "---");
                mvprintw(1, 0, "[b] %s", game::slot2.front != nullptr ? game::slot2.front->name.c_str() : "---");
                mvprintw(2, 0, "[c] %s", game::slot3.front != nullptr ? game::slot3.front->name.c_str() : "---");
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
                        break;
                }
        }

        endwin();

        minilog::fdebug(logfile, "Exiting with status: 0");
        return 0;
}
