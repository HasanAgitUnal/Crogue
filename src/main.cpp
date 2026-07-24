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
#include <ncurses.h>
#include <unistd.h>
#include <CLI/CLI.hpp>
#include <clocale>
#include <csignal>
#include <cstdlib>
#include <random>
#include <string>

#include "game.hpp"
#include "lua.hpp"
#include "minilog.hpp"
#include "package.hpp"
#include "scenes.hpp"
#include "tui.hpp"

bool game_running = false;

#ifdef DEBUG
#include <execinfo.h>
#include <boost/stacktrace.hpp>
#endif

void segfault_handler(int sig) {
        if (game_running) {
                end_program();
        }

#ifdef DEBUG
        minilog::err(minilog::msg::error, "=== SEGMENTATION FAULT ===\033[0m\n");
        minilog::err(boost::stacktrace::stacktrace());

#endif
        exit(139);
}

void interrupt_handler(int sig) {
        if (game_running) {
                end_program();
        }
        exit(130);
}

void handle_cli(int argc, char **argv) {
        CLI::App app{"crogue - Card Based Roguelike Game"};

        uint64_t custom_seed = 0;

        app.add_option("-s,--seed", custom_seed, "Set game seed");

        app.add_flag("-m,--skip-menu", game::_skip_main_menu, "Skip Main Menu");

        // subcommands
        auto *pack_cmd = app.add_subcommand("pack", "Create a plugin package");
        auto *list_cmd = app.add_subcommand("list", "List installed plugins");

        bool force = false;
        std::string plugin_name;
        auto *remove_cmd = app.add_subcommand("remove", "Remove a plugin");
        remove_cmd->add_option("PLUGIN", plugin_name, "Plugin name")->required();
        remove_cmd->add_flag("-F,--force", force, "Force to remove");

        auto *reset_cmd = app.add_subcommand("reset", "Reset plugin settings");
        reset_cmd->add_option("PLUGIN", plugin_name, "Plugin name")->required();
        reset_cmd->add_flag("-F,--force", force, "Force to reset");

        auto *install_cmd = app.add_subcommand("install", "Install one or multiple packages");

        install_cmd->add_flag("-F,--force", force, "Force installation");

        std::vector<std::string> pack_files;
        install_cmd->add_option("-f,--file", pack_files, "Package file name(s)");

        std::vector<std::string> pack_repos;
        install_cmd->add_option("-g,--git-repo", pack_repos, "Package git repo(s)");

        /*
        auto *install_cmd = app.add_subcommand("install", "Install a package");

        bool install_force = false;
        install_cmd->add_flag("-F,--force", install_force, "Force installation / overwrite");

        // Opsiyon grubunu oluştur
        auto *src_group = install_cmd->add_option_group("source", "Package Source");

        std::string pack_file;
        src_group->add_option("-f,--file", pack_file, "Package file name");

        std::string pack_repo;
        src_group->add_option("-g,--git-repo", pack_repo, "Package git repo");

        // Bu gruptan sadece 1 tanesi girilebilir
        src_group->require_option(1);

        */

        //// parse

        try {
                app.parse(argc, argv);
        } catch (const CLI::ParseError &e) {
                exit(app.exit(e));
        }

        if (*pack_cmd) {
                package::pack();
                exit(0);
        }

        if (*list_cmd) {
                package::list();
                exit(0);
        }

        if (*remove_cmd) {
                package::remove(plugin_name, force);
                exit(0);
        }

        if (*reset_cmd) {
                package::reset(plugin_name);
                exit(0);
        }

        if (*install_cmd) {
                if (pack_files.empty() && pack_repos.empty()) {
                        minilog::fatal(1, "At least one package source (-f or -g) must be specified.");
                }

                for (const auto &file : pack_files) {
                        minilog::out("\033[32m==>\033[0m Installing plugin from file: ", file);
                        package::install_file(file, force);
                }

                for (const auto &repo : pack_repos) {
                        minilog::out("\033[32m==>\033[0m Installing plugin via git: ", repo);
                        package::install_git(repo, force);
                }

                exit(0);
        }

        /*
                if (*install_cmd) {
                        if (!pack_file.empty()) {
                                minilog::out("\033[32m==>\033[0m Installing plugin from file: ", pack_file);
                                package::install_file(pack_file, force);
                        } else if (!pack_repo.empty()) {
                                minilog::out("\033[32m==>\033[0m Installing plugin via git: ", pack_repo);
                                package::install_git(pack_repo, force);
                        }

                        exit(0);
                }
        */

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
        signal(SIGTERM, interrupt_handler);
        signal(SIGINT, interrupt_handler);

        // setup minilog categories
        minilog::categories["seed"] = "3;96m";
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
        minilog::categories["settings"] = "38;5;49m";
        minilog::categories["cli"] = "38;5;178m";

        handle_cli(argc, argv);

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

        game_running = true;

        try {
                // start game
                scene::main_menu();

        } catch (std::exception &e) {
                end_program();
                minilog::fatal(1, "An exception throwed: ", e.what());
        }

        end_program();
        return 0;
}
