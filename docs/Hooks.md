Hooks are functions to call after some events happens. Defined with `cr.hook()` function.

Usage:
```lua
cr.hook("<event>", function(args...)

    -- some action

end)
```

## Events

### No return and no argument events

These events does not take any argument and does not return a value.

- `start`: Runned before everything and main menu.
- `reload`: Runned when plugins are reloaded.
- `always`: Always runned before UI refresh.
- `end`: Runned when game ends.
- `die`: Runned when player dies.
- `draw`: Runned after `cr.draw_cards()`.
- `level_gen`: Runned after `cr.generate_levels()`

### Has a return type or argument

These events takes arguments in varius types and may return a bool value.

##### **`key`**

Runned when a key pressed on main game loop.

No return type.
Takes an integer argument: the key pressed.

Handle the key with `string.char()` and `string.byte()` functions, and [`cr.curses.KEY_` variables](./ncurses.md). 

##### **`level_up`**

Runned when exit gate found.

No return type.
Takes an integer argument: current level index.

##### **`slot`**

Runned when a slot picked.

Has bool return type: if true, slot will be skipped for now.
Takes an integer argument: slots number (1, 2, 3)

##### **`item`**

Runned when user wants to use an item.

Has bool return type: if true, using item is canceled.
Takes an shared card argument: the item used.

##### **`card_event`**

Runned when `cr.basic_card_event` called.

Has bool return type: if true, card event is canceled.
Takes following arguments:
  * Shared card : the card
  * integer : extra damage value

## Examples

```lua
-- show a log
cr.hook("start", function()
    cr.log("This log is added before everything")
end)

-- handle keyboard
cr.hook("key", function(key)
    if key == 27 then
        cr.log("pressed ESC")
    elseif key == cr.curses.BACKSPACE then
        cr.log("pressed backspace")
    elseif key == string.byte("a") then
        cr.log("pressed a")
    end
end)

-- disable cr.stat.slot1
cr.hook("slot", function(slot_id)
    if slot_id == 3 then
        return true
    end

    return false
end)
```


