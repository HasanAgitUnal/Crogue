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

- [x] updating ui from plugins
  - [x] ncurses && tui.hpp functions
  - [x] create_scene() to create a custom scene with its own printing functions and keyboard handler

- [x] hooks (level complete, keyboard handling in plugin etc.)

- [x] plugin manager && plugin settings in main menu
  - [x] Plugin manager to enable/disable or delete plugins
  - [x] Plugin settings

### other features && improvements

- [x] installing packages from cli
- [x] saves system
- [x] custom data dir/plugins dir/saves dir from cli
- [x] recovery mode to load broken saves
- [x] add ncurses WINDOW management to cr.curses 
- [x] code quality
- [x] optimizations
- [ ] create vanilla plugins to make a playable game

### BUGS LIST

