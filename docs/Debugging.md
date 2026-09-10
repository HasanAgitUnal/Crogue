Debug build of CROGUE is very usefull for plugin developers.
On debug build, you can see logs and maybe do some cheat mode stuff on debug build.

To check if debug build of CROGUE is running with lua API:
```lua
if cr.debug then
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
