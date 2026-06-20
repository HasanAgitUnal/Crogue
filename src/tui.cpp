#include <ncurses.h>

#include "tui.hpp"
#include "types.hpp"

int ask(std::string what) {
        curs_set(2);
        mvprintw(0, 0, "%s", what.c_str());
        int key = getch();
        curs_set(0);
        return key;
}

void print_line(int line) {
        move(line, 0);
        for (int i = 0; i < COLS; ++i) {
                addstr("─");
        }
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
        printw("[");
        attron(COLOR_PAIR(5));
        printw("%c", c);
        attroff(COLOR_PAIR(5));
        printw("] ");

        print_type(slot.back, false);
        print_type(slot.front, true);

        const char *printname = slot.front != nullptr ? slot.front->name.c_str() : "---";
        printw("%s", printname);
}

int print_slots(int line) {
        print_line(line);
        mvprintw(line, 1, "[");
        attron(COLOR_PAIR(5));
        printw(" Cards ");
        attroff(COLOR_PAIR(5));
        printw("]");

        print_slot(line + 1, 'a', game::slot1);
        print_slot(line + 2, 'b', game::slot2);
        print_slot(line + 3, 'c', game::slot3);
        return line + 4;
}

void print_stats(int line) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        int start_x = max_x / 2;

        mvprintw(line, start_x, "┬");
        mvprintw(line, start_x + 2, "[");
        attron(COLOR_PAIR(5));
        printw(" Stats ");
        attroff(COLOR_PAIR(5));
        printw("]");

        mvprintw(line, start_x, "");
        for (int i = 1; i < 6; i++) {
                if (i == 5) {
                        mvprintw(line + i, start_x, "┴");
                        continue;
                }
                mvprintw(line + i, start_x, "│");
        }

        mvprintw(line + 1, start_x + 2, "HP: %d", game::player::hp);
        mvprintw(line + 2, start_x + 2, "LVL: %d", game::player::level);
}

int print_inventory(int line) {
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);
        int col_width = max_x / 5;

        print_line(line);
        mvprintw(line, 1, "[");
        attron(COLOR_PAIR(5));
        printw(" Inventory ");
        attroff(COLOR_PAIR(5));
        printw("]");
        line++;

        for (int r = 0; r < 2; ++r) {               // 2 lines
                for (int c = 0; c < 5; ++c) {       // 5 cols
                        int index = (c * 2) + r;    // 0, 2, 4, 6, 8 (up) | 1, 3, 5, 7, 9 (down)
                        int x_pos = c * col_width;  // start x

                        move(line + r, x_pos);

                        printw("[");
                        attron(COLOR_PAIR(5));
                        printw("%d", index);
                        attroff(COLOR_PAIR(5));
                        printw("] ");

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

void print_ui() {
        int slot_end = print_slots(0);
        int inventory_end = print_inventory(slot_end + 1);

        // after inventory because of the drawing the box drawing characters
        print_stats(0);

        mvprintw(inventory_end + 1, 0, "%s", game::message.c_str());
        game::message = "";
}
