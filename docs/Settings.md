> [!NOTE]
> Read [Metadata](./Metadata) first

Access settings with `cr.settings()` function with your plugins name (plugin's directory name):
```lua
local settings = cr.settings("my_plugin")

-- Switches has bool value type
local my_input = settings.access_with_this_name

-- Input boxes and choose menus have string value type
local my_input_box = settings.my_input_box
local choosen_char = settings.my_menu

-- Do something with this values
```

`cr.settings()` return a table contains your plugin's settings **and** also always returns a `enabled` field. Using `cr.settings()` for your plugin always returns `enabled` as `true`. But you may use this field to see an another plugin is loaded if you know its name. Return `nil` if plugin not found.
