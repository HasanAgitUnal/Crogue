#include <ncurses.h>
#include <cstdlib>
#include <memory>
#include <string>

#include "cards.hpp"
#include "minilog.hpp"
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
                minilog::fdebug(logfile, "new card_set size: ", game::card_set.size());
        }
}

void draw_slots() {
        auto pop_card = []() -> std::shared_ptr<card_t> {
                if (game::card_set.empty())
                        return nullptr;
                auto c = game::card_set.back();
                game::card_set.pop_back();
                return c;
        };

        // Fill all slots
        for (auto *s : {&game::slot1, &game::slot2, &game::slot3}) {
                s->back = pop_card();
                s->front = pop_card();
        }
}

void setup_test() {
        create_card(5, "Zombie", []() { return -1; });
        create_card(3, "spider", []() { return -2; });
        create_card(4, "healing", []() { return +5; });
        create_card(1, "god", []() {
                game::player::hp = 0;
                return 0;
        });
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
        create_card(1, "~ Exit Gate ~", exit_gate);
        minilog::fdebug(logfile, "deck size: ", game::deck.size());
        draw_cards();
        minilog::fdebug(logfile, "card_set size: ", game::card_set.size());
        draw_slots();

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
