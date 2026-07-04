-- Levels --
local ef = cr.create_level("Enterance I")
local es = cr.create_level("Enterance II")

cr.create_biome({
    difficulty = 0,
    levels = { ef, es }
})

local mf = cr.create_level("Mines I")
local ms = cr.create_level("Mines II")

cr.create_biome({
    difficulty = 10,
    levels = { mf, ms }
})

-- Buffs --
local zombie_buff = cr.create_buff({
    name = "Zombification",
    event = function(self)
        if self.level == 10 then
            cr.player.set_hp(0)
            cr.log("You are now a zombie!", cr.log_type.IMPORTANT)
        end
    end
})

local poison_buff = cr.create_buff({
    name = "Poison",
    event = function(self)
        cr.player.set_hp(cr.player.get_hp() - 1)
        self.level = self.level - 1
    end
})

-- Cards --
cr.create_card({
    count = 5,
    name = "Zombie",
    type = cr.card_type.ENEMY,
    level_ids = {},
    logmsg = "You killed a zombie",
    ttl = 3,
    event = function()
        zombie_buff.level = zombie_buff.level + 1
        return -1
    end
})

cr.create_card({
    count = 3,
    name = "spider",
    type = cr.card_type.ENEMY,
    level_ids = { mf.id, ms.id },
    logmsg = "You killed a spider",
    ttl = 5,
    event = function()
        poison_buff.level = poison_buff.level + 3
        return -2
    end
})

cr.create_card({
    count = 4,
    name = "healing",
    type = cr.card_type.BASIC,
    level_ids = {},
    logmsg = "You feel better",
    ttl = 0,
    event = function()
        return 5
    end
})

cr.create_card({
    count = 1,
    name = "god",
    type = cr.card_type.ENEMY,
    level_ids = {},
    logmsg = "",
    ttl = 10,
    event = function()
        cr.log("You cant fight with a god!", cr.log_type.IMPORTANT)
        cr.player.set_hp(0)
        return 0
    end
})

cr.create_card({
    count = 5,
    name = "apple",
    type = cr.card_type.ITEM,
    level_ids = {},
    logmsg = "This apple was yummy",
    ttl = 0,
    event = function()
        return 1
    end
})

cr.create_card({
    count = 1,
    name = "teleporter",
    type = cr.card_type.ITEM,
    level_ids = { ef.id, mf.id },
    logmsg = "You are teleported",
    ttl = 0,
    event = function()
        local slots = { cr.stat.slot1, cr.stat.slot2, cr.stat.slot3 }
        for _, slot in ipairs(slots) do
            if slot.front and slot.front.name ~= "~ Exit Gate ~" then
                slot.front = slot.back
                slot._lived = 0

                if #cr.stat.card_set == 0 then
                    slot.back = nil
                else
                    slot.back = cr.stat.card_set[#cr.stat.card_set]
                    cr.stat.card_set:erase(#cr.stat.card_set)
                end
            end
        end
        return 0
    end
})

cr.create_card({
    count = 2,
    name = "whising well",
    type = cr.card_type.BASIC,
    level_ids = {},
    logmsg = "You found a whising well",
    ttl = 0,
    event = function()
        local whish = cr.ask_string("What you are whising? [hp / zr (zombification reset)]")
        if whish == "hp" then
            cr.log("You feel better", cr.log_type.NORMAL)
            return 10
        elseif whish == "zr" then
            cr.log("You're starting to look more like a human.", cr.log_type.NORMAL)
            zombie_buff.level = 0
            return 0
        else
            cr.log("Your whish is ignored", cr.log_type.NORMAL)
            return 0
        end
    end
})
