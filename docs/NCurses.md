# NCurses (`cr.curses`)

To allow plugins managing TUI completely, CROGUE opens some NCurses functions for plugins.
Usage of them is not shown here. Look at NCurses documentation for them.

`stdscr` is defined as `cr.curses.stdscr`.

> [!WARNING]
> **Do not destroy TUI**
>
> Avoid using `cr.curses.clear()` to refresh game UI.
> You may remove a thing a plugin did.

## Avaible functions

The functions you expect to have is avaible under `cr.curses`. But some functions are not allowed and some functions are not added. See `src/lua_bindings.cpp` file in source code to see full list of functions.

> [!WARNING]
> `cr.curses.getyx`, `cr.curses.getbegyx`, `cr.curses.getparyx`, `cr.curses.getmaxyx`
> functions **are not used as in normal NCurses**.
> They take a window (or `cr.curses.stdscr`) and return a table with `x`, `y` values.
> Usage:
> ```lua
> local cords = cr.curses.getmaxyx(cr.curses.stdscr)
> printw("X: " .. cords.x .. " Y: " .. cords.y)
> ```
> 
> The functions must be called like that:
> - `cr.curses.getyx(win)`
> - `cr.curses.getbegyx(win)`
> - `cr.curses.getparyx(win)`
> - `cr.curses.getmaxyx(win)`

## Macros

Avaible NCurses macros:

* `cr.curses.COLS()`
* `cr.curses.LINES()`
* `cr.curses.OK`
* `cr.curses.ERR`

## `cr.curses.attr_t` and ANSI

You can use `cr.curses.attr_t` as NCurses `attr_t`.
If you hate ncurses attrs like me you can use `cr.curses.ansi2attr` function to get ncurses attrs:
```lua
local red = cr.curses.ansi2attr("38;5;1m")
cr.curses.attron(red)
cr.curses.printw("This is a red text!")
cr.curses.attroff(red)
```

See [ANSI Support][./ANSI-Support] for the full list of supported ansi codes.

## Keyboard Handling, `KEY_` variables

All of the NCurses `KEY_` variables are accessible from `cr.curses`.
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
