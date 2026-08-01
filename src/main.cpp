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


#ifdef DEBUG
#define DEBUG_TAG "-debug"
#else
#define DEBUG_TAG ""
#endif

#define CROGUE_VERSION "0.1" DEBUG_TAG


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

        // options & flags //
        bool version = false;
        app.add_flag("-v,--version", version, "Print version");

        uint64_t custom_seed = 0;
        app.add_option("-s,--seed", custom_seed, "Set game seed");

        std::string custom_data_dir = "";
        app.add_option("-d,--data", custom_data_dir, "Custom data directory location");

        bool default_data = false;
        app.add_flag("-w,--where-is-my-data", default_data, "Default data directory location for your system");

        app.add_flag("-m,--skip-menu", game::_skip_main_menu, "Skip Main Menu");

        // parse options before subcommands //
        app.parse_complete_callback([&]() {
                if (version) {
                        minilog::out("CROGUE " CROGUE_VERSION "\nCard Based Roguelike Game");
                        exit(0);
                }

                if (default_data) {
                        minilog::out(get_data_dir().string());
                        exit(0);
                }

                // data dir
                if (custom_data_dir.empty()) {
                        game::_data_directory = get_data_dir();
                } else {
                        fs::path dir(custom_data_dir);
                        if (!fs::exists(dir) || !fs::is_directory(dir)) {
                                minilog::fatal(1, dir, " doesn't exists or not a directory");
                        }
                        game::_data_directory = custom_data_dir;
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
        });


        // plugin management //
        auto *pack_cmd = app.add_subcommand("pm", "Manage plugins");

        // build
        auto *build_cmd = pack_cmd->add_subcommand("build", "Create a plugin package at current working directory");

        build_cmd->callback([]() {
                package::pack();
                exit(0);
        });

        // list
        auto *list_cmd = pack_cmd->add_subcommand("list", "List installed plugins");

        list_cmd->callback([]() {
                package::list();
                exit(0);
        });

        // remove
        bool force = false;
        std::string plugin_name;

        auto *remove_cmd = pack_cmd->add_subcommand("remove", "Remove a plugin");
        remove_cmd->add_option("PLUGIN", plugin_name, "Plugin name")->required();
        remove_cmd->add_flag("-F,--force", force, "Force to remove");
        remove_cmd->callback([&]() {
                package::remove(plugin_name, force);
                exit(0);
        });

        // reset
        auto *reset_cmd = pack_cmd->add_subcommand("reset", "Reset plugin settings");
        reset_cmd->add_option("PLUGIN", plugin_name, "Plugin name")->required();

        reset_cmd->callback([&]() {
                package::reset(plugin_name);
                exit(0);
        });

        // update
        std::vector<std::string> update_plugins;
        auto *update_cmd = pack_cmd->add_subcommand("update", "Update plugins installed from git");
        update_cmd->add_option("PLUGINS", update_plugins, "Plugin name(s) to update");

        update_cmd->callback([&]() {
                package::update(update_plugins);
                exit(0);
        });

        // install
        auto *install_cmd = pack_cmd->add_subcommand("install", "Install one or multiple packages");

        install_cmd->add_flag("-F,--force", force, "Force installation");

        std::vector<std::string> pack_files;
        install_cmd->add_option("-f,--file", pack_files, "Package file name(s)");

        std::vector<std::string> pack_repos;
        install_cmd->add_option("-g,--git-repo", pack_repos, "Package git repo(s)");

        install_cmd->callback([&]() {
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
        });

        pack_cmd->require_subcommand(1);


        // parse //

        try {
                app.parse(argc, argv);
        } catch (const CLI::ParseError &e) {
                exit(app.exit(e));
        }
}

/*
 * Main
 */

int main(int argc, char **argv) {
        signal(SIGSEGV, segfault_handler);
        signal(SIGTERM, interrupt_handler);
        signal(SIGINT, interrupt_handler);

        // debug logs
#ifdef DEBUG
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
        minilog::categories["saves"] = "38;5;101m";
#endif

        handle_cli(argc, argv);

        minilog::fdebugc("game", logfile, "Data directory: ", game::_data_directory.string());

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
