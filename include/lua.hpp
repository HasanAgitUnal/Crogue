#pragma once

#include <filesystem>

namespace fs = std::filesystem;

fs::path get_data_dir();
void cleanup_lua();
void setup_lua();
void load_plugins();
