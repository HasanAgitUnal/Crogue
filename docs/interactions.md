# Interacting With Game

Maybe you need to ask something to user or log something to ui etc.
There is functions for them.

### `cr.log`

Used to show log on UI.

Usage:
```lua
cr.log(msg, type)
```

- msg: any string
- type: `cr.log_type` (see [Types](./types.md))

Examples:
```lua
cr.log("Something feels strange", cr.log_type.NORMAL)
cr.log("You are hungry!", cr.log_type.WARN)
cr.log("You cant fight with a god!", cr.log_type.IMPORTANT)
```

### `cr.ask` & `cr.ask_string`

Used to get string or key from user.
`cr.ask` gets one key including ESC key, `cr.ask_string` gets a full string.

Example for confirmation dialog:
```lua
local key = cr.ask("Confirm [Y/n]: ")
-- some handling here...
```

Example card uses `cr.ask_string` function:
```lua
cr.create_card({
    count = 2,
    name = "wishing well",
    type = cr.card_type.BASIC,
    level_ids = {},
    logmsg = "You found a whising well",
    ttl = 0,
    event = function()
        -- In this case we get just 2 characters. But you can get any length
        local wish = cr.ask_string("What you are wishing? [hp / zr (zombification reset)]: ")
        if wish == "hp" then
            cr.log("You feel better", cr.log_type.NORMAL)
            return 10

        elseif wish == "zr" then
            cr.log("You're starting to look more like a human.", cr.log_type.NORMAL)
            zombie_buff.level = 0
            return 0

        else
            cr.log("Your wish is ignored", cr.log_type.NORMAL)
            return 0
        end
    end
})
```
