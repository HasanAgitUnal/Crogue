#include <ncurses.h>
#include <algorithm>
#include <random>

#include "cards.hpp"
#include "minilog.hpp"
#include "types.hpp"

// for now it just exits
int exit_gate() {
        minilog::fdebug(logfile, "player find an exit");
        game::player::level++;
        if (game::player::level == 5) {
                clear();
                printw("You are exiting from dungeon with your loot!");
                refresh();
                getch();

                minilog::fdebug(logfile, "player reached level 5, exiting with status: 0");
                endwin();
                exit(0);
        }

        minilog::fdebug(logfile, "resetting the cards");
        game::card_set = {};
        minilog::fdebug(logfile, "deck size: ", game::deck.size());
        draw_cards();
        minilog::fdebug(logfile, "card_set size: ", game::card_set.size());
        draw_slots();

        game::message = "You are now at level: " + std::to_string(game::player::level);
        return 0;
}

void create_card(const int count, const std::string &name, const card_type &type, const int min_level,
                 std::function<int()> event) {

        game::deck.push_back(
            std::pair{count, std::make_shared<card_t>(card_t{name, type, min_level, std::move(event)})});
}

void add_card(std::shared_ptr<card_t> cardptr) {
        if (cardptr->min_level <= game::player::level) {
                game::card_set.push_back(cardptr);
        }
}

// shuffles the deck and fills game::card_set
void draw_cards() {
        for (auto pair : game::deck) {
                int count = pair.first;
                auto card = pair.second;

                while (count > 0) {
                        count--;
                        add_card(card);
                }
        }

        // shuffle the deck
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(game::card_set.begin(), game::card_set.end(), g);
}

void basic_card_event(const std::shared_ptr<card_t> card) {
        int result = card->event();
        game::player::hp += result;

        minilog::fdebug(logfile, "card event result=", result);
        minilog::fdebug(logfile, "game::player::hp=", game::player::hp);
}

// calls card event
void card_event(const std::shared_ptr<card_t> card) {
        minilog::fdebug(logfile, "card used: ", card->name);

        switch (card->type) {
                case BASIC:
                        minilog::fdebug(logfile, "card type: basic");
                        basic_card_event(card);
                        break;

                // same with basic because differance between basic, enemy and exit is just color on the ui
                case EXIT:
                        minilog::fdebug(logfile, "card type: exit gate");
                        basic_card_event(card);
                        break;

                case ENEMY:
                        minilog::fdebug(logfile, "card type: enemy");
                        basic_card_event(card);
                        break;
                case ITEM: {
                        minilog::fdebug(logfile, "card type: item");
                        bool added = false;

                        for (int i = 0; i < 10; i++) {
                                if (i >= game::player::inventory.size()) {
                                        game::player::inventory.push_back(card);
                                        minilog::fdebug(logfile, "added item via push_back to index: ", i);
                                        added = true;
                                        break;
                                }

                                if (game::player::inventory[i] == nullptr) {
                                        game::player::inventory[i] = card;
                                        minilog::fdebug(logfile, "added item to empty slot: ", i);
                                        added = true;
                                        break;
                                }
                        }

                        // Inventory full
                        if (!added) {
                                curs_set(2);
                                mvprintw(0, 0, "Inventory full. Pick 0-9 to replace: ");

                                int key = getch();
                                if (key >= '0' && key <= '9') {
                                        int index = key - '0';
                                        game::player::inventory[index] = card;
                                        minilog::fdebug(logfile, "replaced item at index: ", index);
                                }
                                curs_set(0);
                        }
                        break;
                }
        }
}

// does the job when a slot is picked by user
void handle_slot(card_slot_t &slot) {
        if (!slot.front) {
                return;
        }

        card_event(slot.front);

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

// fils the slots
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
