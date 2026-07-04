Scenes are very usefull. You can make a game inside a game with this feature. Scenes have their own ui refresh and keyboard handler function. You may use that for a minigame, boss fight, puzzle or something like that.

Scenes are very simple for their job. Let's look `cr.obj.scene`'s fields again:


| Field                 | Type                      | Description                               |
| :-:                   | :-:                       | ---                                       |
| `exit_key`            | `integer`                 | Exit key. (default: `string.byte('q')')   |
| `ui_refresh()`        | Function                  | Functions to refresh UI.                  |
| `key_handler(key)`    | Function takes 1 integer  | Function to handle keyboard               |
| `run()`               | Function                  | Run method                                |

`ui_refresh()` and `run()` does not take an argument or return a value.
`key_handler(key)` gets an integer containing a key number and returns a bool value (is scene finished?). Use `string.byte(char)` to convert char to integer, and `string.char(key)` to convert key to character.
You can use `cr.curses.KEY_` constants in `key_handler`. 
When user presses the `exit_key` scene ends and exits. 0 to disable.


And look at the `cr.create_scene` example:

```lua
local last_key = 0;
local scene = cr.create_scene({
    ui_refresh = function()
        cr.curses.clear();
        if last_key == 0 then
            cr.curses.mvprintw(0, 0, "Press a key")
        else
            cr.curses.mvprintw(0, 0, "You pressed: " .. string.char(last_key))
        end
        cr.curses.refresh()
    end,

    key_handler = function(key)
        if key == string.byte('r') then
            -- Make things red
            cr.curses.attron(cr.curses.ansi2attr("31m"))

        elseif key == string.byte('R') then
            -- Disable red
            cr.curses.attroff(cr.curses.ansi2attr("31m"))

        elseif key == 27 then
            -- ESC pressed! Make italic!
            cr.curses.attron(cr.curses.ansi2attr("3m"))

        elseif key == cr.curses.KEY_ENTER then
            -- enter pressed. disable italic
            cr.curses.attroff(cr.curses.ansi2attr("3m"))
 
        elseif key == cr.curses.KEY_BACKSPACE then
            -- backspace pressed . that is an alternative way to finish scene
            return true;
        end

        last_key = key
        return false; -- scene not finished
    end
})

-- After creating scene run it:
scene:run()
```


