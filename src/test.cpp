#include "cards.hpp"
#include "minilog.hpp"
#include "tui.hpp"
#include "types.hpp"

void setup_test() {
        minilog::fdebug(logfile, "[test] setting up the test");

        // levels
        auto ef = create_level("Enterance I");
        auto es = create_level("Enterance II");

        create_biome("Enterance", 0,
                     {
                         ef,
                         es,
                     });

        auto mf = create_level("Mines I");
        auto ms = create_level("Mines II");

        create_biome("Mines", 10,
                     {
                         mf,
                         ms,
                     });

        auto zombie_buff = create_buff("Zombification", [](auto self) {
                if (self->level == 10) {
                        game::player::hp = 0;
                        log("you are now a zombie!", IMPORTANT);
                }
        });

        // cards
        create_card(5, "Zombie", ENEMY, {}, "You killed a zombie", [=]() {
                zombie_buff->level++;
                return -1;
        });

        auto poison_buff = create_buff("Poison", [](std::shared_ptr<buff_t> self) {
                game::player::hp--;
                self->level--;
        });

        create_card(3, "spider", ENEMY, {mf->id, ms->id}, "You killed a spider", [=]() {
                poison_buff->level += 3;
                return -2;
        });

        create_card(4, "healing", BASIC, {}, "You feel better", []() { return 5; });

        create_card(1, "god", ENEMY, {}, "", []() {
                log("You cant fight with a god!", IMPORTANT);
                game::player::hp = 0;
                return 0;
        });

        create_card(5, "apple", ITEM, {}, "This apple was yummy", []() { return 1; });
        create_card(1, "teleporter", ITEM, {ef->id, mf->id}, "You are teleported", []() {
                // Skip 1 card from all slots
                for (card_slot_t *slot : {&game::slot1, &game::slot2, &game::slot3}) {
                        if (!slot->front || slot->front->name == "~ Exit Gate ~") {
                                continue;
                        }

                        // Update card slot
                        slot->front = slot->back;

                        if (game::card_set.empty()) {
                                slot->back = nullptr;
                        } else {
                                slot->back = game::card_set.back();
                                game::card_set.pop_back();
                                minilog::fdebug(logfile, "new card_set size: ", game::card_set.size());
                        }
                }

                return 0;
        });

        create_card(2, "whising well", BASIC, {}, "You found a whising well", [=]() {
                std::string whish = ask_string("What you are whising? [hp / zr (zombification reset)]");
                if (whish == "hp") {
                        log("You feel better", NORMAL);
                        return 10;
                } else if (whish == "zr") {
                        log("You're starting to look more like a human.", NORMAL);
                        zombie_buff->level = 0;
                } else {
                        log("Your whish is ignored", NORMAL);
                }
                return 0;
        });

        minilog::fdebug(logfile, "[test] test is ready");
}
