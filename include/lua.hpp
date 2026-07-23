#pragma once

#include <filesystem>

namespace fs = std::filesystem;

// from lua.cpp
fs::path get_data_dir();
void cleanup_lua();
void end_program();
void load_plugins();

// from lua_bindings.cpp
void setup_lua();
