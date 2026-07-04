To allow plugins managing TUI completely, CROGUE opens some basic NCurses functions for plugins.
Usage of them is not shown here. Look at NCurses documentation for them.

> [!WARNING]
> **Do not destroy TUI**
> Avoid using clear() to refresh game UI.
> You may remove a thing a plugin did.

Functions:
- `cr.curses.printw`
- `cr.curses.move`
- `cr.curses.mvprintw`
- `cr.curses.clear`
- `cr.curses.refresh`
- `cr.curses.getch`
- `cr.curses.attrset`
- `cr.curses.attron`
- `cr.curses.attroff`
- `cr.curses.curs_set`
- `cr.curses.getmaxyx`

> [!WARNING]
> `cr.curses.getmaxyx` function **is not used as in normal NCurses**.
> It returns a table with x, y values.
> Usage:
> ```lua
> local cords = cr.curses.getmaxyx()
> printw("X: " .. cords.x .. " Y: " .. cords.y)

## `cr.curses.attr_t` and ANSI

And you can use `cr.curses.attr_t` as NCurses `attr_t`.
If you hate ncurses attrs like me you can use `cr.curses.ansi2attr` function to get ncurses attrs:
```lua
local red = cr.curses.ansi2attr("38;5;1m")
cr.curses.attron(red)
cr.curses.printw("This is a red text!")
cr.curses.attroff(red)
```

See [ANSI Support][./ansi.md] for the full list of supported ansi codes.

## Keyboard Handling, `KEY_` variables

All of the NCurses `KEY_` variables are accessable from `cr.curses`.
See NCurses documentation or `ncurses.h` header on your system for full list of `KEY_` variables.
Example:
```lua
local c = cr.curses

c.printw("Press a key")
c.refresh()

local key = c.getch()
if (key == c.KEY_ENTER)
    c.printw("You pressed enter!")
end
```


