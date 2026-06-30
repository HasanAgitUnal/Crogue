# Safe Creting With `cr.create_` Functions

Read [Shared Types](./shared_types.md) to learn shared types.

### Functions

All this functions automatically adds objects to related containers.
There is no function for `cr.obj.card_slot`

| Function            | Return       |
| ---                 | ---          |
| `cr.create_card()`  | Shared Card  |
| `cr.create_buff()`  | Shared Buff  |
| `cr.create_level()` | Shared Level |
| `cr.create_biome()` | Shared Biome |

### Usage

All functions (except `cr.create_level`) uses table as argument. Just use type fields as keys to use functions:

> [!NOTE]
> `cr.create_level()` function takes just a string as level name.
> Generates level id itself

Normal creating:
```lua
local zombie = cr.obj.card.new()

zombie.count = 5
zombie.name = "Zombie"
zombie.type = cr.card_type.ENEMY
zombie.level_ids = {}
zombie.logmsg = "You killed a zombie"
zombie.ttl = 3
zombie.event = function()
    return -1;
end
```

With `cr.create_card()`:
```lua
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

