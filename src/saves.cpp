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

#include <chrono>
#include <cinttypes>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

#include "game.hpp"
#include "minilog.hpp"
#include "package.hpp"
#include "tui.hpp"

using json = nlohmann::json;

namespace fs = std::filesystem;

namespace saves {

uint64_t get_unix_timestamp() {
        using namespace std::chrono;
        return static_cast<uint64_t>(duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

std::string check_save_data(const json &save) {
        if (!save.is_object()) {
                return "not an object";
        }

        std::vector<std::string> required = {
            "name", "seed", "hp", "level", "last_played", "created_with_plugins", "plugins_changed", "inventory"};

        for (auto key : required) {
                if (!save.contains(key)) {
                        return "\"" + key + "\" key not found";
                }
        }

        if (!save["name"].is_string()) {
                return "\"name\" is not a string";
        }

        if (!save["seed"].is_number_integer()) {
                return "\"seed\" is not an integer";
        }

        if (!save["hp"].is_number_integer()) {
                return "\"hp\" is not an integer";
        }

        if (!save["level"].is_number_integer()) {
                return "\"level\" is not an integer";
        }

        if (!save["last_played"].is_number_integer()) {
                return "\"last_played\" is not an integer";
        }

        if (!save["plugins_changed"].is_boolean()) {
                return "\"plugins_changed\" is not a bool";
        }

        if (!save["created_with_plugins"].is_object()) {
                return "\"created_with_plugins\" is not an object";
        }

        for (auto &[plugin, source] : save["created_with_plugins"].items()) {
                if (!source.is_string()) {
                        return "source of the \"" + plugin + "\" is not a string";
                }
        }

        if (!save["inventory"].is_array()) {
                return "\"inventory\" is not an object";
        }

        if (save["inventory"].size() != 10) {
                return "\"inventory\" size is not 10";
        }

        for (const json &item : save["inventory"]) {
                if (!item.is_string() && !item.is_null()) {
                        return "An inventory item is not a string or null";
                }
        }

        return "";
}

std::string save(json save_data) {
        minilog::fdebugc("saves", logfile, "Saving a save with name: ", save_data["name"].get<std::string>());
        fs::create_directories(game::_data_directory / "saves");

        std::string file_name = std::to_string(save_data["seed"].get<uint64_t>()) + "_" +
                                std::to_string(save_data["last_played"].get<uint64_t>()) + ".json";

        fs::path save_path = game::_data_directory / "saves" / file_name;
        std::ofstream file(save_path);
        file << save_data.dump(4);
        file.close();

        return save_path.string();
}

// returns array contains inventory data. example return:
// [ vanilla:apple, vanilla:teleporter, null, vanilla:apple, null, null, null, null, null, null ]
json get_inventory() {
        json inventory = json::array();

        for (int i = 0; i < 10; i++) {
                auto item = game::player::inventory[i];
                if (item == nullptr) {
                        inventory.push_back(nullptr);
                } else {
                        inventory.push_back(item->id);
                }
        }

        return inventory;
}

std::string save_curr() {
        json plugins = json::object();
        if (game::_curr_save_created_with_plugins.empty()) {
                package::list(&plugins);
                game::_curr_save_created_with_plugins = plugins;
        } else {
                plugins = game::_curr_save_created_with_plugins;
        }

        json save_data = json::object();

        if (game::_curr_save_name.empty()) {
                game::_curr_save_name = "No name";
        }

        save_data["name"] = game::_curr_save_name;
        save_data["seed"] = game::seed;
        save_data["hp"] = game::player::hp;
        save_data["level"] = game::player::level;
        auto timestamp = get_unix_timestamp();
        save_data["last_played"] = timestamp;
        save_data["created_with_plugins"] = plugins;
        save_data["plugins_changed"] = game::_plugins_changed;
        save_data["inventory"] = get_inventory();

        return save(save_data);
}

json load(const fs::path path) {
        minilog::fdebugc("saves", logfile, "loading save: ", path.string());
        std::ifstream file(path);
        if (!file.is_open()) {
                minilog::fdebugc("saves", logfile, "can't open save file");
                throw std::runtime_error("Failed to open save file: " + path.string());
        }

        json save_data;

        try {
                file >> save_data;
        } catch (json::parse_error &e) {
                file.close();
                minilog::fdebugc("saves", logfile, "save has invalid JSON format");
                throw std::runtime_error("Invalid JSON: " + std::string(e.what()));
        }
        file.close();

        std::string result = check_save_data(save_data);
        if (!result.empty()) {
                minilog::fdebugc("saves", logfile, "save is invalid: ", result);
                throw std::runtime_error("Save is invalid: " + result);
        }

        return save_data;
}

std::shared_ptr<card_t> find_item(const std::string &id) {
        for (auto &card : game::deck) {
                if (card->id == id) {
                        if (card->type != ITEM) {
                                break;
                        }

                        return card;
                }
        }

        return nullptr;
}

void apply_save(const json save) {
        minilog::fdebugc("saves", logfile, "applying a save with name: ", save["name"].get<std::string>());
        game::seed = save["seed"].get<uint64_t>();
        game::player::hp = save["hp"].get<int>();
        game::player::level = save["level"].get<int>();

        game::player::inventory.clear();
        game::player::inventory.resize(10, nullptr);

        for (int i = 0; i < 10; i++) {
                const json &item = save["inventory"][i];
                if (!item.is_null()) {
                        auto card = find_item(item.get<std::string>());
                        if (card == nullptr) {
                                throw std::runtime_error("Can't apply save: inventory contains invalid items");
                        }

                        game::player::inventory[i] = card;
                }
        }

        game::_plugins_changed = save["plugins_changed"].get<bool>();
        game::_curr_save_created_with_plugins = save["created_with_plugins"];
        game::_curr_save_loaded = save["_filepath"].get<std::string>();
        game::_curr_save_name = save["name"].get<std::string>();
}

void sync_with_plugins(json &saves) {
        json plugins = json::object();
        package::list(&plugins);

        for (auto &s : saves) {
                bool old_state = s.value("plugins_changed", false);
                bool new_state = (plugins != s["created_with_plugins"]);

                // write if changed
                if (old_state != new_state) {
                        s["plugins_changed"] = new_state;
#ifdef DEBUG
                        if (new_state) {
                                minilog::fdebugc("saves", logfile, "found a broken save");
                        }
#endif
                        save(s);
                }
        }
}

json get_saves() {
        fs::path saves_dir = game::_data_directory / "saves";
        json saves = json::array();

        if (!fs::exists(saves_dir)) {
                return saves;
        }

        for (const auto &entry : fs::directory_iterator(saves_dir)) {
                fs::path path = entry.path();

                if (fs::is_regular_file(path) && path.extension() == ".json") {
                        try {
                                json save_data = load(path);
                                save_data["_filepath"] = path.string();
                                saves.push_back(save_data);
                        } catch (const std::exception &e) {
                                // skip invalids
                                continue;
                        }
                }
        }

        sync_with_plugins(saves);

        // sort by last played date
        std::sort(saves.begin(), saves.end(), [](const json &a, const json &b) {
                return a.value("last_played", 0ULL) > b.value("last_played", 0ULL);
        });

        return saves;
}

std::string get_relative_time_str(int64_t diff_sec) {
        if (diff_sec < 60) {
                return "just now";
        }

        int64_t minutes = diff_sec / 60;
        if (minutes < 60) {
                return std::to_string(minutes) + (minutes == 1 ? " minute ago" : " minutes ago");
        }

        int64_t hours = minutes / 60;
        if (hours < 24) {
                return std::to_string(hours) + (hours == 1 ? " hour ago" : " hours ago");
        }

        int64_t days = hours / 24;
        if (days < 30) {
                return std::to_string(days) + (days == 1 ? " day ago" : " days ago");
        }

        int64_t months = days / 30;
        if (months < 12) {
                return std::to_string(months) + (months == 1 ? " month ago" : " months ago");
        }

        int64_t years = days / 365;
        return std::to_string(years) + (years == 1 ? " year ago" : " years ago");
}

std::string get_formatted_date(const uint64_t timestamp) {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto save_time = system_clock::time_point(seconds(timestamp));

        std::time_t now_c = system_clock::to_time_t(now);
        std::time_t save_c = system_clock::to_time_t(save_time);

        std::tm tm_now = *std::localtime(&now_c);
        std::tm tm_save = *std::localtime(&save_c);

        int64_t diff_sec = duration_cast<seconds>(now - save_time).count();
        if (diff_sec < 0)
                diff_sec = 0;

        std::string relative_str = get_relative_time_str(diff_sec);
        std::ostringstream ss;

        // today
        if (tm_now.tm_year == tm_save.tm_year && tm_now.tm_yday == tm_save.tm_yday) {
                ss << "Today, " << std::put_time(&tm_save, "%H:%M") << " (" << relative_str << ")";
        }
        // yesterday
        else if (tm_now.tm_year == tm_save.tm_year && tm_now.tm_yday == tm_save.tm_yday + 1) {
                ss << "Yesterday, " << std::put_time(&tm_save, "%H:%M") << " (" << relative_str << ")";
        }
        // this year
        else if (tm_now.tm_year == tm_save.tm_year) {
                ss << std::put_time(&tm_save, "%d %b, %H:%M") << " (" << relative_str << ")";
        }
        // past years
        else {
                ss << std::put_time(&tm_save, "%d %b %Y, %H:%M") << " (" << relative_str << ")";
        }

        return ss.str();
}

void print_item(WINDOW *win, int line, json &item, bool hover) {
        if (hover) {
                wattr_set(win, A_UNDERLINE, (int)314, NULL);
        }

        mvwprintw(win, line, 1, "%s", item["name"].get<std::string>().c_str());

        wattr_set(win, A_NORMAL, 0, NULL);
}

std::shared_ptr<card_t> find_card(const std::string &id) {
        for (auto &card : game::deck) {
                if (card->id == id) {
                        return card;
                }
        }

        return nullptr;
}

void update_details(WINDOW *win, json &item) {
        werase(win);
        mvwprintw(win, 0, 0, "Save Name         : %s", item["name"].get<std::string>().c_str());
        mvwprintw(win, 1, 0, "Last Played       : %s", get_formatted_date(item["last_played"].get<uint64_t>()).c_str());
        mvwchgat(win, 0, 0, 9, A_NORMAL, 7, NULL);
        mvwchgat(win, 1, 0, 11, A_NORMAL, 7, NULL);

        mvwprintw(win, 3, 0, "HP                : %d", item["hp"].get<int>());
        mvwprintw(win, 4, 0, "Level             : %s", to_roman(item["level"].get<int>()).c_str());
        mvwprintw(win, 5, 0, "Seed              : %" PRIu64, item["seed"].get<uint64_t>());
        mvwchgat(win, 3, 0, 2, A_NORMAL, 7, NULL);
        mvwchgat(win, 4, 0, 5, A_NORMAL, 7, NULL);
        mvwchgat(win, 5, 0, 4, A_NORMAL, 7, NULL);

        for (int i = 0; i < 6; i++) {
                mvwchgat(win, i, 18, 1, A_NORMAL, 9, NULL);
        }

        mvwprintw(win, 7, 0, "Inventory");
        mvwchgat(win, 7, 0, 9, A_NORMAL, 7, NULL);
        print_line(8, win);
        int line = 9;
        for (int i = 0; i < 10; i++) {
                const json &inv_item = item["inventory"][i];

                if (inv_item.is_null()) {
                        continue;
                }

                std::string id = inv_item.get<std::string>();
                auto card = find_card(id);
                if (card == nullptr) {
                        wattron(win, COLOR_PAIR(4));
                        mvwprintw(win, line, 0, "* Unknown Item (%s)", id.c_str());
                        mvwchgat(win, line, 0, 1, A_NORMAL, 6, NULL);
                        wattroff(win, COLOR_PAIR(4));
                } else {
                        mvwprintw(win, line, 0, "* %s", card->name.c_str());
                        mvwchgat(win, line, 0, 1, A_NORMAL, 6, NULL);
                }
                line++;
        }

        if (line == 9) {
                mvwprintw(win, 9, 0, "Empty inventory");
                line++;
        }

        if (item["plugins_changed"].get<bool>()) {
                wattron(win, COLOR_PAIR(4));
                mvwprintw(win, line + 1, 0, "Warning: Plugins are changed after this save created");
                wattroff(win, COLOR_PAIR(4));
        }
}

void draw_layout_decorations(int max_y, int max_x) {


        mvprintw(0, 0, " Select Save                            │ Details");
        attron(COLOR_PAIR(9));
        mvprintw(0, 40, "│");
        attroff(COLOR_PAIR(9));

        print_line(1);

        attron(COLOR_PAIR(9));
        mvprintw(1, 40, "┼");

        for (int line = 2; line < max_y - 2; line++) {
                mvprintw(line, 40, "│");
        }
        attroff(COLOR_PAIR(9));

        print_line(max_y - 2);

        attron(COLOR_PAIR(9));
        mvprintw(max_y - 2, 40, "┴");
        attroff(COLOR_PAIR(9));

        mvprintw(max_y - 1, 0, " [ENTER] Load, [d] Delete, [q] Back, [r] Recover");
        wnoutrefresh(stdscr);
}

void recovery(json &save, std::function<void(void)> stop_curses, std::function<void(void)> reload) {
        minilog::out("\033[34m::\033[0m Recovery started");
        // detect recoverable plugins
        json installed_plugins = json::object();
        package::list(&installed_plugins);

        const json &plugins = save["created_with_plugins"];
        json install_plugins = json::array();
        json plugins_to_copy = json::array();

        std::string unrecoverable = "";

        for (const auto &[plugin_name, source] : plugins.items()) {
                if (installed_plugins.contains(plugin_name)) {
                        // copy if already a copy of the plugin installed
                        plugins_to_copy.push_back(plugin_name);

                } else if (source.get<std::string>() != "local") {
                        // install it from git
                        install_plugins.push_back(source);

                } else {
                        unrecoverable = "A plugin with name \"" + plugin_name + "\"" +
                                        " has unknown source. Only plugins exists in the current data "
                                        "directory and git plugins are recoverable.";
                        break;
                }
        }

        if (!unrecoverable.empty()) {
                clear();
                mvprintw(0, 0, "Can't recover this save: ");
                attron(COLOR_PAIR(2));
                mvprintw(1, 0, "%s", unrecoverable.c_str());
                attroff(COLOR_PAIR(2));
                refresh();
                press_enter_to_continue();
                return;
        }

        clear();
        mvprintw(0, 0,
                 "All plugins are recoverable, do not run any crogue pm commands while "
                 "recovery is running.");
        refresh();
        press_enter_to_continue();


        // stop ncurses to show normal terminal
        stop_curses();

        // copy/install plugins
        fs::path temp_dir = package::create_temp_dir();

        fs::create_directory(temp_dir / "plugins");
        for (auto &plugin : plugins_to_copy) {
                minilog::out("\033[32m==>\033[0m Copying from data directory: ", plugin.get<std::string>());
                fs::path dir = game::_data_directory / "plugins" / plugin.get<std::string>();

                std::error_code ec;
                fs::copy(dir, temp_dir / "plugins" / plugin.get<std::string>(), fs::copy_options::recursive, ec);
                if (ec) {
                        minilog::err(minilog::msg::error, "Copying failed: ", ec.message());
                        minilog::err("Press Enter to continue...");
                        std::string input;
                        std::getline(std::cin, input);
                        reload();
                        fs::remove_all(temp_dir);
                        return;
                }
        }

        std::string install_cmd = game::argv0 + " --data " + temp_dir.string() + " pm install --force ";

        bool install = false;
        for (auto &repo : install_plugins) {
                minilog::out("\033[32m==>\033[0m Adding ", repo.get<std::string>(), " to install...");
                install_cmd += "-g " + repo.get<std::string>() + " ";

                install = true;
        }

        if (install) {
                minilog::out("\033[32m==>\033[0m Running: ", install_cmd);
                int status = std::system(install_cmd.c_str());

                if (status != 0) {
                        minilog::err(minilog::msg::error, "Failed to recover\033[0m");
                        minilog::err("Press Enter to continue...");
                        std::string input;
                        std::getline(std::cin, input);

                        reload();
                        fs::remove_all(temp_dir);
                        return;
                }
        }

        minilog::out("\033[32m==>\033[0m All plugins are recovered");

        // create save file
        minilog::out("\033[32m==>\033[0m Creating recovery.json file at temporary directory");
        save["plugins_changed"] = false;
        fs::remove(save["_filepath"].get<std::string>());
        save.erase("_filepath");

        fs::create_directory(temp_dir / "saves");

        std::ofstream file(temp_dir / "saves" / "recovery.json");
        file << save;
        file.close();

        // launch crogue
        std::string run_cmd = game::argv0 + " --data " + temp_dir.string() + " --load-save " +
                              (temp_dir / "saves" / "recovery.json").string() + " ";

        minilog::out("\033[32m==>\033[0m Running: ", run_cmd);
        int status = std::system(run_cmd.c_str());

        if (status != 0) {
                minilog::err("Press Enter to continue...");
                std::string input;
                std::getline(std::cin, input);
                fs::remove_all(temp_dir);
                reload();
        }

        // get new saves
        for (auto &entry : fs::directory_iterator(temp_dir / "saves")) {
                fs::path path = entry.path();

                if (!fs::is_regular_file(path))
                        continue;

                if (path.filename().string() == "recovery.json") {
                        save["plugins_changed"] = true;
                        saves::save(save);
                        continue;
                }

                fs::copy(path, game::_data_directory / "saves" / path.filename().string());
        }

        reload();
        fs::remove_all(temp_dir);
}

bool saves_tui() {
        json saves_list = get_saves();

        minilog::fdebugc("saves", logfile, "saves_list: ", saves_list.dump());

        if (saves_list.empty()) {
                clear();
                mvprintw(0, 0, "No save found");
                refresh();
                press_enter_to_continue();
                return false;
        }

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        int selected_idx = 0;
        int pad_top = 0;

        int pad_height = saves_list.size();
        WINDOW *pad = newpad(pad_height, 38);

        int details_height = max_y - 4;
        int details_width = max_x - 42;
        WINDOW *details_win = newwin(details_height, details_width, 2, 42);

        keypad(stdscr, TRUE);
        curs_set(0);

        bool redraw_all = true;

        int key = 0;
        while (key != 'q') {
                if (saves_list.empty()) {
                        clear();
                        mvprintw(0, 0, "No save remain");
                        refresh();
                        press_enter_to_continue();
                        delwin(pad);
                        delwin(details_win);
                        return false;
                }

                getmaxyx(stdscr, max_y, max_x);
                int viewport_height = max_y - 3;

                if (viewport_height < 1)
                        viewport_height = 1;

                if (redraw_all) {
                        clear();
                        draw_layout_decorations(max_y, max_x);
                        redraw_all = false;
                }

                // items
                werase(pad);
                for (size_t i = 0; i < saves_list.size(); ++i) {
                        print_item(pad, (int)i, saves_list[i], (int)i == selected_idx);
                }

                // scroll
                if (selected_idx < pad_top) {
                        pad_top = selected_idx;
                } else if (selected_idx >= pad_top + viewport_height) {
                        pad_top = selected_idx - viewport_height + 1;
                }

                update_details(details_win, saves_list[selected_idx]);

                // refresh windows
                pnoutrefresh(pad, pad_top, 0, 3, 0, 4 + viewport_height - 2, 41);
                wnoutrefresh(details_win);
                doupdate();

                key = getch();
                switch (key) {
                        case KEY_UP:
                        case 'k':
                                if (selected_idx > 0) {
                                        selected_idx--;
                                }
                                break;

                        case KEY_DOWN:
                        case 'j':
                                if (selected_idx < (int)saves_list.size() - 1) {
                                        selected_idx++;
                                }
                                break;

                        case 'd':
                                fs::remove(saves_list[selected_idx]["_filepath"].get<std::string>());
                                saves_list.erase(selected_idx);
                                redraw_all = true;
                                selected_idx = 0;
                                minilog::fdebugc("saves", logfile, "saves_list: ", saves_list.dump());
                                pad_height = saves_list.size();
                                break;

                        case 10:
                        case KEY_ENTER:
                                if (saves_list[selected_idx]["plugins_changed"].get<bool>()) {
                                        clear();
                                        mvprintw(0, 0, "You changed plugins after you created this save.");
                                        mvprintw(1, 0,
                                                 "Press [f] to force loading this save, press [r] to try opening this "
                                                 "save in recovery.");

                                        mvprintw(
                                            3, 0,
                                            "If you choose to open in recovery, a temporary data directory will be ");
                                        mvprintw(
                                            4, 0,
                                            "created and plugins will be installed to that directory and another ");
                                        mvprintw(
                                            5, 0,
                                            "crogue instance will be started in that directory with your save loaded.");
                                        mvprintw(6, 0,
                                                 "You can update your save or complete the game, the updated save will "
                                                 "be copied to original data directory.");

                                        attron(COLOR_PAIR(4));
                                        mvprintw(8, 1,
                                                 "TIP: Always install plugins from git to increase your chance to "
                                                 "recover your saves.");
                                        attroff(COLOR_PAIR(4));
                                        refresh();

                                        bool stop = false;
                                        while (!stop) {
                                                int key = getch();
                                                switch (key) {
                                                        case 'r': {
                                                                stop = true;

                                                                redraw_all = true;

                                                                json save = saves_list[selected_idx];

                                                                recovery(
                                                                    save,
                                                                    [&]() {
                                                                            delwin(pad);
                                                                            delwin(details_win);
                                                                            pad = nullptr;
                                                                            details_win = nullptr;
                                                                            endwin();
                                                                    },
                                                                    [&]() {
                                                                            // reload ncurses things
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

                                                                            pad = newpad(pad_height, 38);
                                                                            details_win = newwin(details_height,
                                                                                                 details_width, 2, 42);

                                                                            getmaxyx(stdscr, max_y, max_x);

                                                                            details_height = max_y - 3;
                                                                            details_width = max_x - 42;

                                                                            wresize(details_win, details_height,
                                                                                    details_width);

                                                                            redraw_all = true;
                                                                    });

                                                                saves_list = get_saves();

                                                                selected_idx = 0;
                                                                pad_height = saves_list.size();

                                                                minilog::fdebugc("saves", logfile,
                                                                                 "saves_list: ", saves_list.dump());
                                                                break;
                                                        }

                                                        case 'f':
                                                                stop = true;
                                                                goto LOAD_NORMAL;
                                                                break;
                                                }
                                        }

                                        break;
                                }

                        LOAD_NORMAL:

                                try {
                                        apply_save(saves_list[selected_idx]);
                                } catch (std::runtime_error &e) {
                                        clear();
                                        attron(COLOR_PAIR(2));
                                        mvprintw(0, 0, "%s", e.what());
                                        attroff(COLOR_PAIR(2));
                                        refresh();
                                        press_enter_to_continue();
                                        redraw_all = true;
                                        break;
                                }

                                delwin(pad);
                                delwin(details_win);
                                return true;
                                break;

                        case KEY_RESIZE:
                                getmaxyx(stdscr, max_y, max_x);

                                details_height = max_y - 3;
                                details_width = max_x - 42;

                                wresize(details_win, details_height, details_width);

                                redraw_all = true;
                                break;
                }
        }

        delwin(pad);
        delwin(details_win);
        return false;
}

}  // namespace saves
