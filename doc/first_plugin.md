# First Plugin

Take a quick look at api here:

Create a level:
```lua
local mine1 = cr.create_level("Mines I")
```

Create More and make them a biome:
```lua
local mine1 = cr.create_level("Mines I")
local mine2 = cr.create_level("Mines II")
local mine3 = cr.create_level("Mines III")
local mine4 = cr.create_level("Mines IV")

-- Create biome
cr.create_biome({
    difficulty = 30,
    levels = { mine1, mine2, mine3, mine4 }
})
```

Let's add some cards to our mines:
```lua
-- create mine levels and biome ....

-- Create a spider
cr.create_card({
    count = 3,
    name = "spider",
    -- Spider is a negative card
    type = cr.card_type.ENEMY,
    -- Put our level ids
    level_ids = { mines1.id, mines2.id, mines3.id, mines4.id },
    logmsg = "You killed a spider",
    -- Spider will see player and attack after 5 turns
    ttl = 5,
    event = function()
        -- Spider will damage 2
        return -2
    end
})
```

Spider is too simple. Add it to a poison buff:
```lua
local spider = cr.create_card({...})

-- Create poison buff
local poison_buff = cr.create_buff({
    name = "Poison",
    event = function(self)
        -- Hit player for 1 damage
        cr.player.set_hp(cr.player.get_hp() - 1)

        -- Decrease buff level for 1
        self.level = self.level - 1
    end
})

-- Update spider event
spider.event = function()
    -- Increase poison_buff level by 3 to toggle it
    poison_buff.level = poison_buff.level + 3
    return -2
end
```

But only a spider is not enough. Add zombie too.
```lua
-- Player will die after killing 10 zombies
local zombie_buff = cr.create_buff({
    name = "Zombification",
    event = function(self)
        if self.level == 10 then
            cr.player.set_hp(0)
            cr.log("You are now a zombie!", cr.log_type.IMPORTANT)
        end
    end
})

cr.create_card({
    count = 5,
    name = "Zombie",
    type = cr.card_type.ENEMY,
    level_ids = {}, -- Zombie will appear on all levels!!
    logmsg = "You killed a zombie",
    ttl = 3,
    event = function()
        zombie_buff.level = zombie_buff.level + 1
        return -1
    end
})
```

You can also add items, positive cards, and also your own custom exit gate!

