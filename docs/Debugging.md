You can get CROGUE version via `cr.debug.version` table:
```lua
-- Lets say we are running CROGUE v0.1.2
cr.debug.version.major -- will return 0
cr.debug.version.minor -- will return 1
cr.debug.version.patch -- will returnn 2

-- will return "0.1.2" string on release build
-- if debug build of crogue running (CROGUE v0.1.2-debug for example), it will return "0.1.2-debug"
cr.debug.version.string
```

Debug build of CROGUE is very usefull for plugin developers.
On debug build, you can see logs and maybe do some cheat mode stuff on debug build.

To check if debug build of CROGUE is running with lua API:
```lua
if cr.debug.is_debug_build then
    -- something...
end
```

### Logs
If `build` directory exists inside current working directory, CROGUE prints logs to `build/debug.log` file.
Run `tail -F build/debug.log` on a terminal window to see logs while testing your plugin.
You can also use debug logs with `cr.log` function. These are like normal but displayed only when debug build of CROGUE is running:
```lua
cr.log("This is a debug message!", cr.log_type.DEBUG)
```
