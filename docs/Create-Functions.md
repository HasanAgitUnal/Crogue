Read [Shared Types](./Shared-Types) to learn shared types.

### Functions

All these functions automatically adds objects to related containers.
There is no function for `cr.obj.card_slot`

| Function            | Return       |
| ---                 | ---          |
| `cr.create_card()`  | Shared Card  |
| `cr.create_buff()`  | Shared Buff  |
| `cr.create_level()` | Shared Level |
| `cr.create_biome()` | Shared Biome |
| `cr.create_scene()` | Shared Scene |


### Usage

All functions (except `cr.create_level`) uses table as argument. Just use type fields as keys to use functions:

> [!NOTE]
> `cr.create_level()` function takes just a string as level name.
> Generates level id itself

Normal creating and adding to deck manualy:
```lua
local zombie = cr.obj.card.new()

zombie.count = 5
zombie.name = "Zombie"
zombie.id = "my_plugin:zombie"
zombie.type = cr.card_type.ENEMY
zombie.level_ids = {}
zombie.logmsg = "You killed a zombie"
zombie.power = 1
zombie.ttl = 3
zombie.event = function()
    return -1;
end

-- add to deck
local shared_zombie = cr.shared.card(zombie)
cr.stat.deck:add(shared_zombie)
```

With `cr.create_card()`:
```lua
local zombie = cr.create_card({
    count = 5,
    name = "Zombie",
    id = "my_plugin:zombie",
    type = cr.card_type.ENEMY,
    level_ids = {},
    logmsg = "You killed a zombie",
    power = 1,
    ttl = 3,
    event = function()
        return -1;
    end
})
```

