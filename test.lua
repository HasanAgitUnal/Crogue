-- Levels --
local ef = create_level("Enterance I")
local es = create_level("Enterance II")

create_biome({
    name = "Enterance",
    difficulty = 0,
    levels = { ef, es }
})

local mf = create_level("Mines I")
local ms = create_level("Mines II")

create_biome({
    name = "Mines",
    difficulty = 10,
    levels = { mf, ms }
})

-- Buffs --
local zombie_buff = create_buff({
    name = "Zombification",
    event = function(self)
        if self.level == 10 then
            crogue.player.set_hp(0)
            log("You are now a zombie!", LogType.IMPORTANT)
        end
    end
})

local poison_buff = create_buff({
    name = "Poison",
    event = function(self)
        crogue.player.set_hp(crogue.player.get_hp() - 1)
        self.level = self.level - 1
    end
})

-- Cards --
create_card({
    count = 5,
    name = "Zombie",
    type = CardType.ENEMY,
    level_ids = {},
    logmsg = "You killed a zombie",
    ttl = 3,
    event = function()
        zombie_buff.level = zombie_buff.level + 1
        return -1
    end
})

create_card({
    count = 3,
    name = "spider",
    type = CardType.ENEMY,
    level_ids = { mf.id, ms.id },
    logmsg = "You killed a spider",
    ttl = 5,
    event = function()
        poison_buff.level = poison_buff.level + 3
        return -2
    end
})

create_card({
    count = 4,
    name = "healing",
    type = CardType.BASIC,
    level_ids = {},
    logmsg = "You feel better",
    ttl = 0,
    event = function()
        return 5
    end
})

create_card({
    count = 1,
    name = "god",
    type = CardType.ENEMY,
    level_ids = {},
    logmsg = "",
    ttl = 10,
    event = function()
        log("You cant fight with a god!", LogType.IMPORTANT)
        crogue.player.set_hp(0)
        return 0
    end
})

create_card({
    count = 5,
    name = "apple",
    type = CardType.ITEM,
    level_ids = {},
    logmsg = "This apple was yummy",
    ttl = 0,
    event = function()
        return 1
    end
})

create_card({
    count = 1,
    name = "teleporter",
    type = CardType.ITEM,
    level_ids = { ef.id, mf.id },
    logmsg = "You are teleported",
    ttl = 0,
    event = function()
        local slots = { crogue.stat.slot1, crogue.stat.slot2, crogue.stat.slot3 }
        for _, slot in ipairs(slots) do
            if slot.front and slot.front.name ~= "~ Exit Gate ~" then
                slot.front = slot.back
                slot._lived = 0

                if #crogue.stat.card_set == 0 then
                    slot.back = nil
                else
                    slot.back = crogue.stat.card_set[#crogue.stat.card_set]
                    crogue.stat.card_set:erase(#crogue.stat.card_set)
                end
            end
        end
        return 0
    end
})

create_card({
    count = 2,
    name = "whising well",
    type = CardType.BASIC,
    level_ids = {},
    logmsg = "You found a whising well",
    ttl = 0,
    event = function()
        local whish = ask_string("What you are whising? [hp / zr (zombification reset)]")
        if whish == "hp" then
            log("You feel better", LogType.NORMAL)
            return 10
        elseif whish == "zr" then
            log("You're starting to look more like a human.", LogType.NORMAL)
            zombie_buff.level = 0
            return 0
        else
            log("Your whish is ignored", LogType.NORMAL)
            return 0
        end
    end
})

log("Test plugin loaded", LogType.WARN)

