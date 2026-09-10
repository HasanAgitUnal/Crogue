CROGUE only supports POSIX platforms for now. But for future, make your plugin compatible with Windows.

> Why CROGUE is not compatible with Windows?

CROGUE is compatible with Windows at most. But it uses some special functions on Ncurses (These functions doesn't exists on PDcurses) and Ncurses doesn't supports Windows.
I need help to support PDcurses. Please fork this project and wrap some features with `#ifdef`s and write PDcurses versions for these functions, then create a pull request.

> Which Lua version I should target on my plugin?

CROGUE supports Lua 5.1-5.4. You should target Lua 5.1 for highest compatibility.

> Terminal Support?

Most of the POSIX terminals are supported.
You should target terminals supporting:

* 256-color palette
* UTF-8
* ANSI

Default windows terminals (cmd.exe, powershell) are not supported.
