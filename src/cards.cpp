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
        minilog::fdebug(logfile, "new level index: ", game::player::level);
        if (game::player::level == game::levels.size()) {
                clear();
                printw("You are exiting from dungeon with your loot!");
                refresh();
                getch();

                minilog::fdebug(logfile, "player reached last level, exiting with status: 0");
                endwin();
                exit(0);
        }

        game::levelid = game::levels[game::player::level]->id;
        minilog::fdebug(logfile, "new level id: ", game::levelid);

        minilog::fdebug(logfile, "resetting the cards");
        game::card_set = {};

        minilog::fdebug(logfile, "deck size: ", game::deck.size());
        draw_cards();

        minilog::fdebug(logfile, "card_set size: ", game::card_set.size());
        draw_slots();

        game::message = "You are now at level: " + game::levels[game::player::level]->name;
        return 0;
}

/*
 * Levels
 */

// only generates an id
int generate_unique_level_id() {
        static std::mt19937 id_rng(game::seed);
        std::uniform_int_distribution<int> dist(100000, 999999);

        while (true) {
                int new_id = dist(id_rng);

                bool exists = false;
                for (const auto &biome : game::biomes) {
                        for (const auto &lvl : biome->levels) {
                                if (lvl->id == new_id) {
                                        exists = true;
                                        break;
                                }
                        }
                }

                if (!exists)
                        return new_id;
        }
}

std::shared_ptr<level_t> create_level(const std::string name) {
        std::shared_ptr<level_t> new_level = std::make_shared<level_t>();
        new_level->name = name;
        new_level->id = generate_unique_level_id();

        minilog::fdebug(logfile, "[setup] created level. id: \"", new_level->id, "\" name: \"", name, "\"");
        return new_level;
}

void create_biome(const std::string name, const int difficulty, const std::vector<std::shared_ptr<level_t>> &levels) {
        if (difficulty > 100 || difficulty < 0) {
                endwin();
                minilog::fatal(1, "while creating biome: \"", name, "\" : difficulty value should be in range 0-100");
        }

        std::shared_ptr<biome_t> new_biome = std::make_shared<biome_t>();
        new_biome->name = name;
        new_biome->difficulty = difficulty;
        new_biome->levels = levels;

        game::biomes.push_back(new_biome);

        minilog::fdebug(logfile, "created biome with name: \"", name, "\"");
}

void generate_levels() {
        // copy the biomes
        std::vector<std::shared_ptr<biome_t>> sorted_biomes = game::biomes;

        // sort by difficulty
        std::sort(sorted_biomes.begin(), sorted_biomes.end(),
                  [](std::shared_ptr<biome_t> &a, std::shared_ptr<biome_t> &b) {
                          if (a->difficulty != b->difficulty) {
                                  return a->difficulty < b->difficulty;
                          }
                          return a->name < b->name;
                  });

        for (auto &biome : sorted_biomes) {
                for (auto &level : biome->levels) {
                        game::levels.push_back(level);
                }
        }

        minilog::fdebug(logfile, "Levels generated and sorted. Count: ", game::levels.size());
}

/*
 * Cards
 */

void create_card(const int count, const std::string &name, const card_type &type, const std::vector<int> levelids,
                 std::function<int()> event) {

        game::deck.push_back(
            std::pair{count, std::make_shared<card_t>(card_t{name, type, levelids, std::move(event)})});

        minilog::fdebug(logfile, "[setup] added card. name: \"", name, "\" count: \"", count, "\"");
}

void add_card(std::shared_ptr<card_t> cardptr) {
        // empty level_ids means always active card
        if (cardptr->level_ids.empty()) {
                minilog::fdebug(logfile, "[setup] card is avabile for all levels. adding card with name: \"",
                                cardptr->name, '"');
                game::card_set.push_back(cardptr);

        } else if (std::find(cardptr->level_ids.begin(), cardptr->level_ids.end(), game::levelid) !=
                   cardptr->level_ids.end()) {
                minilog::fdebug(logfile, "[setup] level id is matching. adding card with name: \"", cardptr->name, '"');
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
        std::mt19937_64 rng(game::seed ^ (game::player::level + 1));  // some randomizing
        std::shuffle(game::card_set.begin(), game::card_set.end(), rng);

        minilog::fdebug(logfile, "card_set generated. Count: ", game::card_set.size());
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

/*
 * Other
 */

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

        minilog::fdebug(logfile, "filled slots");
}
