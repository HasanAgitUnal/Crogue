Game functions are used to manage game status. And used by game. But also plugins can use them.

#### `cr.is_game_running()`

Returns `true` if game started. Usefull if you have a async job does something and you want to do something while game is running without using hooks.

#### `cr.reset_game(full)`

Resets: 
* `cr.player` variables (hp=100, level=0, clear inventory)
* current level id to 0
* `cr.stat.card_set`
* `cr.stat.logs`
* Every `level` of all buffs inside `cr.stat.buffs` to 0
* `cr.stat.slot*`'s `back` and `front` fields to nil
* `cr.stat.levels`

Takes one required bool parameter `full`. If `full` is `true` resets the things below too:
* `cr.stat.biomes`
* `cr.stat.deck`
* `cr.stat.buffs`
* All [hooks](./Hooks) of plugins

> [!WARNING]
> **Avoid using cr.reset_game(full)**
>
> This parameter says function to reset everything created from plugins. If you use this, you will destroy everything done before. If you want to remove just one thing from these variables just delete them manualy using `:erase()` method of the containers. Bu you can't delete hooks.

#### `cr.basic_card_event(card, extra)`

1. Runs `card_event` hooks. If one of hooks returns `true`, returns.
2. Calls event of the card and applies extra damage with result of event

Takes 2 required arguments:

* `card`: ([Shared Card](./Shared-Types.md)) The card.
* `extra`: (integer) Extra damage

#### `cr.card_event(card, extra)`

Runs a action depending on card type:
* `cr.card_type.BASIC` or `cr.card_type.EXIT` or `cr.card_type.ENEMY`:
  Runs `cr.basic_card_event` for the card.

* `cr.card_type.ITEM`:
  Adds card to inventory.

Takes 2 required arguments:

* `card`: ([Shared Card](./Shared-Types.md)) The card.
* `extra`: (integer) Extra damage, passed to `cr.basic_card_event` if called.

#### `cr.handle_slot(slot)`

The thing when user picks a slot.

Takes a required `cr.obj.card_slot` parameter, the slot picked.

#### `cr.handle_buffs()`

Calls event of the buffs if `level` of the buff is not 0.

#### `cr.generate_levels()`

Generates `cr.stat.levels` from `cr.stat.biomes`.
**Only run if `cr.stat.levels` is empty. If its not empty, function will does not clears it.**

#### `cr.draw_cards()`

Generates `cr.stat.cards` randomly from `cr.stat.deck` using game seed and current level.
**Only run if `cr.stat.cards` is empty. If its not empty, function will does not clears it.**

#### `cr.draw_slots()`

Fills `cr.stat.slot*` 's `back` and `front`. 
**Removes existing cards inside slots.**

