# Game Status & Containers

Everything about game status accesed with `cr.stat` table.
You can change or read values.

## Variables with getters & setters

Some variables accesed with getters and setters.

### `cr.stat.get_levelid()` & `cr.stat.set_levelid()`

You know level IDs from [Types](./types.md). The level ID at `cr.stat` contains the level ID of the current level.

> [!WARNING]
> Wrong usage of `cr.stat.set_levelid()` may fuck up game!
> Actualy level ID just used to generate `cr.stat.card_set`. But if one plugin needs level ID, your plugin will fuck this plugin.

Examples:
```lua
local current_id = cr.stat.get_levelid()

cr.stat.set_levelid(5) -- fuck the game!!
```

### `cr.stat.get_seed()` & `cr.stat.set_seed()`

This is the game seed. Its a string contains numbers. 
Why its a string? -> Because Lua doesnt support unsigned 64 integers.
You need a helper function to use seed in randomizing.

> [!WARNING]
> Like level ID, you should use `cr.stat.set_seed()` carefully

Examples:
```lua
-- This function may help to get seed as integer
function get_usable_seed()
    local hash = 0
    local s = cr.stat.get_seed()
    for i = 1, #s do
        hash = (hash * 37 + string.byte(s, i)) % 9007199254740992
    end
    return hash
end

math.randomseed(get_usable_seed())

-- getting a random number
local randomnumber = math.random(1, 100)
cr.log("randomnumber: " .. tostring(randomnumber), cr.log_type.NORMAL)
```

## Containers

> [!INFO]
>
> **Container methods**
>
> To see full list of methods of containers go to [Container Operations](https://sol2.readthedocs.io/en/v2.20.6/containers.html#container-operations) on sol2 documentation

### `cr.stat.deck`

A table contains [shared](./shared_types.md) cards. Used to generate `cr.stat.card_set` content. When `cr.create_card()` is used, new cards will be added to this container.

##### `cr.stat.card_set`

A table contains shared cards. Contains duplicate cards. When a card in slot used, new card will came from this container.

### `cr.stat.slot1`, `cr.stat.slot2` and `cr.stat.slot3`

Slots on the UI in `cr.obj.card_slot` type.

### `cr.stat.biomes`

A table contains shared biomes. Biome registry of the game. Used once to generate `cr.stat.levels`. When `cr.create_biome()` is used new biomes will be added to this container.

### `cr.stat.levels`

A table contains shared levels. Ordered levels of the game. After ordering biomes by their difficulty, their levels will go to this container.
Current index of the level stored in `cr.player.level`. You can access current level with this value:
```lua
local current_level = cr.stat.levels[cr.player.level]
cr.log("Current level name: " .. current_level.name, cr.log_type.NORMAL)
```

### `cr.stat.buffs`

A table contains shared levels. When `cr.create_buff()` is used new buffs will go to this container.
Every turn game checks this container. If one of the buffs level is not 0, its event will be called.

### `cr.stat.logs`

A complex table contains the latest 9 logs. When you use `cr.log()` function new log will be put here.

Struture of this table:
```txt
cr.stat.logs
├── <index number> 
│    ├── first         // log type (cr.log_type)
│    └── second        // log msg (string)
...
```

> [!WARNING]
> **Never try :add() logs**
> At the C++ side log pairs not binded perfectly.
> You can edit existing pairs but can't use :add() method on it.

Example reads last log's name:
```lua
local logname = cr.stat.logs[1].second
```

