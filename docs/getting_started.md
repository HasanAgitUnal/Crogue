# Getting Started

Let's create a plugin.

Plugins directory locations:
- Linux: If `$XDG_DATA_DIR` environment set: `$XDG_DATA_DIR/crogue/plugins`, if not set defaults to `$HOME/.local/share/crogue/plugins`
- Windows: `%APPDATA%\crogue\plugins`

Create a new directory with your plugin name inside plugins directory.
Then create a new file called `init.lua`.
Then start writing your code!
Look at [First Plugin](./first_plugin.md) to see an example.
You can also use `require` to create a modular plugin.

