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

void print_switch(WINDOW *win, plugin_item_t &plugin) {
        bool enabled = game::settings::settings[plugin.name]["enabled"].get<bool>();
        if (enabled) {
                wattr_set(win, A_NORMAL, (int)311, NULL);
                mvwprintw(win, plugin.startline, 0, "  ");

                wattr_set(win, A_NORMAL, (int)310, NULL);
                mvwprintw(win, plugin.startline, 2, " ");

                wattr_set(win, A_NORMAL, 0, NULL);
        } else {
                wattr_set(win, A_NORMAL, (int)313, NULL);
                mvwprintw(win, plugin.startline, 0, " ");

                wattr_set(win, A_NORMAL, (int)312, NULL);
                mvwprintw(win, plugin.startline, 1, "  ");

                wattr_set(win, A_NORMAL, 0, NULL);
        }
}

void print_plugin(WINDOW *win, plugin_item_t &plugin, bool hovered) {
        wmove(win, plugin.startline, 1);

        bool enabled = game::settings::settings[plugin.name]["enabled"].get<bool>();
        print_switch(win, plugin);

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

void plugin_manager() {
        minilog::fdebugc("settings", logfile, "settings: ", game::settings::settings.dump(4));
        minilog::fdebugc("settings", logfile, "metadata: ", game::settings::metadata.dump(4));

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        // scrollable pad
        WINDOW *pad = newpad(200, max_x - 2);
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
                mvprintw(max_y - 1, 1, "Enter to enable/disable, d to delete, hjkl or arrow keys to navigate");
                refresh();

                // center
                int view_height = (max_y - 3) - 3;

                werase(pad);

                int lastend = 0;
                int index = 0;

                for (auto &[key, value] : game::settings::settings.items()) {
                        std::string name = key;
                        plugin_item_t plugin;
                        lastend = plugin2item(pad, name, lastend, plugin);
                        print_plugin(pad, plugin, index == choice ? true : false);

                        if (index == choice) {
                                if (plugin.startline < offset)
                                        offset = plugin.startline;
                                if (plugin.startline >= offset + view_height)
                                        offset = plugin.startline - view_height + 1;
                        }

                        index++;
                }

                prefresh(pad, offset, 0, 3, 1, 3 + view_height, max_x - 2);

                key = getch();
                switch (key) {
                        case KEY_RESIZE:
                                getmaxyx(stdscr, max_y, max_x);
                                wresize(pad, 200, max_x - 2);
                                break;

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
                                getmaxyx(pad, y, x);
                                wattron(pad, COLOR_PAIR(5));
                                curs_set(2);
                                mvwprintw(pad, y - 2, 0, "Are you sure [Y/n]?");
                                wattroff(pad, COLOR_PAIR(5));
                                wrefresh(pad);

                                while (true) {
                                        int key = getch();
                                        if (key == 'Y' || key == 'y') {
                                                fs::path plugin_path = get_data_dir() / "plugins" / plugin;
                                                std::error_code e;
                                                fs::remove_all(plugin_path, e);
                                                if (e) {
                                                        endwin();
                                                        minilog::fatal(1, "Can't delete directory \"", plugin,
                                                                       "\": ", e.message());
                                                }

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
                }
        }

        delwin(pad);
}

void settings() {
        clear();
        refresh();
        plugin_manager();

        // save settings
        json game_settings = json::object();

        for (auto &[key, value] : game::settings::settings.items()) {
                game_settings[key] = value["enabled"].get<bool>();
        }

        fs::path settings_path = get_data_dir() / "settings.json";
        std::ofstream settings_file(settings_path.string());
        if (!settings_file.is_open()) {
                clear();
                attron(COLOR_PAIR(4));  // yellow
                mvprintw(0, 0, "E: Can't open \"%s\". Settings are not saved!!", settings_path.string().c_str());
                attroff(COLOR_PAIR(4));  // yellow
                refresh();
                getch();
        }

        settings_file << game_settings.dump(4);
        settings_file.close();
}
