#include <ncurses.h>
#include <string>

#include "minilog.hpp"

const std::string logfile = "./build/debug.log";

int main(int argc, char **argv) {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        refresh();
        minilog::fdebug(logfile, "Started");

        int key;
        while (key != 'q') {
                minilog::fdebug(logfile, "Key pressed: ", key);

                clear();

                // UI refresh
                printw("UI");

                refresh();

                // keyboard handling
                key = getch();
                switch (key) {
                        // something
                }
        }

        endwin();

        minilog::fdebug(logfile, "Exiting with status: 0");
        return 0;
}
