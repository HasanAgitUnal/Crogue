Create `metadata.json` file under your plugin directory root. JSON5 or JSONC is not supported. And extra keys are allowed, so you can use comments with adding a key like "comment", "author", "something" etc.

Format:
```json
{
        "name": "My Plugin's Visual Name",
        "description": "Description for my plugin",
        "settings": [
                {
                        "type": "switch",
                        "name": "That is a switch",
                        "description": "You can enable/disable something with this",
                        "option": "access_with_this_name",
                        "default": true
                },
                {
                        "type": "input",
                        "name": "An Input Box",
                        "description": "Description of input",
                        "option": "my_input_box",
                        "default": "default value"
                },
                {
                        "type": "choose",
                        "name": "Choose from menu",
                        "description": "menu description",
                        "option": "my_menu",
                        "values": [
                                "a",
                                "b",
                                "c"
                        ],
                        "default": "b"
                }
        ]
}
```

You already know what `name` and `description` means.
The `settings` array is **optional** but very usefull when you want to user can change behavior of your plugin on plugin manager > settings at main menu.
All keys of the objects under `settings` are **REQUIRED** when `settings` is given. There is no limit for adding settings but it can't be empty.

If `metadata.json` is invalid, CROGUE won't load your plugin and give an error to user.

See [Settings](./Settings) for usage of this settings on lua code.


