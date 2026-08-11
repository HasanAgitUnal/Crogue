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

#include <ncurses.h>
#include <charconv>
#include <minilog.hpp>
#include <string_view>
#include <vector>

#include "game.hpp"
#include "tui.hpp"

void setup_colors() {
        //// 256 ////
        for (int i = 0; i < COLORS && i < COLOR_PAIRS - 1; i++) {
                init_pair(i + 1, i, -1);
        }

        //// settings ////

        // enabled switch
        init_extended_pair(310, COLOR_WHITE, 245);
        init_extended_pair(311, COLOR_WHITE, 250);  // head

        // disabled switch
        init_extended_pair(312, COLOR_WHITE, 235);
        init_extended_pair(313, COLOR_WHITE, 240);  // head

        // hovered item
        init_extended_pair(314, COLOR_WHITE, 236);
}

void print_line(int line, WINDOW *win) {
        wmove(win, line, 0);
        wattron(win, COLOR_PAIR(9));
        int max_y, max_x;
        getmaxyx(win, max_y, max_x);
        for (int i = 0; i < max_x - 1; ++i) {
                waddstr(win, "─");
        }
        wattroff(win, COLOR_PAIR(9));
}

std::vector<std::string_view> split(std::string_view str, std::string_view delim) {
        std::vector<std::string_view> output;
        size_t first = 0;
        size_t last = str.find_first_of(delim);

        while (last != std::string_view::npos) {
                output.emplace_back(str.substr(first, last - first));
                first = last + 1;
                last = str.find_first_of(delim, first);
        }

        output.emplace_back(str.substr(first));
        return output;
}

int to_int(std::string_view sv) {
        int value = 0;
        std::from_chars(sv.data(), sv.data() + sv.size(), value);
        return value;
}

attr_t parse_ansi_color(std::string params) {
        attr_t style = A_NORMAL;
        attr_t color = A_NORMAL;
        bool started_256 = false;
        std::string_view last;

        // clean 'm'
        if (!params.empty() && params.back() == 'm') {
                params.pop_back();
        }

        for (auto code : split(params, ";")) {
                int int_code = to_int(code);

                // 256
                if (started_256) {
                        color = COLOR_PAIR(to_int(code) + 1);
                        started_256 = false;
                        last = "";
                        continue;
                }

                // 256
                if (last == "38" && code == "5") {
                        started_256 = true;
                }

                // bold
                else if (code == "1") {
                        style |= A_BOLD;
                }

                // dim
                else if (code == "2") {
                        style |= A_DIM;
                }

                // italic
                else if (code == "3") {
                        style |= A_ITALIC;
                }

                // underline
                else if (code == "4") {
                        style |= A_UNDERLINE;
                }

                // blink
                else if (code == "5") {
                        style |= A_BLINK;
                }

                // reverse
                else if (code == "7") {
                        style |= A_REVERSE;
                }

                // invis
                else if (code == "8") {
                        style |= A_INVIS;
                }

                // 3X normal
                else if (int_code >= 30 && int_code <= 37) {
                        color = COLOR_PAIR(int_code - 29);
                }

                // 9X bright
                else if (int_code >= 90 && int_code <= 97) {
                        color = COLOR_PAIR(int_code - 81);

                }

                // reset
                else if (code == "0") {
                        style = A_NORMAL;
                        color = A_NORMAL;
                }

                last = code;
        }

        // return game::lua.create_table_with("style", style, "color", color);
        return style | color;
}

void print_ansi(const std::string &str) {
        bool in_escape = false;
        std::string params = "";
        std::string buffer = "";

        for (size_t i = 0; i < str.size(); ++i) {
                unsigned char ch = (unsigned char)str[i];

                if (ch == '\033') {
                        if (!buffer.empty()) {
                                printw("%s", buffer.c_str());
                                buffer = "";
                        }
                        in_escape = true;
                        if (i + 1 < str.size() && str[i + 1] == '[')
                                i++;
                        continue;
                }

                if (in_escape) {
                        if (ch == 'm') {
                                /*
                                sol::table ansiattr = parse_ansi_color(params);
                                attrset(ansiattr.get<attr_t>("style"));
                                attron(ansiattr.get<attr_t>("color"));
                                */
                                attrset(parse_ansi_color(params));
                                params = "";
                                in_escape = false;
                        } else {
                                params += (char)ch;
                        }
                } else {
                        buffer += (char)ch;
                }
        }
        if (!buffer.empty())
                printw("%s", buffer.c_str());
        attrset(A_NORMAL);
}

int get_real_size(const std::string &line) {
        bool in_escape = false;
        int size = 0;

        for (size_t i = 0; i < line.size(); ++i) {
                if (line[i] == '\033') {
                        in_escape = true;
                        if (i + 1 < line.size() && line[i + 1] == '[')
                                i++;
                        continue;
                }
                if (in_escape) {
                        if (line[i] == 'm')
                                in_escape = false;
                } else {
                        if ((line[i] & 0xc0) != 0x80)
                                size++;
                }
        }
        return size;
}

std::string to_roman(int n) {
        if (n < 0) {
                return "-" + to_roman(-n);
        }

        struct romandata_t {
                int val;
                const char *res;
        };

        const romandata_t data[] = {{1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"},
                                    {90, "XC"},  {50, "L"},   {40, "XL"}, {10, "X"},   {9, "IX"},
                                    {5, "V"},    {4, "IV"},   {1, "I"}};
        std::string res = "";
        for (const auto &entry : data) {
                while (n >= entry.val) {
                        res += entry.res;
                        n -= entry.val;
                }
        }
        return res;
}

std::string handle_input(WINDOW *win, int y, int start_x, int box_size, std::string value, std::string allowed_chars,
                         std::function<void(void)> refresher) {

        curs_set(2);
        auto redraw_input = [&]() {
                mvwprintw(win, y, start_x, "%-*s", box_size, value.c_str());
                mvwchgat(win, y, start_x, box_size, A_NORMAL, 501, NULL);
                wmove(win, y, start_x + value.length());
                refresher();  // support for refresh(), wrefresh() and prefresh() in lazy way
        };

        int ch;
        while (ch != '\n' && ch != KEY_ENTER && ch != 10) {
                redraw_input();

                ch = getch();

                if (ch == 27)
                        break;

                if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                        if (!value.empty())
                                value.pop_back();

                } else if (value.length() < (size_t)box_size) {
                        // do not allow enter by default, but this can be overriden with allowed_chars
                        if (allowed_chars.empty() && (ch == '\n' || ch == 10 || ch == KEY_ENTER)) {
                                continue;
                        }

                        // add char if allowed
                        if (allowed_chars.empty() || allowed_chars.find((char)ch) != std::string::npos) {
                                value += (char)ch;
                        }
                }
        }
        curs_set(0);

        return value;
}

/*
 * Main Menu
 */

void show_error(std::pair<std::string, std::string> plugin) {
        auto &plugin_name = plugin.first;
        auto &error_msg = plugin.second;

        clear();
        mvprintw(0, 0, "An error occurred while loading plugin \"%s\": ", plugin_name.c_str());
        attron(COLOR_PAIR(2));  // red
        mvprintw(1, 0, "%s", error_msg.c_str());
        attroff(COLOR_PAIR(2));  // red
        press_enter_to_continue();
}

void plugin_errors() {
        if (game::plugin_errors.empty()) {
                return;
        }

        for (auto &plugin : game::plugin_errors) {
                show_error(plugin);
        }
}

void print_seed_item(int y, int max_x, bool selected) {
        int box_size = 20;
        int label_len = 5;  // "Seed:"
        int total_len = label_len + 1 + box_size;
        int start_x = (max_x - total_len) / 2;

        if (selected) {
                mvprintw(y, start_x, "Seed:");
        } else {
                attron(COLOR_PAIR(241));
                mvprintw(y, start_x, "Seed:");
                attroff(COLOR_PAIR(241));
        }

        mvaddch(y, start_x + label_len, ' ');

        mvprintw(y, start_x + label_len + 1, "%-*s", box_size, std::to_string(game::seed).c_str());
        mvchgat(y, start_x + label_len + 1, box_size, A_NORMAL, 500, NULL);
}

void print_menu(const std::vector<std::string> &menu, int choice) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        clear();

        std::vector<std::string> banner_lines;
        std::string line;
        std::stringstream ss(banner);
        while (std::getline(ss, line)) {
                if (!line.empty())
                        banner_lines.push_back(line);
        }

        int banner_h = banner_lines.size();
        int menu_h = menu.size() + (menu.size() - 1);
        int spacing = 2;  // space between banner and menu
        int total_h = banner_h + spacing + menu_h;

        int start_y = (max_y - total_h) / 2;

        // print banner
        for (int i = 0; i < banner_h; ++i) {
                int x = (max_x - get_real_size(banner_lines[i])) / 2;
                move(start_y + i, x);
                print_ansi(banner_lines[i]);
        }

        // print menu
        int menu_start_y = start_y + banner_h + spacing;
        for (int i = 0; i < (int)menu.size(); ++i) {
                int current_y = menu_start_y + (i * 2);
                if (menu[i] == "Seed:") {
                        print_seed_item(current_y, max_x, i == choice);
                } else {
                        int x = (max_x - (int)menu[i].length()) / 2;
                        if (i == choice) {
                                mvprintw(current_y, x, "%s", menu[i].c_str());
                        } else {
                                attron(COLOR_PAIR(241));
                                mvprintw(current_y, x, "%s", menu[i].c_str());
                                attroff(COLOR_PAIR(241));
                        }
                }
        }
        refresh();
}

/*
 * Game
 */

int ask(std::string what) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        curs_set(2);
        attron(COLOR_PAIR(13));
        mvprintw(max_y - 5, 0, "%s", what.c_str());
        attroff(COLOR_PAIR(13));
        int key = getch();
        curs_set(0);

        minilog::fdebugc("ask", logfile, "char: ", key);
        return key;
}

void press_enter_to_continue() {
        curs_set(2);
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(max_y - 1, 0, "Press Enter to continue... ");
        refresh();
        while (true) {
                int key = getch();

                if (key == KEY_ENTER || key == 10) {
                        break;
                }
        }

        curs_set(0);
}

std::string ask_string(std::string what) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        std::string input;
        int ch;

        curs_set(1);
        attron(COLOR_PAIR(13));
        mvprintw(max_y - 5, 0, "%s", what.c_str());
        attroff(COLOR_PAIR(13));

        while ((ch = getch()) != '\n' && ch != KEY_ENTER) {
                if (ch == 27) {
                        curs_set(0);
                        return std::string((char *)27);  // return esc

                } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                        if (!input.empty()) {
                                input.pop_back();
                                int cur_y, cur_x;
                                getyx(stdscr, cur_y, cur_x);
                                mvaddch(cur_y, cur_x - 1, ' ');
                                move(cur_y, cur_x - 1);
                        }
                } else if (isprint(ch)) {
                        input += ch;
                        addch(ch);
                }
        }

        curs_set(0);

        minilog::fdebugc("ask", logfile, "string: ", input);
        return input;
}

void print_type(const std::shared_ptr<card_t> card, bool bold) {
        if (card == nullptr) {
                printw("  ");
                return;
        }

        char c;
        attr_t attr = bold ? A_BOLD : A_DIM;
        switch (card->type) {
                case BASIC:
                case ITEM:
                        c = '+';
                        attr |= COLOR_PAIR(3);
                        break;
                case ENEMY:
                        c = '-';
                        attr |= COLOR_PAIR(2);
                        break;
                case EXIT:
                        c = '#';
                        attr |= COLOR_PAIR(5);
                        break;
        }

        attron(attr);
        printw("%c ", c);
        attroff(attr);
}

void print_slot(int line, const char c, const card_slot_t slot) {
        minilog::fdebugc("ui", logfile, "Printing slot: ", c);
        move(line, 0);
        attron(COLOR_PAIR(9));
        printw("[");
        attroff(COLOR_PAIR(9));
        attron(COLOR_PAIR(7));
        printw("%c", c);
        attroff(COLOR_PAIR(7));
        attron(COLOR_PAIR(9));
        printw("] ");
        attroff(COLOR_PAIR(9));

        print_type(slot.back, false);
        print_type(slot.front, true);

        const char *printname = slot.front != nullptr ? slot.front->name.c_str() : "---";
        printw("%s ", printname);

        attr_t lived_attr = A_NORMAL;
        if (slot._lived != 0) {
                if (slot.front->ttl == slot._lived + 1) {
                        lived_attr |= COLOR_PAIR(2);  // red
                } else {
                        lived_attr |= COLOR_PAIR(5);  // blue
                }

                attron(lived_attr);
                printw("+%d", slot._lived);
                attroff(lived_attr);
        }
}

int print_slots(int line) {
        minilog::fdebugc("ui", logfile, "Printing slots");
        print_line(line);
        attron(COLOR_PAIR(9));
        mvprintw(line, 1, "[");
        attroff(COLOR_PAIR(9));
        attron(COLOR_PAIR(7));
        printw(" Cards ");
        attroff(COLOR_PAIR(7));
        attron(COLOR_PAIR(9));
        printw("]");
        attroff(COLOR_PAIR(9));

        print_slot(line + 1, 'a', game::slot1);
        print_slot(line + 2, 'b', game::slot2);
        print_slot(line + 3, 'c', game::slot3);
        return line + 4;
}

void print_stats(int line) {
        minilog::fdebugc("ui", logfile, "Printing stats");
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        int start_x = max_x / 2;

        attron(COLOR_PAIR(9));
        mvprintw(line, start_x, "┬");
        mvprintw(line, start_x + 2, "[");
        attroff(COLOR_PAIR(9));
        attron(COLOR_PAIR(7));
        printw(" Stats ");
        attroff(COLOR_PAIR(7));
        attron(COLOR_PAIR(9));
        printw("]");

        mvprintw(line, start_x, "");
        for (int i = 1; i < 6; i++) {
                if (i == 5) {
                        mvprintw(line + i, start_x, "┴");
                        continue;
                }
                mvprintw(line + i, start_x, "│");
        }

        attroff(COLOR_PAIR(9));

        mvprintw(line + 1, start_x + 2, "HP: %d", game::player::hp);
        mvprintw(line + 2, start_x + 2, "LVL: %d (%s)", game::player::level + 1,
                 game::levels[game::player::level]->name.c_str());
}

int print_inventory(int line) {
        minilog::fdebugc("ui", logfile, "Printing inventory");

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        int col_width = max_x / 5;

        print_line(line);
        attron(COLOR_PAIR(9));
        mvprintw(line, 1, "[");
        attroff(COLOR_PAIR(9));
        attron(COLOR_PAIR(7));
        printw(" Inventory ");
        attroff(COLOR_PAIR(7));
        attron(COLOR_PAIR(9));
        printw("]");
        attroff(COLOR_PAIR(9));
        line++;

        for (int r = 0; r < 2; ++r) {               // 2 lines
                for (int c = 0; c < 5; ++c) {       // 5 cols
                        int index = (c * 2) + r;    // 0, 2, 4, 6, 8 (up) | 1, 3, 5, 7, 9 (down)
                        int x_pos = c * col_width;  // start x

                        move(line + r, x_pos);

                        attron(COLOR_PAIR(9));
                        printw("[");
                        attroff(COLOR_PAIR(9));
                        attron(COLOR_PAIR(7));
                        printw("%d", index);
                        attroff(COLOR_PAIR(7));
                        attron(COLOR_PAIR(9));
                        printw("] ");
                        attroff(COLOR_PAIR(9));

                        std::string name = "-";
                        if (index < game::player::inventory.size() && game::player::inventory[index] != nullptr) {
                                name = game::player::inventory[index]->name;
                        }

                        // cut the name
                        int available_space = col_width - 6;
                        if (available_space > 0) {
                                if (name.length() > (size_t)available_space) {
                                        name = name.substr(0, available_space - 2) + "..";
                                }
                                printw("%s", name.c_str());
                        }
                }
        }

        return line + 2;
}

int print_logs(int line) {
        minilog::fdebugc("ui", logfile, "Printing logs");
        print_line(line);
        attron(COLOR_PAIR(9));
        mvprintw(line, 1, "[");
        attroff(COLOR_PAIR(9));
        attron(COLOR_PAIR(7));
        printw(" Logs ");
        attroff(COLOR_PAIR(7));
        attron(COLOR_PAIR(9));
        printw("]");
        attroff(COLOR_PAIR(9));
        line++;

        for (int i = 0; i < game::logs.size(); i++) {
                log_type &type = game::logs[i].first;
                attr_t color = A_NORMAL;

                switch (type) {
                        case NORMAL:
                                color |= COLOR_PAIR(248);
                                break;
                        case WARN:
                                color |= COLOR_PAIR(4);
                                break;
                        case IMPORTANT:
                                color |= COLOR_PAIR(2);
                                break;
                }

                attron(color);
                mvprintw(line + i, 0, "%s", game::logs[i].second.c_str());
                attroff(color);
        }

        line += 9;

        return line + 1;  // 1 line space for ask()
}

void print_buffs(int line) {
        minilog::fdebugc("ui", logfile, "Printing buffs");

        print_line(line);
        attron(COLOR_PAIR(9));
        mvprintw(line, 1, "[");
        attroff(COLOR_PAIR(9));
        attron(COLOR_PAIR(7));
        printw(" Buffs ");
        attroff(COLOR_PAIR(7));
        attron(COLOR_PAIR(9));
        printw("]");
        attroff(COLOR_PAIR(9));
        line += 2;

        int active_idx = 0;
        int col_width = COLS / 3;

        for (auto buff : game::buffs) {
                if (buff->level == 0)
                        continue;

                int row = active_idx / 3;
                int col = active_idx % 3;
                int x = col * col_width;

                std::string roman = to_roman(buff->level);
                std::string name = buff->name;

                int reserved = 4 + (int)roman.length();
                int available = col_width - reserved;

                if (available > 0 && (int)name.length() > available) {
                        name = name.substr(0, (available > 3 ? available - 3 : available)) + "...";
                }

                attron(COLOR_PAIR(7));
                mvprintw(line + row, x, "◆ ");
                attroff(COLOR_PAIR(7));
                printw("%s ", name.c_str());
                attron(COLOR_PAIR(7));
                printw("%s", roman.c_str());
                attroff(COLOR_PAIR(7));
                active_idx++;
        }
}

void print_ui() {
        minilog::fdebugc("ui", logfile, "Printing UI");

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        int slot_end = print_slots(0);
        print_buffs(slot_end + 1);
        print_logs(max_y - 16);
        print_inventory(max_y - 3);

        // after everything because of the drawing the box drawing characters
        print_stats(0);
}
