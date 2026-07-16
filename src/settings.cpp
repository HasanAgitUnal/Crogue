#include <ncurses.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "game.hpp"
#include "minilog.hpp"
#include "tui.hpp"

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

std::vector<std::string> wrap_text(std::string text, int width) {

        std::vector<std::string> lines;
        std::stringstream ss(text);
        std::string word, line;
        while (ss >> word) {
                if (line.length() + word.length() + 1 > (size_t)width) {
                        lines.push_back(line);
                        line = word;
                } else {
                        if (!line.empty())
                                line += " ";
                        line += word;
                }
        }
        if (!line.empty())
                lines.push_back(line);
        return lines;
}

void print_switch(WINDOW *win, int line, bool enabled) {
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

void print_plugin(WINDOW *win, plugin_item_t &plugin, bool hovered) {
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

// function used to edit settings
void toggle_setting(WINDOW *win, settings_item_t &setting, std::function<void(void)> refresher) {
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
                // empty for now
        }
}

void print_setting(WINDOW *win, settings_item_t &setting, bool hovered) {
        minilog::fdebugc("settings", logfile, setting.data.dump(4));
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
                std::string value = current_value.get<std::string>();
                // empty for now
        }
}

int plugin2item(WINDOW *win, std::string &pluginname, int start, plugin_item_t &plugin) {
        int max_y, max_x;
        getmaxyx(win, max_y, max_x);

        plugin.desc = wrap_text(game::settings::metadata[pluginname]["description"], max_x);
        plugin.title = game::settings::metadata[pluginname]["name"];
        plugin.name = pluginname;
        plugin.startline = start;

        // end line
        return start + plugin.desc.size() + 2;
}

int setting2item(WINDOW *win, std::string plugin, int start, json &data, settings_item_t &setting) {
        int max_y, max_x;
        getmaxyx(win, max_y, max_x);

        setting.plugin_name = plugin;
        setting.title = data["name"];
        setting.desc = wrap_text(data["description"].get<std::string>(), max_x);
        setting.option = data["option"];
        setting.startline = start;
        setting.data = data;

        // end line
        return start + setting.desc.size() + 2;
}

void plugin_settings(std::string pluginname) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        // scrollable pad
        WINDOW *settings_pad = newpad(500, max_x - 2);

        int offset = 0;
        auto refresher = [&]() { prefresh(settings_pad, offset, 0, 0, 0, max_y - 1, max_x - 2); };

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
                int view_height = max_y - 1;
                werase(settings_pad);

                for (int i = 0; i < itemsc; i++) {
                        print_setting(settings_pad, items[i], i == choice);

                        if (i == choice) {
                                if (items[i].startline < offset)
                                        offset = items[i].startline;
                                if (items[i].startline >= offset + view_height - 2)
                                        offset = items[i].startline - view_height + 3;
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

                        case 10:
                        case KEY_ENTER:
                                toggle_setting(settings_pad, items[choice], refresher);
                                break;

                        case KEY_RESIZE:
                                getmaxyx(stdscr, max_y, max_x);
                                wresize(settings_pad, 200, max_x - 2);
                                clear();
                                refresh();
                                break;
                }
        }

        delwin(settings_pad);
}

void plugin_manager() {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        // scrollable pad
        WINDOW *manager_pad = newpad(500, max_x - 2);
        int offset = 0;

        std::vector<std::string> plugin_names;
        for (auto &[key, value] : game::settings::settings.items()) {
                plugin_names.push_back(key);
        }

        int itemsc = game::settings::settings.size();
        int choice = 0;
        int key;
        while (key != 'q') {
                clear();
                // top
                mvprintw(0, 1, "Manager");
                print_line(1);

                // bottom
                mvprintw(max_y - 1, 1, "[Enter] Toggle, [d] Delete, [j/k/arrows] Navigate");
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

                        case 'd': {
                                std::string plugin = plugin_names[choice];

                                int y, x;
                                getmaxyx(manager_pad, y, x);
                                wattron(manager_pad, COLOR_PAIR(5));
                                curs_set(2);
                                mvwprintw(manager_pad, y - 2, 0, "Are you sure [Y/n]?");
                                wattroff(manager_pad, COLOR_PAIR(5));
                                wrefresh(manager_pad);

                                while (true) {
                                        int key = getch();
                                        if (key == 'Y' || key == 'y') {
                                                minilog::fdebugc("settings", logfile, "Deleting plugin: ", plugin);

                                                fs::path plugin_path = get_data_dir() / "plugins" / plugin;
                                                std::error_code e;
                                                fs::remove_all(plugin_path, e);
                                                if (e) {
                                                        endwin();
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

                                        } else if (key == 'n' || key == 'N') {
                                                break;
                                        }
                                }

                                curs_set(0);
                                break;
                        }

                        case KEY_RESIZE:
                                getmaxyx(stdscr, max_y, max_x);
                                wresize(manager_pad, 200, max_x - 2);
                                break;
                }
        }

        delwin(manager_pad);
}

void settings() {
        minilog::fdebugc("settings", logfile, "settings: ", game::settings::settings.dump(4));
        minilog::fdebugc("settings", logfile, "metadata: ", game::settings::metadata.dump(4));

        clear();
        refresh();

#ifdef DEBUG
        // TEST:
        plugin_settings("vanilla");
        return;
#endif

        plugin_manager();

        // save settings
        json game_settings = json::object();

        for (auto &[key, value] : game::settings::settings.items()) {
                game_settings[key] = value["enabled"].get<bool>();
        }

        fs::path settings_path = get_data_dir() / "settings.json";
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
