#include <ncurses.h>

#include "cards.hpp"
#include "minilog.hpp"
#include "test.hpp"
#include "tui.hpp"

namespace scene {

void game() {
        //  TODO: remove this after adding plugin system
        setup_test();

        minilog::fdebugc("setup", logfile, "Generating levels");
        generate_levels();
        game::levelid = game::levels[0]->id;
        log("You are now at level: " + game::levels[game::player::level]->name, WARN);

        create_card(1, "~ Exit Gate ~", EXIT, {}, "", exit_gate);

        minilog::fdebugc("setup", logfile, "deck size: ", game::deck.size());
        draw_cards();

        minilog::fdebugc("setup", logfile, "card_set size: ", game::card_set.size());
        draw_slots();

        int key = 0;
        while (key != 'q') {
                handle_buffs();

                if (check_die()) {
                        break;
                }

                clear();
                print_ui();
                refresh();

                // keyboard handling
                key = getch();
                minilog::fdebugc("key", logfile, "pressed key: ", key);
                switch (key) {
                        case 'a':
                                minilog::fdebugc("game", logfile, "Picked card slot1");
                                handle_slot(game::slot1);
                                break;
                        case 'b':
                                minilog::fdebugc("game", logfile, "Picked card slot2");
                                handle_slot(game::slot2);
                                break;
                        case 'c':
                                minilog::fdebugc("game", logfile, "Picked card slot3");
                                handle_slot(game::slot3);
                                break;
                        case '0' ... '9':
                                int index = key - '0';
                                minilog::fdebugc("inventory", logfile, "User tried to use item index: ", index);
                                if (index < game::player::inventory.size()) {
                                        if (game::player::inventory[index]) {
                                                minilog::fdebugc("event", logfile, "Calling card event for item: ",
                                                                 game::player::inventory[index]->name);

                                                basic_card_event(game::player::inventory[index]);
                                                game::player::inventory[index] = nullptr;
                                        }
                                }
                                break;
                }
        }
}

}  // namespace scene
