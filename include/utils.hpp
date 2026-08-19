#pragma once
#include <string>
#include <string_view>
#include <vector>

std::vector<std::string> wrap_text(std::string text, int width);
std::string trim(const std::string &str);
std::vector<std::string_view> split(std::string_view str, std::string_view delim);
int to_int(std::string_view sv);
std::string to_roman(int n);
