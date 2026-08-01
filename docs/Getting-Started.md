### Data Directory Location & Structure

Default data directory location depending on your system:

**Windows:**
  * if `%APPDATA%` environment variable defined: `%APPDATA%\crogue`
  * if not defaults to: `%USERPROFILE%\AppData\Roaming\crogue`

**POSIX:**
  * if `XDG_DATA_HOME` envirenment variable defined: `$XDG_DATA_HOME/crogue`
  * if not default to `~/.local/share/crogue`

You can also learn it with `--where-is-my-data` (or `-w`) CLI flag:
```bash
$ crogue --where-is-my-data
/home/melon/.local/share/crogue
```

And you can set it with `--data` (`-d`) option:
```bash
$ crogue --data /path/to/custom_data_dir/
```

Directory Structure:
```bash
├── plugins                 # Plugins directory
│   └── my_plugin           # A plugin
│       ├── init.lua        # Lua code
│       ├── metadata.json   # Plugin metadata
│       ├── pack_name.txt   # Plugin's name
│       ├── settings.json   # Settings of the file generated from metadata
│       └── git_repo.json   # Exists if plugin is installed from git, contains repo url and current commit
│
├── saves                   # Saves directory
│   └──  4342664553149908956_1785512851.json
│
└──  settings.json         # Main game settings, contains plugins enabled or not
```

### Creating your first plugin

Create a new directory for your plugin. And create these files:
```txt
init.lua
metadata.json
pack_name.txt
```

Pick a name for your plugin. And put it inside `pack_name.txt`:
```txt
my_plugin
```

Edit your [`metadata.json`](./Metadata.md):
```json
{
    "name": "My Plugin",
    "description": "This is my first plugin"
}
```

Then write put some code inside `init.lua`:
```lua
cr.create_card({
        count = 5,
        name = "Skeleton",
        type = cr.card_type.ENEMY,
        level_ids = {},    -- appears on every level
        logmsg = "Last words of the Skeleton was \"AAA!\"",
        ttl = 3,            -- time-to-live
        power = 1,
        event = function()
                return -1   -- -1 health
        end
})
```

Congrulations you created your first plugin.

### Sharing with others

Build it with `crogue pm build`:
```bash
crogue pm build
```

> [!NOTE]
> You can manualy create a `.zip` file but this commands checks is your plugin is valid.

Now you can share the `.zip` file with anyone. They will install plugin with this command:
```bash
crogue pm install -f my_plugin
```

But sharing file is not a good way to share, upload your plugins source files to a git repository.
Lets say your repository is `https://github.com/my_name/my_plugin.git`.
Installation command:
```bash
crogue pm install -g https://github.com/my_name/my_plugin.git
```

And if you change something they can update plugin version with `crogue pm update my_plugin`:
```bash
crogue pm update my_plugin
```
