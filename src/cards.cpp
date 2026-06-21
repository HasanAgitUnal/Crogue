#include <ncurses.h>
#include <algorithm>
#include <random>

#include "cards.hpp"
#include "minilog.hpp"
#include "tui.hpp"
#include "types.hpp"

void log(const std::string msg, const log_type type) {
        game::logs.push_back({type, msg});
        minilog::fdebugc("uilog", logfile, msg);

        if (game::logs.size() > 9) {
                game::logs.pop_front();
        }
}

bool check_die() {
        if (game::player::hp <= 0) {
                minilog::fdebugc("player", logfile, "Player died");

                log("You died!", IMPORTANT);

                clear();
                print_ui();
                refresh();

                int key;
                while (true) {
                        key = ask("Press enter or q to quit.");
                        minilog::fdebugc("key", logfile, "pressed key: ", key);
                        if (key == 10 || key == 13 || key == 'q') {
                                break;
                        }
                }

                return 1;
        }

        return 0;
}

int exit_gate() {
        minilog::fdebugc("player", logfile, "player find an exit");
        game::player::level++;
        minilog::fdebugc("setup", logfile, "new level index: ", game::player::level);
        if (game::player::level == (int)game::levels.size()) {
                clear();
                printw("You are exiting from dungeon with your loot!");
                refresh();
                getch();

                minilog::fdebugc("setup", logfile, "player reached last level, exiting with status: 0");
                endwin();
                exit(0);
        }

        game::levelid = game::levels[game::player::level]->id;
        minilog::fdebugc("setup", logfile, "new level id: ", game::levelid);

        minilog::fdebugc("setup", logfile, "resetting the cards");
        game::card_set = {};

        minilog::fdebugc("setup", logfile, "deck size: ", game::deck.size());
        draw_cards();

        minilog::fdebugc("setup", logfile, "card_set size: ", (int)game::card_set.size());
        draw_slots();

        log("You are now at level: " + game::levels[game::player::level]->name, WARN);
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

        minilog::fdebugc("setup", logfile, "created level.id: \"", new_level->id, "\" name: \"", name, "\"");
        return new_level;
}

void create_biome(const std::string name, const int difficulty, const std::vector<std::shared_ptr<level_t>> &levels) {
        if (difficulty > 100 || difficulty < 0) {
                endwin();
                minilog::fout(minilog::msg::fatal, "[setup] While creating biome: \"", name,
                              "\" : difficulty value should be in range 0-100");
                minilog::fatal(1, "While creating biome: \"", name, "\" : difficulty value should be in range 0-100");
        }

        std::shared_ptr<biome_t> new_biome = std::make_shared<biome_t>();
        new_biome->name = name;
        new_biome->difficulty = difficulty;
        new_biome->levels = levels;

        game::biomes.push_back(new_biome);

        minilog::fdebugc("setup", logfile, "Created biome with name: \"", name, "\"");
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

        minilog::fdebugc("setup", logfile, "Levels generated and sorted. Count: ", (int)game::levels.size());
}

/*
 * Buffs
 */

std::shared_ptr<buff_t> create_buff(const std::string name, std::function<void(std::shared_ptr<buff_t>)> event) {
        auto new_buff = std::make_shared<buff_t>(buff_t{name, event, 0});

        new_buff->name = name;
        new_buff->event = event;
        new_buff->level = 0;

        game::buffs.push_back(new_buff);
        return new_buff;
}

void handle_buffs() {
        for (auto buff : game::buffs) {
                if (buff->level != 0) {
                        minilog::fdebugc("event", logfile, "Calling event for buff: ", buff->name);
                        buff->event(buff);
                }
        }
}

/*
 * Cards
 */

void create_card(const int count, const std::string &name, const card_type &type, const std::vector<int> levelids,
                 const std::string &logmsg, std::function<int()> event) {

        game::deck.push_back(
            std::pair{count, std::make_shared<card_t>(card_t{name, type, levelids, std::move(event), logmsg})});

        minilog::fdebugc("setup", logfile, "Created card. name: \"", name, "\" count: \"", count, "\"");
}

void add_card(std::shared_ptr<card_t> cardptr) {
        // empty level_ids means always active card
        if (cardptr->level_ids.empty()) {
                minilog::fdebugc("setup", logfile, "reason: global. Adding card with name: \"", cardptr->name, "\"");
                game::card_set.push_back(cardptr);

        } else if (std::find(cardptr->level_ids.begin(), cardptr->level_ids.end(), game::levelid) !=
                   cardptr->level_ids.end()) {
                minilog::fdebugc("setup", logfile, "reason: level id. Adding card with name: \"", cardptr->name, "\"");
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

        minilog::fdebugc("setup", logfile, "card_set generated. Count: ", (int)game::card_set.size());
}

void basic_card_event(const std::shared_ptr<card_t> card) {
        if (!card->logmsg.empty()) {
                log(card->logmsg, NORMAL);
        }

        minilog::fdebugc("event", logfile, "Calling event for card: ", card->name);
        int result = card->event();
        game::player::hp += result;

        minilog::fdebugc("event", logfile, "card event result=", result);
        minilog::fdebugc("event", logfile, "game::player::hp=", game::player::hp);
}

template <typename... Args>
std::string format(const std::string &fmt, Args... args) {
        size_t size = snprintf(nullptr, 0, fmt.c_str(), args...) + 1;
        if (size <= 0)
                return "";

        std::unique_ptr<char[]> buf(new char[size]);

        snprintf(buf.get(), size, fmt.c_str(), args...);

        return std::string(buf.get(), buf.get() + size - 1);
}

// calls card event
void card_event(const std::shared_ptr<card_t> card) {
        minilog::fdebugc("event", logfile, "Card used: ", card->name);

        switch (card->type) {
                case BASIC:
                        minilog::fdebugc("event", logfile, "card type: BASIC");
                        basic_card_event(card);
                        break;

                // same with basic because differance between basic, enemy and exit is just color on the ui
                case EXIT:
                        minilog::fdebugc("event", logfile, "card type: EXIT");
                        basic_card_event(card);
                        break;

                case ENEMY:
                        minilog::fdebugc("event", logfile, "card type: ENEMY");
                        basic_card_event(card);
                        break;
                case ITEM: {
                        minilog::fdebugc("event", logfile, "card type: ITEM");
                        log(format("You found item: %s", card->name.c_str()), NORMAL);

                        bool added = false;

                        for (int i = 0; i < 10; i++) {
                                if (i >= (int)game::player::inventory.size()) {
                                        game::player::inventory.push_back(card);
                                        minilog::fdebugc("inventory", logfile,
                                                         "added item via push_back to index: ", i);
                                        added = true;
                                        break;
                                }

                                if (game::player::inventory[i] == nullptr) {
                                        game::player::inventory[i] = card;
                                        minilog::fdebugc("inventory", logfile, "added item to empty slot: ", i);
                                        added = true;
                                        break;
                                }
                        }

                        // Inventory full
                        if (!added) {
                                int key = ask("Inventory full. Pick 0-9 to replace: ");

                                if (key >= '0' && key <= '9') {
                                        int index = key - '0';
                                        game::player::inventory[index] = card;
                                        minilog::fdebugc("inventory", logfile, "replaced item at index: ", index);
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

        int old_level = game::player::level;
        card_event(slot.front);

        if (old_level != game::player::level) {
                return;
        }

        // Update card slot
        slot.front = slot.back;

        if (game::card_set.empty()) {
                slot.back = nullptr;
        } else {
                slot.back = game::card_set.back();
                game::card_set.pop_back();
                minilog::fdebugc("setup", logfile, "New card_set size: ", (int)game::card_set.size());
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

        minilog::fdebugc("setup", logfile, "filled slots");
}
