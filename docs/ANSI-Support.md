This page lists which ANSI codes `cr.curses.ansi2attr` supports
**Background colors are not supported**.
256-color palette is initialized before everything with +1 offset (see table).
If an unsupported code is given, it will return `A_NORMAL`.
The ANSI code must be given without ESC and '[' character (example: "3;31m"). 



| Code      | NCurses Style     | Description                   |
| :-:       | :--               | ---                           |
| 0m        | A_NORMAL          | Reset style                   |
| 38;5;Xm   | COLOR_PAIR(X+1)   | X foreground color (256-color)|
| 3Xm       | COLOR_PAIR(X-29)  | X foreground color (normal)   |
| 9Xm       | COLOR_PAIR(X-81)  | X foreground color (bright)   |
| 1m        | A_BOLD            | Bold                          |
| 2m        | A_DIM             | Dim                           |
| 3m        | A_ITALIC          | Italic                        |
| 4m        | A_UNDERLINE       | Underline                     |
| 5m        | A_BLINK           | Blink                         |
| 7m        | A_REVERSE         | Reverse                       |
| 8m        | A_INVIS           | Invis                         |

