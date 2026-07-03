# CROGUE

Under development.

## TODOS

### base features

- [x] main logic (pick card -> call event)
- [x] inventory
- [x] levels & biomes
- [x] custom seed from cli
- [x] logs
- [x] buffs
- [x] ask() & ask_string() function
- [x] scene based tui (main menu)
- [x] time to live for cards

### plugin system
- [x] basics
  - [x] basic card/buff/level/biome creating & logs + ask_*()
  - [x] access to all variables
  - [x] game functions in game

- [ ] updating ui from plugins
  - [x] ncurses && tui.hpp functions
  - [ ] create_custom_scene() to create a custom scene with its own printing functions and keyboard handler

- [ ] on_* events (level complete, keyboard handling in plugin etc.)

- [ ] plugin settings in main menu
- [ ] create vanilla plugins to make a playable game
