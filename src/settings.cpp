#include <fstream>
#include <minilog.hpp>
#include <vector>

#include "game.hpp"
#include "tui.hpp"
#include "utils.hpp"

struct plugin_item_t {
        std::string name;
        std::string title;
        std::vector<std::string> desc;
        int startline;
};

struct settings_item_t {
        std::string plugin_name;
        std::string title;
        std::string option;
        std::vector<std::string> desc;
        int startline;
        json data;
};

static void print_switch(WINDOW *win, int line, bool enabled) {
        if (enabled) {
                wattr_set(win, A_NORMAL, (int)311, NULL);
                mvwprintw(win, line, 0, "  ");

                wattr_set(win, A_NORMAL, (int)310, NULL);
                mvwprintw(win, line, 2, " ");

                wattr_set(win, A_NORMAL, 0, NULL);
        } else {
                wattr_set(win, A_NORMAL, (int)313, NULL);
                mvwprintw(win, line, 0, " ");

                wattr_set(win, A_NORMAL, (int)312, NULL);
                mvwprintw(win, line, 1, "  ");

                wattr_set(win, A_NORMAL, 0, NULL);
        }
}

static void print_plugin(WINDOW *win, plugin_item_t &plugin, bool hovered) {
        bool enabled = game::settings::settings[plugin.name]["enabled"].get<bool>();
        print_switch(win, plugin.startline, enabled);

        if (hovered) {
                wattr_set(win, A_UNDERLINE, (int)314, NULL);
                mvwprintw(win, plugin.startline, 4, "%s", plugin.title.c_str());
                wattr_set(win, A_NORMAL, 0, NULL);
        } else {
                mvwprintw(win, plugin.startline, 4, "%s", plugin.title.c_str());
        }

        int line_n = plugin.startline + 1;
        for (std::string line : plugin.desc) {
                mvwprintw(win, line_n++, 4, "%s", line.c_str());
        }
}

// offset of plugin_settings offset value
// its here because choosable settings type needs this to draw choose menu
int settings_offset = 0;

static std::string handle_menu(WINDOW *win, std::vector<std::string> items) {
        int itemsc = items.size();
        int key;
        int choice = 0;
        int offset = 0;
        int max_y, max_x;
        getmaxyx(win, max_y, max_x);

        while (true) {
                werase(win);

                if (choice < offset)
                        offset = choice;
                if (choice >= offset + max_y)
                        offset = choice - max_y + 1;

                for (int i = 0; i < itemsc; i++) {
                        if (i >= offset && i < offset + max_y) {
                                int y_pos = i - offset;

                                mvwprintw(win, y_pos, 0, "%s", items[i].c_str());

                                mvwchgat(win, y_pos, 0, max_x, A_NORMAL, 501, NULL);

                                if (i == choice) {
                                        mvwchgat(win, y_pos, 0, max_x, A_BOLD, (int)314, NULL);
                                }
                        }
                }

                wrefresh(win);

                key = getch();
                switch (key) {
                        case 'j':
                        case KEY_DOWN:
                                choice = (choice + 1) % itemsc;
                                break;
                        case 'k':
                        case KEY_UP:
                                choice = (choice - 1 + itemsc) % itemsc;
                                break;
                        case 10:  // ENTER
                        case KEY_ENTER:
                                return items[choice];
                        case 27:  // ESC
                                return "";
                }
        }
}

// function used to edit settings
static void toggle_setting(WINDOW *win, settings_item_t &setting, std::function<void(void)> refresher) {
        json current_value = game::settings::settings[setting.plugin_name][setting.data["option"].get<std::string>()];
        if (setting.data["type"].get<std::string>() == "switch") {
                bool value = current_value.get<bool>();
                // invert value
                game::settings::settings[setting.plugin_name][setting.data["option"]] = !value;

        } else if (setting.data["type"].get<std::string>() == "input") {
                int max_y, max_x;
                getmaxyx(stdscr, max_y, max_x);

                std::string current_str = current_value.get<std::string>();

                // clang-format off

                int box_size = 6 + setting.title.length();
                std::string new_value = handle_input(win, setting.startline, box_size, max_x - box_size - 2, current_str, "", refresher);

                // clang-format on

                game::settings::settings[setting.plugin_name][setting.data["option"]] = new_value;

        } else if (setting.data["type"].get<std::string>() == "choose") {
                std::vector<std::string> options = setting.data["values"].get<std::vector<std::string>>();

                int screen_max_y, screen_max_x;
                getmaxyx(stdscr, screen_max_y, screen_max_x);

                int current_line_y = setting.startline - settings_offset;
                int menu_height = (int)options.size();

                int menu_y;
                if (current_line_y + 1 + menu_height < screen_max_y) {
                        menu_y = current_line_y + 1;
                } else {
                        menu_y = current_line_y - menu_height;
                }

                int start_x = 4 + setting.title.length() + 2;
                int box_size = screen_max_x - start_x - 4;

                // Pencereyi oluştur
                WINDOW *menu_win = newwin(menu_height, box_size, menu_y, start_x);
                keypad(menu_win, TRUE);

                // Menüyü çalıştır
                std::string selected = handle_menu(menu_win, options);

                if (!selected.empty()) {
                        game::settings::settings[setting.plugin_name][setting.data["option"]] = selected;
                }

                delwin(menu_win);
                touchwin(win);
                refresher();
        }
}

static void print_setting(WINDOW *win, settings_item_t &setting, bool hovered) {
        json current_value = game::settings::settings[setting.plugin_name][setting.data["option"].get<std::string>()];

        if (setting.data["type"].get<std::string>() == "switch") {
                bool enabled = current_value.get<bool>();
                print_switch(win, setting.startline, enabled);

                if (hovered) {
                        wattr_set(win, A_UNDERLINE, (int)314, NULL);
                        mvwprintw(win, setting.startline, 4, "%s", setting.title.c_str());
                        wattr_set(win, A_NORMAL, 0, NULL);
                } else {
                        mvwprintw(win, setting.startline, 4, "%s", setting.title.c_str());
                }

                int line_n = setting.startline + 1;
                for (std::string line : setting.desc) {
                        mvwprintw(win, line_n++, 4, "%s", line.c_str());
                }


        } else if (setting.data["type"].get<std::string>() == "input") {
                std::string val = current_value.get<std::string>();

                int max_y, max_x;
                getmaxyx(win, max_y, max_x);

                int start_x = 4 + setting.title.length() + 2;
                int box_size = max_x - start_x - 2;

                if (hovered) {
                        wattr_set(win, A_UNDERLINE, (int)314, NULL);
                        mvwprintw(win, setting.startline, 4, "%s:", setting.title.c_str());
                        wattr_set(win, A_NORMAL, 0, NULL);
                } else {
                        mvwprintw(win, setting.startline, 4, "%s:", setting.title.c_str());
                }

                mvwprintw(win, setting.startline, start_x, "%-*s", box_size, val.c_str());

                mvwchgat(win, setting.startline, start_x, box_size, A_NORMAL, 500, NULL);

                int line_n = setting.startline + 1;
                for (std::string line : setting.desc) {
                        mvwprintw(win, line_n++, 4, "%s", line.c_str());
                }

        } else if (setting.data["type"].get<std::string>() == "choose") {
                std::string val = current_value.get<std::string>();

                int max_y, max_x;
                getmaxyx(win, max_y, max_x);

                int start_x = 4 + setting.title.length() + 2;
                int box_size = max_x - start_x - 2;

                if (hovered) {
                        wattr_set(win, A_UNDERLINE, (int)314, NULL);
                        mvwprintw(win, setting.startline, 4, "%s:", setting.title.c_str());
                        wattr_set(win, A_NORMAL, 0, NULL);
                } else {
                        mvwprintw(win, setting.startline, 4, "%s:", setting.title.c_str());
                }

                mvwprintw(win, setting.startline, start_x, "%-*s", box_size, val.c_str());

                mvwchgat(win, setting.startline, start_x, box_size, A_NORMAL, (int)314, NULL);

                int line_n = setting.startline + 1;
                for (std::string line : setting.desc) {
                        mvwprintw(win, line_n++, 4, "%s", line.c_str());
                }
        }
}

static int plugin2item(WINDOW *win, std::string &pluginname, int start, plugin_item_t &plugin) {
        int max_y, max_x;
        getmaxyx(win, max_y, max_x);

        auto description = game::settings::metadata[pluginname]["description"].get<std::string>();
        plugin.desc = wrap_text(description, max_x - 4);
        plugin.title = game::settings::metadata[pluginname]["name"];
        plugin.name = pluginname;
        plugin.startline = start;

        // end line
        return start + plugin.desc.size() + 2;
}

static int setting2item(WINDOW *win, std::string plugin, int start, json &data, settings_item_t &setting) {
        int max_y, max_x;
        getmaxyx(win, max_y, max_x);

        setting.plugin_name = plugin;
        setting.title = data["name"];
        auto description = data["description"].get<std::string>();
        setting.desc = wrap_text(description, max_x - 4);
        setting.option = data["option"];
        setting.startline = start;
        setting.data = data;

        // end line
        return start + setting.desc.size() + 2;
}

static void plugin_settings(std::string pluginname) {
        clear();
        refresh();

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        int pad_height = 5;
        for (const auto &setting : game::settings::metadata[pluginname]["settings"]) {
                auto description = setting["description"].get<std::string>();
                pad_height += wrap_text(description, max_x - 6).size() + 2;
        }

        WINDOW *settings_pad = newpad(pad_height, max_x - 2);

        auto refresher = [&]() { prefresh(settings_pad, settings_offset, 0, 3, 0, max_y - 5, max_x - 2); };

        // generate UI
        std::vector<settings_item_t> items;
        int lastend = 0;

        for (auto setting : game::settings::metadata[pluginname]["settings"]) {
                settings_item_t item;
                lastend = setting2item(settings_pad, pluginname, lastend, setting, item);
                items.push_back(item);
        }

        int itemsc = items.size();
        int key = 0;
        int choice = 0;
        while (key != 'q') {
                mvprintw(0, 1, "Settings: %s", game::settings::metadata[pluginname]["name"].get<std::string>().c_str());
                print_line(1);

                mvprintw(max_y - 1, 0, "[q] Quit, [Enter] Edit, [r] Reset to default, [j/k/arrows] Navigate");
                refresh();

                int view_height = max_y - 5;
                werase(settings_pad);

                for (int i = 0; i < itemsc; i++) {
                        print_setting(settings_pad, items[i], i == choice);

                        if (i == choice) {
                                if (items[i].startline < settings_offset)
                                        settings_offset = items[i].startline;
                                if (items[i].startline >= settings_offset + view_height - 2)
                                        settings_offset = items[i].startline - view_height + 3;
                        }
                }

                refresher();

                key = getch();
                switch (key) {
                        case 'j':
                        case KEY_DOWN:
                                choice = (choice + 1) % itemsc;
                                break;

                        case 'k':
                        case KEY_UP:
                                choice = (choice - 1 + itemsc) % itemsc;
                                break;

                        case 'r':
                                // reset to default
                                game::settings::settings[pluginname][items[choice].data["option"]] =
                                    items[choice].data["default"];
                                break;

                        case 10:
                        case KEY_ENTER:
                                toggle_setting(settings_pad, items[choice], refresher);
                                break;

                        case KEY_RESIZE:
                                getmaxyx(stdscr, max_y, max_x);
                                pad_height = 5;
                                for (const auto &s : game::settings::metadata[pluginname]["settings"]) {
                                        auto d = s["description"].get<std::string>();
                                        pad_height += wrap_text(d, max_x - 6).size() + 2;
                                }

                                wresize(settings_pad, pad_height, max_x - 2);

                                items.clear();
                                lastend = 0;
                                for (auto s : game::settings::metadata[pluginname]["settings"]) {
                                        settings_item_t item;
                                        lastend = setting2item(settings_pad, pluginname, lastend, s, item);
                                        items.push_back(item);
                                }
                                settings_offset = 0;

                                clear();
                                refresh();
                                break;
                }
        }

        // save settings
        fs::path file_path = game::_data_directory / "plugins" / pluginname / "settings.json";
        std::ofstream file(file_path);

        json settings = game::settings::settings[pluginname];
        settings.erase("enabled");

        file << settings.dump(4);
        file.close();
        minilog::fdebugc("settings", logfile, "Saved plugin settings to ", file_path);

        delwin(settings_pad);

        clear();
        refresh();
}

static void plugin_manager() {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        int pad_height = 0;
        for (auto &[key, value] : game::settings::settings.items()) {
                auto description = game::settings::metadata[key]["description"].get<std::string>();
                pad_height += wrap_text(description, max_x - 6).size() + 2;
        }

        WINDOW *manager_pad = newpad(pad_height, max_x - 2);
        int offset = 0;

        std::vector<std::string> plugin_names;
        for (auto &[key, value] : game::settings::settings.items()) {
                plugin_names.push_back(key);
        }

        int itemsc = game::settings::settings.size();
        int choice = 0;
        int key;
        while (key != 'q') {
                mvprintw(0, 1, "Manager");
                print_line(1);

                mvprintw(max_y - 1, 0, "[q] Quit, [Enter] Toggle, [d] Delete, [s] Settings, [j/k/arrows] Navigate");
                refresh();

                // center
                int view_height = (max_y - 3) - 3;

                werase(manager_pad);

                int lastend = 0;
                int index = 0;

                for (auto &[key, value] : game::settings::settings.items()) {
                        std::string name = key;
                        plugin_item_t plugin;
                        lastend = plugin2item(manager_pad, name, lastend, plugin);
                        print_plugin(manager_pad, plugin, index == choice ? true : false);

                        if (index == choice) {
                                if (plugin.startline < offset)
                                        offset = plugin.startline;
                                if (plugin.startline >= offset + view_height)
                                        offset = plugin.startline - view_height + 1;
                        }

                        index++;
                }

                prefresh(manager_pad, offset, 0, 3, 1, 3 + view_height, max_x - 2);

                key = getch();
                switch (key) {
                        case 10:
                        case KEY_ENTER: {
                                bool enabled = game::settings::settings[plugin_names[choice]]["enabled"].get<bool>();
                                game::settings::settings[plugin_names[choice]]["enabled"] = !enabled;
                                break;
                        }

                        case 'j':
                        case KEY_DOWN:
                                choice = (choice + 1) % itemsc;
                                break;

                        case 'k':
                        case KEY_UP:
                                choice = (choice - 1 + itemsc) % itemsc;
                                break;

                        case 's':
                                if (game::settings::metadata[plugin_names[choice]].contains("settings")) {
                                        plugin_settings(plugin_names[choice]);
                                }
                                break;

                        case 'd': {
                                std::string plugin = plugin_names[choice];

                                // clang-format off
                                attron(COLOR_PAIR(5));
                                curs_set(2);
                                //                      [q] Quit, [Enter] Toggle, [d] Delete, [s] Settings, [j/k/arrows] Navigate
                                mvprintw(max_y - 1, 0, "Are you sure [Y/n]?                                                      ");
                                move (max_y-1, 19); // move cursor to the right position
                                attroff(COLOR_PAIR(5));
                                refresh();
                                // clang-format on

                                while (true) {
                                        int key = getch();
                                        if (key == 'Y' || key == 'y') {
                                                minilog::fdebugc("settings", logfile, "Deleting plugin: ", plugin);

                                                fs::path plugin_path = game::_data_directory / "plugins" / plugin;
                                                std::error_code e;
                                                fs::remove_all(plugin_path, e);
                                                if (e) {
                                                        end_program();
                                                        minilog::fatal(1, "Can't delete directory \"", plugin,
                                                                       "\": ", e.message());
                                                }

                                                minilog::fdebugc("settings", logfile, "Deleted plugin: ", plugin);

                                                // update plugin names
                                                plugin_names.clear();
                                                for (auto &[key, value] : game::settings::settings.items()) {
                                                        plugin_names.push_back(key);
                                                }

                                                int itemsc = game::settings::settings.size();
                                                if (choice == itemsc) {
                                                        choice--;
                                                }

                                                break;

                                        } else if (key == 'n' || key == 'N' || key == 27) {
                                                break;
                                        }
                                }

                                curs_set(0);
                                break;
                        }

                        case KEY_RESIZE:
                                getmaxyx(stdscr, max_y, max_x);
                                pad_height = 0;
                                for (auto &[k, v] : game::settings::settings.items()) {
                                        auto d = game::settings::metadata[k]["description"].get<std::string>();
                                        pad_height += wrap_text(d, max_x - 6).size() + 2;
                                }

                                wresize(manager_pad, pad_height, max_x - 2);
                                offset = 0;
                                clear();
                                refresh();
                                break;
                }
        }

        delwin(manager_pad);
}

void settings() {
        minilog::fdebugc("settings", logfile, "settings: ", game::settings::settings.dump());
        minilog::fdebugc("settings", logfile, "metadata: ", game::settings::metadata.dump());

        if (game::settings::metadata.empty()) {
                // return if no plugin loaded
                int max_y, max_x;
                getmaxyx(stdscr, max_y, max_x);
                mvprintw(max_y - 1, 0, "No plugin loaded!");
                getch();
                return;
        }

        clear();
        refresh();

        plugin_manager();

        // save settings
        json game_settings = json::object();

        for (auto &[key, value] : game::settings::settings.items()) {
                game_settings[key] = value["enabled"].get<bool>();
        }

        fs::path settings_path = game::_data_directory / "settings.json";
        minilog::fdebugc("settings", logfile, "Saving game settings to ", settings_path.string());
        std::ofstream settings_file(settings_path.string());
        if (!settings_file.is_open()) {
                clear();
                attron(COLOR_PAIR(4));  // yellow
                mvprintw(0, 0, "E: Can't open \"%s\". Settings are not saved!!", settings_path.string().c_str());
                attroff(COLOR_PAIR(4));  // yellow
                refresh();
                getch();
        } else {
                settings_file << game_settings.dump(4);
                settings_file.close();
                minilog::fdebugc("settings", logfile, "Saved game settings to ", settings_path.string());
        }
}
