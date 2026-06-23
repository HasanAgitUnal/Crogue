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

#include "minilog.hpp"
#include "tui.hpp"
#include "types.hpp"

void setup_colors() {
        // 256
        for (int i = 0; i < COLORS && i < COLOR_PAIRS - 1; i++) {
                init_pair(i + 1, i, -1);
        }
}

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

std::string ask_string(std::string what) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        std::string input;
        int ch;

        curs_set(1);
        attron(COLOR_PAIR(13));
        mvprintw(max_y - 5, 0, "%s: ", what.c_str());
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

void print_line(int line) {
        move(line, 0);
        attron(COLOR_PAIR(9));
        for (int i = 0; i < COLS; ++i) {
                addstr("─");
        }
        attroff(COLOR_PAIR(9));
}

void print_type(const std::shared_ptr<card_t> card, bool bold) {
        if (card == nullptr) {
                printw(" ");
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
        printw("%s", printname);
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
        mvprintw(line + 2, start_x + 2, "LVL: %d", game::player::level);
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
