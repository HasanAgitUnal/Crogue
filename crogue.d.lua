-- ============================================
-- CROGUE Lua API Definitions For LSPs
--
-- Add this file to your project directory
-- ============================================

--- @meta

-- ============================================
-- Types For LSP
-- ============================================

--- @class _ARGS_create_card : cr.obj.card
--- @field new nil

--- @class _ARGS_create_buff : cr.obj.buff
--- @field level nil
--- @field new nil

--- @class _ARGS_create_biome : cr.obj.biome
--- @field new nil

--- @class _ARGS_create_scene : cr.obj.scene
--- @field run nil
--- @field new nil

--- @class _log
--- @field first cr.log_type                            Log type
--- @field second string                                Log message

--- @class _CURSES_window

--- @class _CURSES_attr_t : integer

--- @class _CURSES_chtype : integer

--- @class _CURSES_coords
--- @field x integer
--- @field y integer

--- A class contains sol2 container methods (https://sol2.readthedocs.io/en/v2.20.6/containers.html#container-operations)
--- @class _CONTAINER<T>
--- @field [integer] T
--- @field size fun(self: _CONTAINER<T>): integer                               Returns the number of elements in the container.
--- @field set fun(self: _CONTAINER<T>, key: integer, value: T)                 Sets a value at the given index. If value is nil, erases the element at that index. If index is size+1, inserts the value at the end.
--- @field at fun(self: _CONTAINER<T>, key: integer): T                         Returns the value at the given index. For non‑random‑access containers, this may return multiple values.
--- @field get fun(self: _CONTAINER<T>, key: integer): T                        Returns the value at the given index (same as `at`). For non‑random‑access containers, this may return multiple values.
--- @field find fun(self: _CONTAINER<T>, target: T): boolean                    Finds a value in the container. (For sequence containers, this performs a linear search.)
--- @field erase fun(self: _CONTAINER<T>, target: integer)                      Erases the element at the given index. (For sequence containers, the target is an index.)
--- @field insert fun(self: _CONTAINER<T>, target: integer, value: T)           Inserts a value at the specified index. (For sequence containers, the target is an index.)
--- @field add fun(self: _CONTAINER<T>, value: T)                               Adds a value to the end of the container.
--- @field clear fun(self: _CONTAINER<T>, )                                     Removes all elements from the container.
--- @field pairs fun(self: _CONTAINER<T>, ): fun(), _CONTAINER<T>, nil          Lua 5.2+ only; use c:pairs() in Lua 5.1/LuaJIT.
--- @field ipairs fun(self: _CONTAINER<T>, ): fun(), _CONTAINER<T>, integer     Lua 5.2+ only; use c:ipairs() in Lua 5.1/LuaJIT.

--- @alias _HOOK_EVENT "before_refresh" | "after_refresh" | "start" | "game_start" | "game_end" | "game_quit" | "reload" | "ending" | "draw" | "level_gen" | "die" | "key" | "level_up" | "slot" | "item" | "card_event" | "s_save" | "s_load" | "damage"

-- ============================================
-- Main Table
-- ============================================

--- CROGUE Plugin API
--- @class cr
---
--- @field debug boolean                Is debug build of crogue?
---
--- @field card_type cr.card_type
--- @field log_type cr.log_type
--
--- @field obj cr.obj
--
--- @field shared cr.shared
--
--- @field stat cr.stat
--- @field player cr.player
--
--- @field tui cr.tui
--
--- @field curses cr.curses
--
--- @field ask fun(what: string):integer                                Ask for a key to user
--- @field ask_string fun(what: string):string                          Ask for a text to user
--- @field log fun(msg: string, type?: cr.log_type)                     Display a log with given urgency
--
--- @field create_card fun(table: _ARGS_create_card):_SHARED_card       Creates a shared card, adds to cr.stat.deck and returns the card.
--- @field create_buff fun(table: _ARGS_create_buff):_SHARED_buff       Creates a shared buff, adds to cr.stat.buffs, and returns the biome
--- @field create_level fun(name: string):_SHARED_level                 Creates a shared level with given name, assigns an ID, and returns the level
--- @field create_biome fun(table: _ARGS_create_biome):_SHARED_biome    Creates a shared biome, adds to cr.stat.biomes, and returns the biome
--- @field create_scene fun(table: _ARGS_create_scene):cr.obj.scene     Creates a scene and returns it
--
--- @field reset_game fun(full: boolean)                                Resets game
--- @field generate_levels fun()                                        Generates cr.stat.levels from cr.stat.biomes
--- @field draw_cards fun()                                             Generates cr.stat.card_set from cr.stat.deck
--- @field draw_slots fun()                                             Draws card to all slots
--- @field handle_slot fun(slot: cr.obj.card_slot)                      Does the action when user picks a slot
--- @field handle_buffs fun()                                           Checks level of all buffs inside (cr.stat.buffs), runs their events if level ~= 0
--- @field basic_card_event fun(card: _SHARED_card, extra: integer)     Runs the card event and applies HP change and extra damage (player_hp = player_hp + event_return - extra_damage)
--- @field card_event fun(card: _SHARED_card, extra: integer)           Does action related to type of the card
--
--- @field settings fun(plugin: string):table                           Returns settings of the given plugin. see: https://github.com/HasanAgitUnal/CROGUE/wiki/Settings
--- @field get_data_dir fun():string                                    Returns data directory path (Use cross platform thing to use this path!!)
--- @field is_game_running fun():boolean                                Returns true if a game is running. Usefull for async jobs.
--- @field hook fun(event: _HOOK_EVENT, func: function)                 Creates a hook for event
cr = {}

-- ============================================
-- Enums
-- ============================================

-- stylua: ignore start

--- Used to set card type
--- @enum cr.card_type
cr.card_type = {
        BASIC = 0,      -- Any card that have a positive effect. Icon at UI: +
        ITEM = 1,       -- An item. Icon at UI: +
        ENEMY = 2,      -- Any card that have a negative effect. Icon at UI: -
        EXIT = 3,       -- Type of the crogue:exit_gate. Icon at UI: #
}

--- Used to set log type
--- @enum cr.log_type
cr.log_type = {
        NORMAL = 0,     -- Displayed normaly
        WARN = 1,       -- Displayed yellow
        IMPORTANT = 2,  -- Displayed red
        DEBUG = 3       -- Displayed blue and italic. Displayed only on debug build of CROGUE, usefull to debug your plugin
}

-- stylua: ignore end

-- ============================================
-- Objects
-- ============================================

--- Crogue Objects
--- @class cr.obj
--- @field card cr.obj.card                     CROGUE cards
--- @field card_slot cr.obj.card_slot           CROGUE card slots
--- @field level cr.obj.level                   CROGUE levels
--- @field biome cr.obj.biome                   CROGUE biomes, used to manage and sort levels
--- @field buff cr.obj.buff                     CROUGE buffs, used to apply a constant effect
--- @field scene cr.obj.scene                   CROGUE scenes, used to create custom TUIs
cr.obj = {}

--- CROGUE cards
--- @class cr.obj.card
--- @field count integer                         Card count to generate in cr.stat.card_set
--- @field name string                           Card name
--- @field info string                           Card info
--- @field id string                             Card id, plugin_name:something syntax is recommended
--- @field type cr.card_type                     Card type
--- @field level_ids _CONTAINER<integer>         Card will appear on these levels, set to {} make it appear on all levels
--- @field logmsg string                         Log message will be displayed when card found
--- @field ttl integer                           Time-to-live see: https://github.com/HasanAgitUnal/CROGUE/wiki/Variable-Types
--- @field power integer                         Card power not damage!! see: https://github.com/HasanAgitUnal/CROGUE/wiki/Variable-Types
--- @field event fun():integer                   Card event, the return value will be appied to player health (use negative values to give damage)
--- @field new fun():cr.obj.card                 Create a new empty object
cr.obj.card = {}

--- CROGUE card slots
--- @class cr.obj.card_slot
--- @field front _SHARED_card|nil                Card at the front
--- @field back _SHARED_card|nil                 Card at the back
--- @field _lived integer                        See: https://github.com/HasanAgitUnal/CROGUE/wiki/Variable-Types
--- @field new fun():cr.obj.card_slot            Create a new empty object
cr.obj.card_slot = {}

--- CROGUE levels
--- @class cr.obj.level
--- @field name string                           Level name, use roman numbers
--- @field id integer                            Level ID, automaticaly setted when created with cr.create_level() function. Do not manualy set it.
--- @field new fun():cr.obj.level                Create a new empty object
cr.obj.level = {}

--- CROGUE biomes, used to manage and sort levels
--- @class cr.obj.biome
--- @field difficulty integer                    Difficulty of the biome, used to sort biomes. Range: 0-100
--- @field levels _CONTAINER<_SHARED_level>      Levels the biome contains
--- @field new fun():cr.obj.biome                Create a new empty object
cr.obj.biome = {}

--- CROUGE buffs, used to apply a constant effect
--- @class cr.obj.buff
--- @field name string                           Buff name
--- @field level integer                         Buff level. If its 0, user doesn't effected with this buff. So its event will be runned when level ~= 0
--- @field event fun(self: _SHARED_buff)         Event of the buff
--- @field new fun():cr.obj.buff                 Create a new empty object
cr.obj.buff = {}

--- CROGUE scenes, used to create custom TUIs
--- @class cr.obj.scene
--- @field exit_key integer                      ASCII value of the key to exit scene Defaults to 113 (ASCII 'q')
--- @field ui_refresh fun()                      The function will be used for refreshing UI
--- @field key_handler fun(key: integer):boolean Keyboard handler function. If returns true, scene ends. Takes ASCII code of the pressed key.
--- @field run fun()                             Method to start scene. Do not change it!
--- @field new fun():cr.obj.scene                Create a new empty object
cr.obj.scene = {}

-- ============================================
-- Shared Types
-- ============================================

--- Shared version of cr.obj.card. TYPE NAME IS NOT REAL!! Only for LSP integration.
--- @class _SHARED_card
--- @field count integer                                Card count to generate in cr.stat.card_set
--- @field name string                                  Card name
--- @field info string                                  Card info
--- @field id string                                    Card id, plugin_name:something syntax is recommended
--- @field type cr.card_type                            Card type
--- @field level_ids _CONTAINER<integer>                Card will appear on these levels, set to {} make it appear on all levels
--- @field logmsg string                                Log message will be displayed when card found
--- @field ttl integer                                  Time-to-live see: https://github.com/HasanAgitUnal/CROGUE/wiki/Variable-Types
--- @field power integer                                Card power not damage!! see: https://github.com/HasanAgitUnal/CROGUE/wiki/Variable-Types
--- @field event fun():integer                          Card event, the return value will be appied to player health (use negative values to give damage)

--- Shared version of cr.obj.level. TYPE NAME IS NOT REAL!! Only for LSP integration.
--- @class _SHARED_level
--- @field name string                                  Level name, use roman numbers
--- @field id integer                                   Level ID, automaticaly setted when created with cr.create_level() function. Do not manualy set it.

--- Shared version of cr.obj.biome. TYPE NAME IS NOT REAL!! Only for LSP integration.
--- @class _SHARED_biome
--- @field difficulty integer                           Difficulty of the biome, used to sort biomes. Range: 0-100
--- @field levels _CONTAINER<_SHARED_level>             Levels the biome contains

--- Shared version of cr.obj.buff. TYPE NAME IS NOT REAL!! Only for LSP integration.
--- @class _SHARED_buff
--- @field name string                                  Buff name
--- @field level integer                                Buff level. If its 0, user doesn't effected with this buff. So its event will be runned when level ~= 0
--- @field event fun(self: _SHARED_buff)                Event of the buff

--- Normal to shared object converter functions
--- @class cr.shared
--- @field card fun(card:cr.obj.card):_SHARED_card              Makes given card a shared card
--- @field buff fun(buff:cr.obj.buff):_SHARED_buff              Makes given buff a shared buff
--- @field level fun(level:cr.obj.level):_SHARED_level          Makes given level a shared level
--- @field biome fun(biome:cr.obj.biome):_SHARED_biome          Makes given biome a shared biome

-- ============================================
-- Game Status Variables
-- ============================================

--- Game Status Variables
--- @class cr.stat
--- @field deck _CONTAINER<_SHARED_card>                        Card deck, cards first registered to this container
--- @field card_set _CONTAINER<_SHARED_card>                    Current cards in the game
--- @field slot1 cr.obj.card_slot                               Card slot1
--- @field slot2 cr.obj.card_slot                               Card slot2
--- @field slot3 cr.obj.card_slot                               Card slot3
--- @field biomes _CONTAINER<_SHARED_biome>                     Biome registry
--- @field levels _CONTAINER<_SHARED_level>                     Level list generated from cr.stat.biome. Should be regenerated when a new biome added to the cr.stat.biome
--- @field buffs _CONTAINER<_SHARED_buff>                       Buffs are stored inside this container, every turn CROGUE checks their levels and runs their event if level ~= 0
--- @field logs _CONTAINER<_log>                                Last logs are stored there. Do not trust it completely, old logs are deleted automaticaly.
--
--- @field get_levelid fun():integer                            Get levelid
--- @field set_levelid fun(value: integer)                      Set levelid
--- @field get_seed fun():string                                Get seed
--- @field set_seed fun(value: string)                          Set seed
--
cr.stat = {}

--- Player Variables
--- @class cr.player
--- @field inventory _CONTAINER<_SHARED_card>                   Player inventory. Contains collected items
--- @field get_hp fun():integer                                 Get player hp
--- @field set_hp fun(value: integer)                           Set player hp
--- @field get_level fun():integer                              Get current level index (for cr.stat.levels)
--- @field set_level fun(value: integer)                        Set current level index (for cr.stat.levels)
cr.player = {}

-- ============================================
-- TUI
-- ============================================

--- CROGUE TUI Functions
--- @class cr.tui
--- @field print_ansi fun(ansi: string)                         Prints given ANSI art using NCurses.
--- @field print_line fun(line: integer, win?: _CURSES_window)  Prints a line to given line using NCurses. Optionaly takes a NCurses window.
--- @field print_slots fun(line: integer):integer               Prints slots. May be used to refresh UI but you will need some magic numbers from CROGUE source code.
--- @field print_stats fun(line: integer)                       Prints stats. May be used to refresh UI but you will need some magic numbers from CROGUE source code.
--- @field print_buffs fun(line: integer)                       Prints buffs. May be used to refresh UI but you will need some magic numbers from CROGUE source code.
--- @field print_logs fun(line: integer):integer                Prints logs. May be used to refresh UI but you will need some magic numbers from CROGUE source code.
--- @field print_inventory fun():integer                        Prints player inventory. May be used to refresh UI but you will need some magic numbers from CROGUE source code.
--- @field print_all fun()                                      Refreshes the UI completely
cr.tui = {}

-- ============================================
-- Functions
-- ============================================

--- Ask for a key to user
--- @param what string                          The question
--- @return integer ASCII                       ASCII code of the key
function cr.ask(what) end

--- Ask for a text to user
--- @param what string                          The question
--- @return string text                         The text user entered
function cr.ask_string(what) end

--- Display a log with given urgency
--- @param msg string                           Log message
--- @param type cr.log_type                     Urgency
function cr.log(msg, type) end

--- Creates a shared card, adds to cr.stat.deck and returns the card.
--- @param table _ARGS_create_card              Card properties
--- @return _SHARED_card card                   Created card
function cr.create_card(table) end

--- Creates a shared level with given name, assigns an ID, and returns the level
--- @param name string                          Level name
--- @return _SHARED_level level                 Created level
function cr.create_level(name) end

--- Creates a shared biome, adds to cr.stat.biomes, and returns the biome
--- @param table _ARGS_create_biome             Biome properties
--- @return _SHARED_biome biome                 Created biome
function cr.create_biome(table) end

--- Creates a shared buff, adds to cr.stat.buffs, and returns the biome
--- @param table _ARGS_create_buff              Buff properties
--- @return _SHARED_buff buff                   Created buff
function cr.create_buff(table) end

--- Creates a scene and returns it
--- @param table _ARGS_create_scene             Scene properties
--- @return cr.obj.scene scene                  Created scene
function cr.create_scene(table) end

--- Resets game
--- @param full boolean                         Hard reset! May fuck game
function cr.reset_game(full) end

--- Generates cr.stat.levels from cr.stat.biomes
function cr.generate_levels() end

--- Generates cr.stat.card_set from cr.stat.deck
function cr.draw_cards() end

--- Draws card to all slots
function cr.draw_slots() end

--- Does the action when user picks a slot
--- @param slot cr.obj.card_slot                The slot
function cr.handle_slot(slot) end

--- Checks level of all buffs inside (cr.stat.buffs), runs their events if level ~= 0
function cr.handle_buffs() end

--- Runs the card event and applies HP change and extra damage (player_hp = player_hp + event_return - extra_damage)
--- @param card _SHARED_card                    The card
--- @param extra integer                        Extra damage
function cr.basic_card_event(card, extra) end

--- Does action related to type of the card
--- If BASIC, EXIT, ENEMY: passes arguments to cr.basic_card_event
--- If ITEM: adds card to inventory and displays a log ("You found item: %s")
--- @param card _SHARED_card                    The card
--- @param extra integer                        Extra damage
function cr.card_event(card, extra) end

--- Returns settings of the given plugin. see: https://github.com/HasanAgitUnal/CROGUE/wiki/Settings
--- @param plugin string                        Plugin name, mostly call with your plugin's name
--- @return table settings                      Plugin settings as table.
function cr.settings(plugin) end

--- Returns data directory path (Use cross platform thing to use this path!!)
--- @return string path                         Directory path
function cr.get_data_dir() end

--- Returns true if a game is running. Usefull for async jobs.
--- @return boolean is_running                  Is running?
function cr.is_game_running() end

--- Creates a hook for event
--- @overload fun(event: "before_refresh", func: fun())                                                 Always runned before UI refresh.
--- @overload fun(event: "after_refresh", func: fun())                                                  Always runned after UI refresh.
--- @overload fun(event: "start", func: fun())                                                          Runned before everything and main menu.
--- @overload fun(event: "game_start", func: fun())                                                     When a new game starts.
--- @overload fun(event: "game_end", func: fun())                                                       When a game ends.
--- @overload fun(event: "game_quit", func: fun())                                                      When user quits from game.
--- @overload fun(event: "reload", func: fun())                                                         Runned when plugins are reloaded.
--- @overload fun(event: "ending", func: fun())                                                         Runned when player finds Amulet of Yendor.
--- @overload fun(event: "draw", func: fun())                                                           Runned after `cr.draw_cards()`.
--- @overload fun(event: "level_gen", func: fun())                                                      Runned after `cr.generate_levels()`
--- @overload fun(event: "die", func: fun())                                                            Runned when player dies.
--- @overload fun(event: "key", func: fun(key: integer))                                                Runned when a key pressed on main game loop. No return type, Takes an integer argument: the key pressed (ASCI).
--- @overload fun(event: "level_up", func: fun(level: integer))                                         Runned when exit gate found. No return type. Takes an integer argument: current level index.
--- @overload fun(event: "slot", func: fun(slot: integer): boolean)                                     Runned when a slot picked. Has bool return type: if true, slot will be skipped for now. Takes an integer argument: slots number (1, 2, 3)
--- @overload fun(event: "item", func: fun(card: _SHARED_card): boolean)                                Runned when user wants to use an item. Has bool return type: if true, using item is canceled. Takes an shared card argument: the item used.
--- @overload fun(event: "card_event", func: fun(card: _SHARED_card, extra: integer): boolean)          Runned when `cr.basic_card_event` called. Has bool return type: if true, card event is canceled. Takes arguments: the card (shared card), extra damage value (integer).
--- @overload fun(event: "s_save", func: fun(data: string))                                             Runned after a save created/updated. No return type, Takes 1 string argument: save data as json. See https://github.com/HasanAgitUnal/CROGUE/wiki/Hooks for example save data
--- @overload fun(event: "s_load", func: fun(data: string))                                             Runned after a save loaded. No return type, Takes 1 string argument: save data as json. See https://github.com/HasanAgitUnal/CROGUE/wiki/Hooks for example save data
--- @overload fun(event: "damage", func: fun(id: string, base: integer, extra: integer))                Runned after a card event runned and before changing HP (with card event return value and extra damage).
function cr.hook(event, func) end

-- ============================================
-- NCurses
-- ============================================

--- NCurses Access for plugins. Things under this table is not documented there, see official NCurses documentation.
--- @class cr.curses
--- @field stdscr _CURSES_window
--- @field attr_t _CURSES_attr_t
--
--- @field ansi2attr fun(ansi: string): _CURSES_attr_t
--
--- @field move fun(y: integer, x: integer): integer
--- @field wmove fun(win: _CURSES_window, y: integer, x: integer): integer
--- @field clear fun(): integer
--- @field wclear fun(win: _CURSES_window): integer
--- @field erase fun(): integer
--- @field werase fun(win: _CURSES_window): integer
--- @field refresh fun(): integer
--- @field wrefresh fun(win: _CURSES_window): integer
--- @field prefresh fun(pad: _CURSES_window, pminrow: integer, pmincol: integer, sminrow: integer, smincol: integer, smaxrow: integer, smaxcol: integer): integer
--- @field wnoutrefresh fun(win: _CURSES_window): integer
--- @field pnoutrefresh fun(pad: _CURSES_window, pminrow: integer, pmincol: integer, sminrow: integer, smincol: integer, smaxrow: integer, smaxcol: integer): integer
--- @field doupdate fun(): integer
--- @field resize_term fun(lines: integer, cols: integer): integer
--
--- @field keyname fun(c: integer): string
--- @field key_name fun(c: integer): string      -- actually takes wchar_t, but int in binding
--- @field unctrl fun(c: _CURSES_chtype): string
--
--- @field napms fun(ms: integer): integer
--- @field beep fun(): integer
--- @field flash fun(): integer
--
--- @field scrl fun(n: integer): integer
--- @field wscrl fun(win: _CURSES_window, n: integer): integer
--- @field setscrreg fun(top: integer, bottom: integer): integer
--- @field wsetscrreg fun(win: _CURSES_window, top: integer, bottom: integer): integer
--
--- @field touchwin fun(win: _CURSES_window): integer
--- @field untouchwin fun(win: _CURSES_window): integer
--- @field touchline fun(win: _CURSES_window, start: integer, count: integer): integer
--- @field wtouchln fun(win: _CURSES_window, y: integer, n: integer, changed: integer): integer
--- @field is_linetouched fun(win: _CURSES_window, line: integer): boolean
--- @field is_wintouched fun(win: _CURSES_window): boolean
--
--- @field bkgd fun(ch: _CURSES_chtype): integer
--- @field wbkgd fun(win: _CURSES_window, ch: _CURSES_chtype): integer
--- @field bkgdset fun(ch: _CURSES_chtype)
--- @field wbkgdset fun(win: _CURSES_window, ch: _CURSES_chtype)
--- @field getbkgd fun(win: _CURSES_window): _CURSES_chtype
--
--- @field insch fun(ch: _CURSES_chtype): integer
--- @field winsch fun(win: _CURSES_window, ch: _CURSES_chtype): integer
--- @field mvinsch fun(y: integer, x: integer, ch: _CURSES_chtype): integer
--- @field mvwinsch fun(win: _CURSES_window, y: integer, x: integer, ch: _CURSES_chtype): integer
--
--- @field insstr fun(str: string): integer
--- @field winsstr fun(win: _CURSES_window, str: string): integer
--- @field mvinsstr fun(y: integer, x: integer, str: string): integer
--- @field mvwinsstr fun(win: _CURSES_window, y: integer, x: integer, str: string): integer
--
--- @field insnstr fun(str: string, n: integer): integer
--- @field winsnstr fun(win: _CURSES_window, str: string, n: integer): integer
--- @field mvinsnstr fun(y: integer, x: integer, str: string, n: integer): integer
--- @field mvwinsnstr fun(win: _CURSES_window, y: integer, x: integer, str: string, n: integer): integer
--
--- @field addnstr fun(str: string, n: integer): integer
--- @field waddnstr fun(win: _CURSES_window, str: string, n: integer): integer
--- @field mvaddnstr fun(y: integer, x: integer, str: string, n: integer): integer
--- @field mvwaddnstr fun(win: _CURSES_window, y: integer, x: integer, str: string, n: integer): integer
--
--- @field addchstr fun(chstr: _CURSES_chtype[]): integer
--- @field waddchstr fun(win: _CURSES_window, chstr: _CURSES_chtype[]): integer
--- @field mvaddchstr fun(y: integer, x: integer, chstr: _CURSES_chtype[]): integer
--- @field mvwaddchstr fun(win: _CURSES_window, y: integer, x: integer, chstr: _CURSES_chtype[]): integer
--
--- @field addchnstr fun(chstr: _CURSES_chtype[], n: integer): integer
--- @field waddchnstr fun(win: _CURSES_window, chstr: _CURSES_chtype[], n: integer): integer
--- @field mvaddchnstr fun(y: integer, x: integer, chstr: _CURSES_chtype[], n: integer): integer
--- @field mvwaddchnstr fun(win: _CURSES_window, y: integer, x: integer, chstr: _CURSES_chtype[], n: integer): integer
--
--- @field inch fun(): _CURSES_chtype
--- @field winch fun(win: _CURSES_window): _CURSES_chtype
--- @field mvinch fun(y: integer, x: integer): _CURSES_chtype
--- @field mvwinch fun(win: _CURSES_window, y: integer, x: integer): _CURSES_chtype
--
--- @field instr fun(str: string): integer
--- @field winstr fun(win: _CURSES_window, str: string): integer
--- @field mvinstr fun(y: integer, x: integer, str: string): integer
--- @field mvwinstr fun(win: _CURSES_window, y: integer, x: integer, str: string): integer
--
--- @field innstr fun(str: string, n: integer): integer
--- @field winnstr fun(win: _CURSES_window, str: string, n: integer): integer
--- @field mvinnstr fun(y: integer, x: integer, str: string, n: integer): integer
--- @field mvwinnstr fun(win: _CURSES_window, y: integer, x: integer, str: string, n: integer): integer
--
--- @field inchstr fun(chstr: _CURSES_chtype[]): integer
--- @field winchstr fun(win: _CURSES_window, chstr: _CURSES_chtype[]): integer
--- @field mvinchstr fun(y: integer, x: integer, chstr: _CURSES_chtype[]): integer
--- @field mvwinchstr fun(win: _CURSES_window, y: integer, x: integer, chstr: _CURSES_chtype[]): integer
--
--- @field inchnstr fun(chstr: _CURSES_chtype[], n: integer): integer
--- @field winchnstr fun(win: _CURSES_window, chstr: _CURSES_chtype[], n: integer): integer
--- @field mvinchnstr fun(y: integer, x: integer, chstr: _CURSES_chtype[], n: integer): integer
--- @field mvwinchnstr fun(win: _CURSES_window, y: integer, x: integer, chstr: _CURSES_chtype[], n: integer): integer
--
--- @field newwin fun(lines: integer, cols: integer, y: integer, x: integer): _CURSES_window
--- @field newpad fun(lines: integer, cols: integer): _CURSES_window
--- @field derwin fun(parent: _CURSES_window, lines: integer, cols: integer, y: integer, x: integer): _CURSES_window
--- @field dupwin fun(win: _CURSES_window): _CURSES_window
--- @field mvwin fun(win: _CURSES_window, y: integer, x: integer): integer
--- @field delwin fun(win: _CURSES_window): integer
--- @field copywin fun(src: _CURSES_window, dst: _CURSES_window, sminrow: integer, smincol: integer, dminrow: integer, dmincol: integer, dmaxrow: integer, dmaxcol: integer, overlay: integer): integer
--- @field wresize fun(win: _CURSES_window, lines: integer, cols: integer): integer
--
--- @field keypad fun(win: _CURSES_window, bf: boolean): integer
--- @field timeout fun(delay: integer)
--- @field wtimeout fun(win: _CURSES_window, delay: integer)
--- @field nodelay fun(win: _CURSES_window, bf: boolean): integer
--- @field notimeout fun(win: _CURSES_window, bf: boolean): integer
--- @field meta fun(win: _CURSES_window, bf: boolean): integer
--- @field intrflush fun(win: _CURSES_window, bf: boolean): integer
--- @field halfdelay fun(tenths: integer): integer
--- @field typeahead fun(fd: integer): integer
--- @field qiflush fun()
--- @field noqiflush fun()
--
--- @field leaveok fun(win: _CURSES_window, bf: boolean): integer
--- @field scrollok fun(win: _CURSES_window, bf: boolean): integer
--- @field clearok fun(win: _CURSES_window, bf: boolean): integer
--- @field idlok fun(win: _CURSES_window, bf: boolean): integer
--- @field idcok fun(win: _CURSES_window, bf: boolean)
--- @field immedok fun(win: _CURSES_window, bf: boolean)
--- @field syncok fun(win: _CURSES_window, bf: boolean): integer
--- @field redrawwin fun(win: _CURSES_window): integer
--- @field wredrawln fun(win: _CURSES_window, beg_line: integer, num_lines: integer): integer
--
--- @field cbreak fun(): integer
--- @field nocbreak fun(): integer
--- @field raw fun(): integer
--- @field noraw fun(): integer
--- @field echo fun(): integer
--- @field noecho fun(): integer
--- @field nl fun(): integer
--- @field nonl fun(): integer
--- @field filter fun()
--
--- @field has_colors fun(): boolean
--
--- @field curs_set fun(visibility: integer): integer
--- @field def_prog_mode fun(): integer
--- @field reset_prog_mode fun(): integer
--- @field def_shell_mode fun(): integer
--- @field reset_shell_mode fun(): integer
--- @field savetty fun(): integer
--- @field resetty fun(): integer
--- @field endwin fun(): integer
--- @field isendwin fun(): boolean
--
--- @field printw fun(fmt: string, ...: any): integer
--- @field wprintw fun(win: _CURSES_window, fmt: string, ...: any): integer
--- @field mvprintw fun(y: integer, x: integer, fmt: string, ...: any): integer
--- @field mvwprintw fun(win: _CURSES_window, y: integer, x: integer, fmt: string, ...: any): integer
--
--- @field addch fun(ch: _CURSES_chtype): integer
--- @field waddch fun(win: _CURSES_window, ch: _CURSES_chtype): integer
--- @field mvaddch fun(y: integer, x: integer, ch: _CURSES_chtype): integer
--- @field mvwaddch fun(win: _CURSES_window, y: integer, x: integer, ch: _CURSES_chtype): integer
--
--- @field addstr fun(str: string): integer
--- @field waddstr fun(win: _CURSES_window, str: string): integer
--- @field mvaddstr fun(y: integer, x: integer, str: string): integer
--- @field mvwaddstr fun(win: _CURSES_window, y: integer, x: integer, str: string): integer
--
--- @field delch fun(): integer
--- @field wdelch fun(win: _CURSES_window): integer
--- @field mvdelch fun(y: integer, x: integer): integer
--- @field mvwdelch fun(win: _CURSES_window, y: integer, x: integer): integer
--
--- @field insertln fun(): integer
--- @field winsertln fun(win: _CURSES_window): integer
--
--- @field deleteln fun(): integer
--- @field wdeleteln fun(win: _CURSES_window): integer
--
--- @field insdelln fun(n: integer): integer
--- @field winsdelln fun(win: _CURSES_window, n: integer): integer
--
--- @field clrtoeol fun(): integer
--- @field wclrtoeol fun(win: _CURSES_window): integer
--
--- @field clrtobot fun(): integer
--- @field wclrtobot fun(win: _CURSES_window): integer
--
--- @field box fun(win: _CURSES_window, verch: _CURSES_chtype, horch: _CURSES_chtype): integer
--- @field border fun(ls: _CURSES_chtype, rs: _CURSES_chtype, ts: _CURSES_chtype, bs: _CURSES_chtype, tl: _CURSES_chtype, tr: _CURSES_chtype, bl: _CURSES_chtype, br: _CURSES_chtype): integer
--
--- @field hline fun(ch: _CURSES_chtype, n: integer): integer
--- @field whline fun(win: _CURSES_window, ch: _CURSES_chtype, n: integer): integer
--- @field mvhline fun(y: integer, x: integer, ch: _CURSES_chtype, n: integer): integer
--- @field mvwhline fun(win: _CURSES_window, y: integer, x: integer, ch: _CURSES_chtype, n: integer): integer
--
--- @field vline fun(ch: _CURSES_chtype, n: integer): integer
--- @field wvline fun(win: _CURSES_window, ch: _CURSES_chtype, n: integer): integer
--- @field mvvline fun(y: integer, x: integer, ch: _CURSES_chtype, n: integer): integer
--- @field mvwvline fun(win: _CURSES_window, y: integer, x: integer, ch: _CURSES_chtype, n: integer): integer
--
--- @field attron fun(attrs: _CURSES_attr_t): integer
--- @field wattron fun(win: _CURSES_window, attrs: _CURSES_attr_t): integer
--- @field attrset fun(attrs: _CURSES_attr_t): integer
--- @field wattrset fun(win: _CURSES_window, attrs: _CURSES_attr_t): integer
--- @field attroff fun(attrs: _CURSES_attr_t): integer
--- @field wattroff fun(win: _CURSES_window, attrs: _CURSES_attr_t): integer
--- @field chgat fun(n: integer, attr: _CURSES_attr_t, pair: integer, opts: any?): integer
--- @field wchgat fun(win: _CURSES_window, n: integer, attr: _CURSES_attr_t, pair: integer, opts: any?): integer
--- @field mvchgat fun(y: integer, x: integer, n: integer, attr: _CURSES_attr_t, pair: integer, opts: any?): integer
--- @field mvwchgat fun(win: _CURSES_window, y: integer, x: integer, n: integer, attr: _CURSES_attr_t, pair: integer, opts: any?): integer
--
--- @field ungetch fun(ch: integer): integer
--
--- @field getch fun(): integer
--- @field wgetch fun(win: _CURSES_window): integer
--- @field mvgetch fun(y: integer, x: integer): integer
--- @field mvwgetch fun(win: _CURSES_window, y: integer, x: integer): integer
--
--- @field getstr fun(str: string): integer
--- @field wgetstr fun(win: _CURSES_window, str: string): integer
--- @field mvgetstr fun(y: integer, x: integer, str: string): integer
--- @field mvwgetstr fun(win: _CURSES_window, y: integer, x: integer, str: string): integer
--
--- @field getnstr fun(str: string, n: integer): integer
--- @field wgetnstr fun(win: _CURSES_window, str: string, n: integer): integer
--- @field mvgetnstr fun(y: integer, x: integer, str: string, n: integer): integer
--- @field mvwgetnstr fun(win: _CURSES_window, y: integer, x: integer, str: string, n: integer): integer
--
--- @field flushinp fun(): integer
--
--- @field getyx fun(win: _CURSES_window): _CURSES_coords
--- @field getbegyx fun(win: _CURSES_window): _CURSES_coords
--- @field getparyx fun(win: _CURSES_window): _CURSES_coords
--- @field getmaxyx fun(win: _CURSES_window): _CURSES_coords
--
--- @field LINES fun(): integer
--- @field COLS fun(): integer
--
--- @field OK 0
--- @field ERR -1
--
--- @field KEY_CODE_YES integer
--- @field KEY_MIN integer
--- @field KEY_BREAK integer
--- @field KEY_SRESET integer
--- @field KEY_RESET integer
--- @field KEY_DOWN integer
--- @field KEY_UP integer
--- @field KEY_LEFT integer
--- @field KEY_RIGHT integer
--- @field KEY_HOME integer
--- @field KEY_BACKSPACE integer
--- @field KEY_F0 integer
--- @field KEY_F fun(n: integer): integer
--- @field KEY_DL integer
--- @field KEY_IL integer
--- @field KEY_DC integer
--- @field KEY_IC integer
--- @field KEY_EIC integer
--- @field KEY_CLEAR integer
--- @field KEY_EOS integer
--- @field KEY_EOL integer
--- @field KEY_SF integer
--- @field KEY_SR integer
--- @field KEY_NPAGE integer
--- @field KEY_PPAGE integer
--- @field KEY_STAB integer
--- @field KEY_CTAB integer
--- @field KEY_CATAB integer
--- @field KEY_ENTER integer
--- @field KEY_PRINT integer
--- @field KEY_LL integer
--- @field KEY_A1 integer
--- @field KEY_A3 integer
--- @field KEY_B2 integer
--- @field KEY_C1 integer
--- @field KEY_C3 integer
--- @field KEY_BTAB integer
--- @field KEY_BEG integer
--- @field KEY_CANCEL integer
--- @field KEY_CLOSE integer
--- @field KEY_COMMAND integer
--- @field KEY_COPY integer
--- @field KEY_CREATE integer
--- @field KEY_END integer
--- @field KEY_EXIT integer
--- @field KEY_FIND integer
--- @field KEY_HELP integer
--- @field KEY_MARK integer
--- @field KEY_MESSAGE integer
--- @field KEY_MOVE integer
--- @field KEY_NEXT integer
--- @field KEY_OPEN integer
--- @field KEY_OPTIONS integer
--- @field KEY_PREVIOUS integer
--- @field KEY_REDO integer
--- @field KEY_REFERENCE integer
--- @field KEY_REFRESH integer
--- @field KEY_REPLACE integer
--- @field KEY_RESTART integer
--- @field KEY_RESUME integer
--- @field KEY_SAVE integer
--- @field KEY_SBEG integer
--- @field KEY_SCANCEL integer
--- @field KEY_SCOMMAND integer
--- @field KEY_SCOPY integer
--- @field KEY_SCREATE integer
--- @field KEY_SDC integer
--- @field KEY_SDL integer
--- @field KEY_SELECT integer
--- @field KEY_SEND integer
--- @field KEY_SEOL integer
--- @field KEY_SEXIT integer
--- @field KEY_SFIND integer
--- @field KEY_SHELP integer
--- @field KEY_SHOME integer
--- @field KEY_SIC integer
--- @field KEY_SLEFT integer
--- @field KEY_SMESSAGE integer
--- @field KEY_SMOVE integer
--- @field KEY_SNEXT integer
--- @field KEY_SOPTIONS integer
--- @field KEY_SPREVIOUS integer
--- @field KEY_SPRINT integer
--- @field KEY_SREDO integer
--- @field KEY_SREPLACE integer
--- @field KEY_SRIGHT integer
--- @field KEY_SRSUME integer
--- @field KEY_SSAVE integer
--- @field KEY_SSUSPEND integer
--- @field KEY_SUNDO integer
--- @field KEY_SUSPEND integer
--- @field KEY_UNDO integer
--- @field KEY_MOUSE integer
--- @field KEY_RESIZE integer
--- @field KEY_MAX integer
cr.curses = {}
