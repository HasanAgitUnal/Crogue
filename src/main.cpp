// CROGUE - Card-based rogulike TUI game
// Copyright (C) 2026  Hasan Agit Ünal
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#define BOOST_STACKTRACE_USE_ADDR2LINE
#include <execinfo.h>
#include <ncurses.h>
#include <unistd.h>
#include <CLI/CLI.hpp>
#include <boost/stacktrace.hpp>
#include <clocale>
#include <csignal>
#include <cstdlib>
#include <random>
#include <string>

#include "cards.hpp"
#include "lua.hpp"
#include "minilog.hpp"
#include "scenes.hpp"
#include "tui.hpp"
#include "types.hpp"

void segfault_handler(int sig) {
        endwin();

        minilog::err(minilog::msg::error, "=== SEGMENTATION FAULT ===\033[0m\n");
        minilog::err(boost::stacktrace::stacktrace());

        exit(139);
}

void handle_cli(int argc, char **argv) {
        CLI::App app{"crogue - Card Based Roguelike Game"};

        uint64_t custom_seed = 0;
        app.add_option("-s,--seed", custom_seed, "Set game seed");

        app.add_flag("-m,--skip-menu", game::_skip_main_menu, "Skip Main Menu");

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
        signal(SIGSEGV, segfault_handler);

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
        minilog::categories["lua"] = "38;5;21m";

        minilog::fout(logfile, minilog::msg::info, "---- START ----");

        handle_cli(argc, argv);

        setup_lua();

        // ncurses things
        setlocale(LC_ALL, "");
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        set_escdelay(25);
        curs_set(0);
        start_color();
        use_default_colors();

        setup_colors();

        // start game
        scene::main_menu();

        cleanup_lua();
        endwin();
        minilog::fdebugc("setup", logfile, "Exiting with status: 0");
        minilog::fout(logfile, minilog::msg::info, "----  END  ----");
        return 0;
}
