# Shared Types

Read [Types](./types.md) first.

Normal types are just normal variables. You can't put them inside [`cr.stat`](./game-status) containers like `cr.stat.deck`.
First you should convert them to shared versions.

When you do a change shared object variable it will be synced to the original variable.
Example:

```lua
-- You created zombie with cr.create_card function which returns shared card object
-- cr.create_card automaticaly added zombie to the cr.stat.deck container
local zombie = cr.create_card(...)

-- You changed name and ttl
zombie.name = "New Zombie"
zombie.ttl = 10
-- Now your changes applied automaticaly to cr.stat.deck !!
```

If you dont want this you should first create card manualy with `cr.obj.card.new()`, then create another variable with `cr.create_card()` function. Now every time you want to refresh card you should update the shared version's fields with normal versions field.

### Why?

Its about the **pointers** in C++.
CROGUE uses `std::shared_ptr`s to store game status.
All `cr.shared` and `cr.create_` functions returns shared_ptr's.
And all containers you will use are stores shared_ptrs.

### Converting

Use related functions for their type:

| Type               | Function                |
| ---                | ---                     |
| `cr.obj.card`      | `cr.shared.card()`      |
| `cr.obj.card_slot` | `cr.shared.card_slot()` |
| `cr.obj.buff`      | `cr.shared.buff()`      |
| `cr.obj.level`     | `cr.shared.level()`     |
| `cr.obj.biome`     | `cr.shared.biome()`     |


Example for `cr.obj.card`:
```lua
-- Create as shared object
local shared_zombie = cr.shared.card(cr.obj.card.new())

-- Fill zombie normaly
shared_zombie.count = 5
shared_zombie.name = "Zombie"
shared_zombie.type = cr.card_type.ENEMY
shared_zombie.level_ids = {}
shared_zombie.logmsg = "You killed a zombie"
shared_zombie.ttl = 3
shared_zombie.event = function()
    return -1;
end
```

### I'm lazy

I know you dont want to use that long syntax and add object to containers manualy.
There is shortcuts and recommended ways for them:

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

Simple and works read [Safe Creating With `cr.create_` Functions](./create_functions.md) to see full function list.

