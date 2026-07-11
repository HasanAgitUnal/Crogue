/*
 * Manage
 * ----------------
 *
 * ==# Vanilla
 *     Official base plugin for CROGUE
 *
 * #== Something
 *     Description for something
 *
 *
 * Enter to enable/disable, hjkl or arrow keys to navigate.
 *
 */

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

        // top
        mvprintw(0, 1, "Settings");
        print_line(1);

        WINDOW *win = newwin(max_y - 4, max_x - 2, 3, 1);

        int lastend = 0;
        for (auto &[key, value] : game::settings::settings.items()) {
                std::string name = key;
                plugin_item_t plugin;
                lastend = plugin2item(win, name, lastend, plugin);
                print_plugin(win, plugin, true);
        }

        wrefresh(win);

        // bottom
        mvprintw(max_y - 2, 1, "Enter to enable/disable, hjkl or arrow keys to navigate");

        // no keyboard handling yet, for testing ui printing without scroll
        getch();

        delwin(win);
}

void settings() {
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
