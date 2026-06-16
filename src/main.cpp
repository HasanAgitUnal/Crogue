#include <ncurses.h>
#include <memory>
#include <string>

#include "minilog.hpp"
#include "test.hpp"
#include "types.hpp"

const std::string logfile = "./build/debug.log";
std::vector<std::shared_ptr<card_t>> &cards = test::cards;

void handle_slot(card_slot_t &slot) {

        if (!slot.front) {
                return;
        }

        {
                int result = slot.front->event();
                minilog::fdebug(logfile, "card event result=", result);
                minilog::fdebug(logfile, "card event result=", result);
        }

        // Update card slot
        slot.front = slot.back;

        if (cards.empty()) {
                slot.back = nullptr;
        } else {
                slot.back = cards.back();
                cards.pop_back();
                minilog::fdebug(logfile, "new cards size: ", cards.size());
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

        // Initialize card slots
        card_slot_t c1 = {*(&cards.back() - 1), cards.back()};
        card_slot_t c2 = {*(&cards.back() - 3), *(&cards.back() - 2)};
        card_slot_t c3 = {*(&cards.back() - 5), *(&cards.back() - 4)};

        for (int i = 0; i < 6; i++) {
                cards.pop_back();
        }

        int key;
        while (key != 'q') {
                clear();

                // ui
                mvprintw(0, 0, "[a] %s", c1.front != nullptr ? c1.front->name.c_str() : "---");
                mvprintw(1, 0, "[b] %s", c2.front != nullptr ? c2.front->name.c_str() : "---");
                mvprintw(2, 0, "[c] %s", c3.front != nullptr ? c3.front->name.c_str() : "---");
                mvprintw(3, 0, "HP: %d", game::player::hp);

                refresh();

                // keyboard handling
                key = getch();
                minilog::fdebug(logfile, "Pressed key: ", key);
                switch (key) {
                        case 'a':
                                minilog::fdebug(logfile, "picked card slot1");
                                handle_slot(c1);
                                break;
                        case 'b':
                                minilog::fdebug(logfile, "picked card slot2");
                                handle_slot(c2);
                                break;
                        case 'c':
                                minilog::fdebug(logfile, "picked card slot3");
                                handle_slot(c3);
                                break;
                }

                if (game::player::hp == 0) {
                        minilog::fdebug(logfile, "player healt is 0");
                        break;
                }
        }

        endwin();

        minilog::fdebug(logfile, "Exiting with status: 0");
        return 0;
}
