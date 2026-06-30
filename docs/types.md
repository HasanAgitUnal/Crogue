# Variable Types (`cr.obj`)

Variable types you will work with listed here.

> [!WARNING]
>
> Creating this types manually is not recommended and needs `cr.shared` functions to use.
> Use `cr.create_` functions instead of them.

## Enums

Under `cr` we have 2 enums:

### `cr.card_type`


Used to identify card behavior and symbol in ui slots.

| Type    | Symbol in UI | Description                                                                      |
| :-:     | :-:          | ---                                                                              |
| `BASIC` | `+`          | Its event will be called normally. Card should have a **positive** effect.           |
| `ITEM`  | `+`          | It will go to inventory first. Its event will be called when used from inventory |
| `ENEMY` | `-`          | Its event will be called normally. Card should have a **negative** effect.           |
| `EXIT`  | `#`          | Exit card's type. Mostly you dont need it.                                       |


### `cr.log_type`


Used to set urgency of the log with `cr.log` function and `cr.stat.logs` table.
Urgency is color on the UI.

| Type        | Color  |
| :-:         | :-:    |
| `NORMAL`    | White  |
| `WARN`      | Yellow |
| `IMPORTANT` | Red    |

Example usage:

```lua
cr.log("Hello World", cr.log_type.NORMAL)
cr.log("Thats Important!!", cr.log_type.IMPORTANT)
```

## Types

All types can be created with `cr.obj.<type name>.new()` function or [`cr.create_` functions](./create_functions.md)
Always use `cr.create_` functions.

### Card Value `cr.obj.card`

`cr.obj.card` is the main variable type. It has this fields:

| Field       | Type                | Description                                                                               |
| :-:         | :-:                 | ---                                                                                       |
| `count`     | integer             | Count of the card in deck                                                                 |
| `name`      | string              | Card name                                                                                 |
| `type`      | `cr.card_type`      | Card type                                                                                 |
| `level_ids` | table with integers | A table contains level ids will card show up. If empty, card will show up in every level. |
| `logmsg`    | string              | The log will appear after card used. Leave empty to disable.                              |
| `ttl`       | integer             | Time-to-live for card. See below to understand. 0 to disable.                             | 
| `event`     | function            | Card event to call. See below for details                                                 | 


###### Time-to-live

`cr.obj.card_slot_t` has `_lived` field. If `_lived` value equals `ttl` value of the card at front of the slot, card event will be called.
Best practice is using `ttl` **ALWAYS on negative** effecting cards.

###### Card Events

You can do anything you want in card event. But you must return an number.
The number you return will be added to player health (`cr.player.hp`). Use negative values to hit damage.


Example creating a card with `cr.obj.card.new()`:
```lua
local zombie = cr.obj.card.new()

zombie.count = 5
zombie.name = "Zombie"
zombie.type = cr.card_type.ENEMY
zombie.level_ids = {}  -- Zombie will appear on all levels!
zombie.logmsg = "You killed a zombie"
zombie.ttl = 3
zombie.event = function()
    return -1;  -- Hit for 1 hp
end
```
You can also use `.new()` for other types but `cr.create_` functions are better.


Example with `cr.create_card`:
```lua
-- cr.create_ functions are:
-- safe (automatically returns cr.shared.card),
-- readable,
-- and automatically adds to cr.stat.deck
local zombie = cr.create_card({
    count = 5,
    name = "Zombie",
    type = cr.card_type.ENEMY,
    level_ids = {},
    logmsg = "You killed a zombie",
    ttl = 3,
    event = function()
        return -1;
    end
})
```

Also read [Safe Creting With `cr.create_` Functions](./create_functions.md).


### Buffs (`cr.obj.buff`)

Buffs are things like poison, zombification etc.


| Field     | Type      | Description                                                   |
| :-:       | :-:       | ---                                                           |
| `name`    | string    | Buff name.                                                    |
| `event`   | function  | Buff event (see below)                                        |
| `level`   | integer   | Buff level. Bigger is stronger effect. If 0 buff is cleared   | 

###### Buff events

A buff event gets value `self` (A `cr.shared.buff` object points the buff called).
You can do everything with a buff event.

Examples for buffs:
```lua
-- Everly turn decreases player health and its level by 1
local poison_buff = cr.create_buff({
    name = "Poison",
    event = function(self)
        cr.player.set_hp(cr.player.get_hp() - 1)
        self.level = self.level - 1
    end
})

-- Kills player if player kills 10 zombie
local zombie_buff = cr.create_buff({
    name = "Zombification",
    event = function(self)
        if self.level == 10 then
            cr.player.set_hp(0)
            cr.log("You are now a zombie!", cr.log_type.IMPORTANT)
        end
    end
})

-- Creating a zombie card integrated with zombification
local zombie = cr.create_card({
    count = 5,
    name = "Zombie",
    type = cr.card_type.ENEMY,
    level_ids = {},
    logmsg = "You killed a zombie",
    ttl = 3,
    event = function()
        -- When a zombie is killed zombification level will increase 1
        zombie_buff.level = zombie_buff.level + 1
        return -1;
    end
})
```

### Card Slots (`cr.obj.card_slot`)

Type of `cr.stat.slot1`, `cr.stat.slot2` and `cr.stat.slot3`.
You should not create this type of variable. But you can work with existing variables.


| Field     | Type          | Description               |
| :-:       | :-:           | ---                       |
| `back`    | Shared Card   | Card at the back of slot  |
| `front`   | Shared Card   | Card at the front of slot |
| `_lived`  | integer       | Look above (Time-to-live) |

Look [Shared Types](./shared_types.md) to learn what is `cr.shared`.

Example of changing name of the card at front of slot1:

```lua
cr.stat.slot1.front.name = "Fake name!"
```

### Levels (`cr.obj.level`)

Always use roman numbers in name instead of the normal numbers.

| Field     | Type      | Description       |
| :-:       | :-:       | ---               |
| `name`    | string    | Name of the level |
| `id`      | integer   | ID of level.      |


Example of creating card for specific level:

```lua
-- Create level with cr.create_level function to generate a valid levelid
local mine1 = cr.create_level("Mine I")

-- Our zombie example again
local zombie = cr.create_card({
    count = 5,
    name = "Zombie",
    type = cr.card_type.ENEMY,
    level_ids = {mine1.id},
    logmsg = "You killed a zombie",
    ttl = 3,
    event = function()
        return -1;
    end
})
```

### Biomes (`cr.obj.biome`)

Biomes groups levels with difficulty.

| Field         | Type                      | Description                               |
| :-:           | :-:                       | ---                                       |
| `difficulty`  | integer                   | Difficulty of the level in range 0-100    |
| `levels`      | table with Shared Levels  | Levels the biome contains                 |


Creating a biome:
```lua
-- We have too many levels
local mine1 = cr.create_level("Mine I")
local mine2 = cr.create_level("Mine II")
local mine3 = cr.create_level("Mine III")
local mine4 = cr.create_level("Mine IV")

-- Make them biome
local biome = cr.create_biome({
    difficulty = 30,
    levels = { mine1, mine2, mine3, mine4 }
})
```
