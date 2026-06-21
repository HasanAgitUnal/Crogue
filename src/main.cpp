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
                minilog::fdebugc("seed", logfile, "using custom seed: ", game::seed);

        } else {
                std::random_device rd;
                std::mt19937_64 seed_gen(rd());
                game::seed = seed_gen();
                minilog::fdebugc("seed", logfile, "using random seed: ", game::seed);
        }
}

/*
 * Main
 */

int main(int argc, char **argv) {
        // setup minilog categories
        minilog::categories["seed"] = "3;98m";
        minilog::categories["setup"] = "32m";
        minilog::categories["event"] = "36m";
        minilog::categories["player"] = "1;96m";
        minilog::categories["inventory"] = "34m";
        minilog::categories["game"] = "35m";
        minilog::categories["key"] = "95m";
        minilog::categories["ui"] = "33m";
        minilog::categories["ask"] = "93m";
        minilog::categories["uilog"] = "1;3;93m";
        minilog::categories["test"] = "31m";

        minilog::fout(logfile, minilog::msg::info, "---- START ----");

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
        minilog::fdebugc("setup", logfile, "Exiting with status: 0");
        minilog::fout(logfile, minilog::msg::info, "----  END  ----");
        return 0;
}
