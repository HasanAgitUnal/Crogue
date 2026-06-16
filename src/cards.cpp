#include <algorithm>
#include <random>

#include "cards.hpp"
#include "types.hpp"

void create_card(const int count, const std::string &name, std::function<int()> event) {
        game::deck.push_back(std::pair{count, std::make_shared<card_t>(card_t{name, std::move(event)})});
}

void add_card(std::shared_ptr<card_t> cardptr) {
        game::card_set.push_back(cardptr);
}

void draw_cards() {
        for (auto pair : game::deck) {
                int count = pair.first;
                auto card = pair.second;

                while (count > 0) {
                        count--;
                        add_card(card);
                }
        }

        // shuffle the deck
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(game::card_set.begin(), game::card_set.end(), g);
}
