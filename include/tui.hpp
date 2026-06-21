#pragma once

#include <string>

void setup_colors();

int ask(std::string what);
std::string ask_string(std::string what);

int print_slots(int line);
void print_stats(int line);
void print_buffs(int line);
int print_logs(int line);
int print_inventory(int line);
void print_ui();
