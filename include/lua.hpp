#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// from lua.cpp
void cleanup_lua();
void end_program();
void load_plugins();
std::string check_metadata(const json &metadata);

// from lua_bindings.cpp
void setup_lua();
