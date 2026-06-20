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
        init_pair(1, COLOR_RED, -1);
        init_pair(2, COLOR_GREEN, -1);
        init_pair(4, COLOR_BLUE, -1);
        init_pair(5, COLOR_MAGENTA, -1);
        refresh();

        minilog::fdebug(logfile, "started");

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
                print_ui();
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
