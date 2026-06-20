#include <ncurses.h>

#include "tui.hpp"
#include "types.hpp"

void setup_colors() {
        // normal 1-8
        init_pair(1, COLOR_RED, -1);
        init_pair(2, COLOR_GREEN, -1);
        init_pair(3, COLOR_YELLOW, -1);
        init_pair(4, COLOR_BLUE, -1);
        init_pair(5, COLOR_MAGENTA, -1);
        init_pair(6, COLOR_CYAN, -1);
        init_pair(7, COLOR_WHITE, -1);
        init_pair(8, COLOR_BLACK, -1);

        // bright 9-16
        init_pair(9, COLOR_RED + 8, -1);
        init_pair(10, COLOR_GREEN + 8, -1);
        init_pair(11, COLOR_YELLOW + 8, -1);
        init_pair(12, COLOR_BLUE + 8, -1);
        init_pair(13, COLOR_MAGENTA + 8, -1);
        init_pair(14, COLOR_CYAN + 8, -1);
        init_pair(15, COLOR_WHITE + 8, -1);
        init_pair(16, COLOR_BLACK + 8, -1);
}

int ask(std::string what) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        curs_set(2);
        attron(COLOR_PAIR(12));
        mvprintw(max_y - 5, 0, "%s", what.c_str());
        attroff(COLOR_PAIR(12));
        int key = getch();
        curs_set(0);
        return key;
}

void print_line(int line) {
        move(line, 0);
        attron(COLOR_PAIR(16));
        for (int i = 0; i < COLS; ++i) {
                addstr("─");
        }
        attroff(COLOR_PAIR(16));
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
                        attr |= COLOR_PAIR(2);
                        break;
                case ENEMY:
                        c = '-';
                        attr |= COLOR_PAIR(1);
                        break;
                case EXIT:
                        c = '#';
                        attr |= COLOR_PAIR(4);
                        break;
        }

        attron(attr);
        printw("%c ", c);
        attroff(attr);
}

void print_slot(int line, const char c, const card_slot_t slot) {
        move(line, 0);
        attron(COLOR_PAIR(16));
        printw("[");
        attroff(COLOR_PAIR(16));
        attron(COLOR_PAIR(6));
        printw("%c", c);
        attroff(COLOR_PAIR(6));
        attron(COLOR_PAIR(16));
        printw("] ");
        attroff(COLOR_PAIR(16));

        print_type(slot.back, false);
        print_type(slot.front, true);

        const char *printname = slot.front != nullptr ? slot.front->name.c_str() : "---";
        printw("%s", printname);
}

int print_slots(int line) {
        print_line(line);
        attron(COLOR_PAIR(16));
        mvprintw(line, 1, "[");
        attroff(COLOR_PAIR(16));
        attron(COLOR_PAIR(6));
        printw(" Cards ");
        attroff(COLOR_PAIR(6));
        attron(COLOR_PAIR(16));
        printw("]");
        attroff(COLOR_PAIR(16));

        print_slot(line + 1, 'a', game::slot1);
        print_slot(line + 2, 'b', game::slot2);
        print_slot(line + 3, 'c', game::slot3);
        return line + 4;
}

void print_stats(int line) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        int start_x = max_x / 2;

        attron(COLOR_PAIR(16));
        mvprintw(line, start_x, "┬");
        mvprintw(line, start_x + 2, "[");
        attroff(COLOR_PAIR(16));
        attron(COLOR_PAIR(6));
        printw(" Stats ");
        attroff(COLOR_PAIR(6));
        attron(COLOR_PAIR(16));
        printw("]");

        mvprintw(line, start_x, "");
        for (int i = 1; i < 6; i++) {
                if (i == 5) {
                        mvprintw(line + i, start_x, "┴");
                        continue;
                }
                mvprintw(line + i, start_x, "│");
        }

        attroff(COLOR_PAIR(16));

        mvprintw(line + 1, start_x + 2, "HP: %d", game::player::hp);
        mvprintw(line + 2, start_x + 2, "LVL: %d", game::player::level);
}

int print_inventory(int line) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        int col_width = max_x / 5;

        print_line(line);
        attron(COLOR_PAIR(16));
        mvprintw(line, 1, "[");
        attroff(COLOR_PAIR(16));
        attron(COLOR_PAIR(6));
        printw(" Inventory ");
        attroff(COLOR_PAIR(6));
        attron(COLOR_PAIR(16));
        printw("]");
        attroff(COLOR_PAIR(16));
        line++;

        for (int r = 0; r < 2; ++r) {               // 2 lines
                for (int c = 0; c < 5; ++c) {       // 5 cols
                        int index = (c * 2) + r;    // 0, 2, 4, 6, 8 (up) | 1, 3, 5, 7, 9 (down)
                        int x_pos = c * col_width;  // start x

                        move(line + r, x_pos);

                        attron(COLOR_PAIR(16));
                        printw("[");
                        attroff(COLOR_PAIR(16));
                        attron(COLOR_PAIR(6));
                        printw("%d", index);
                        attroff(COLOR_PAIR(6));
                        attron(COLOR_PAIR(16));
                        printw("] ");
                        attroff(COLOR_PAIR(16));

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
        print_line(line);
        attron(COLOR_PAIR(16));
        mvprintw(line, 1, "[");
        attroff(COLOR_PAIR(16));
        attron(COLOR_PAIR(6));
        printw(" Logs ");
        attroff(COLOR_PAIR(6));
        attron(COLOR_PAIR(16));
        printw("]");
        attroff(COLOR_PAIR(16));
        line++;

        for (int i = 0; i < game::logs.size(); i++) {
                log_type &type = game::logs[i].first;
                attr_t color A_NORMAL;

                if (i == game::logs.size() - 1) {
                        color |= A_BOLD;
                }

                switch (type) {
                        case NORMAL:
                                color |= COLOR_PAIR(16);
                                break;
                        case WARN:
                                color |= COLOR_PAIR(3);
                                break;
                        case IMPORTANT:
                                color |= COLOR_PAIR(1);
                                break;
                }

                attron(color);
                mvprintw(line + i, 0, "%s", game::logs[i].second.c_str());
                attroff(color);
        }

        line += 5;

        return line + 1;  // 1 line space for ask()
}

void print_ui() {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        int slot_end = print_slots(0);
        print_logs(max_y - 12);
        print_inventory(max_y - 3);

        // after everything because of the drawing the box drawing characters
        print_stats(0);
}
