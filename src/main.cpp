#include <ncurses.h>
#include <CLI/CLI.hpp>
#include <clocale>
#include <random>
#include <string>

#include "cards.hpp"
#include "minilog.hpp"
#include "tui.hpp"
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

        auto zombie_buff = create_buff("Zombification", [](auto self) {
                if (self->level == 10) {
                        game::player::hp = 0;
                        log("you are now a zombie!", IMPORTANT);
                }
        });

        // cards
        create_card(5, "Zombie", ENEMY, {}, "You killed a zombie", [=]() {
                zombie_buff->level++;
                return -1;
        });

        auto poison_buff = create_buff("Poison", [](std::shared_ptr<buff_t> self) {
                game::player::hp--;
                self->level--;
        });

        create_card(3, "spider", ENEMY, {mf->id, ms->id}, "You killed a spider", [=]() {
                poison_buff->level += 3;
                return -2;
        });

        create_card(4, "healing", BASIC, {}, "You feel better", []() { return 5; });

        create_card(1, "god", ENEMY, {}, "", []() {
                log("You cant fight with a god!", IMPORTANT);
                game::player::hp = 0;
                return 0;
        });

        create_card(5, "apple", ITEM, {}, "This apple was yummy", []() { return 1; });
        create_card(1, "teleporter", ITEM, {ef->id, mf->id}, "You are teleported", []() {
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

        create_card(2, "whising well", BASIC, {}, "You found a whising well", [=]() {
                std::string whish = ask_string("What you are whising? [hp / zr (zombification reset)]");
                if (whish == "hp") {
                        log("You feel better", NORMAL);
                        return 10;
                } else if (whish == "zr") {
                        log("You're starting to look more like a human.", NORMAL);
                        zombie_buff->level = 0;
                } else {
                        log("Your whish is ignored", NORMAL);
                }
                return 0;
        });
}

void handle_cli(int argc, char **argv) {
        CLI::App app{"crogue - Card Based Roguelike Game"};

        uint64_t custom_seed = 0;
        app.add_option("-s,--seed", custom_seed, "Set game seed");

        try {
                app.parse(argc, argv);
        } catch (const CLI::ParseError &e) {
                exit(app.exit(e));
        }

        // seed
        if (custom_seed) {
                game::seed = custom_seed;
                minilog::fdebug(logfile, "[seed] using custom seed: ", game::seed);

        } else {
                std::random_device rd;
                std::mt19937_64 seed_gen(rd());
                game::seed = seed_gen();
                minilog::fdebug(logfile, "[seed] using random seed: ", game::seed);
        }
}

/*
 * Main
 */

int main(int argc, char **argv) {
        minilog::fdebug(logfile, "---- START ----");

        handle_cli(argc, argv);

        // ncurses things
        setlocale(LC_ALL, "");
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        start_color();
        use_default_colors();

        setup_colors();

        //  TODO: remove this after adding plugin system
        setup_test();

        minilog::fdebug(logfile, "Generating levels");
        generate_levels();
        game::levelid = game::levels[0]->id;
        log("You are now at level: " + game::levels[game::player::level]->name, WARN);

        create_card(1, "~ Exit Gate ~", EXIT, {}, "", exit_gate);

        minilog::fdebug(logfile, "deck size: ", game::deck.size());
        draw_cards();

        minilog::fdebug(logfile, "card_set size: ", game::card_set.size());
        draw_slots();

        int key = 0;
        while (key != 'q') {
                handle_buffs();

                if (check_die()) {
                        break;
                }

                clear();
                print_ui();
                refresh();

                // keyboard handling
                key = getch();
                minilog::fdebug(logfile, "[key] pressed key: ", key);
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
        }

        endwin();

        minilog::fdebug(logfile, "Exiting with status: 0");
        minilog::fdebug(logfile, "---- END ----");
        return 0;
}
