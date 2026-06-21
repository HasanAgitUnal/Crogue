#include <ncurses.h>
#include <CLI/CLI.hpp>
#include <clocale>
#include <random>
#include <string>

#include "minilog.hpp"
#include "scenes.hpp"
#include "tui.hpp"
#include "types.hpp"

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

        // start game
        scene::game();

        endwin();
        minilog::fdebug(logfile, "[setup] Exiting with status: 0");
        minilog::fdebug(logfile, "---- END ----");
        return 0;
}
