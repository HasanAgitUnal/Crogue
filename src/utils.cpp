#include <charconv>
#include <string>
#include <string_view>
#include <vector>

std::vector<std::string> wrap_text(std::string &text, int width) {

        std::vector<std::string> lines;
        std::stringstream ss(text);
        std::string word, line;
        while (ss >> word) {
                if (line.length() + word.length() + 1 > (size_t)width) {
                        lines.push_back(line);
                        line = word;
                } else {
                        if (!line.empty())
                                line += " ";
                        line += word;
                }
        }
        if (!line.empty())
                lines.push_back(line);
        return lines;
}

std::string trim(const std::string &str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos)
                return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
}

std::vector<std::string_view> split(std::string_view str, std::string_view delim) {
        std::vector<std::string_view> output;
        size_t first = 0;
        size_t last = str.find_first_of(delim);

        while (last != std::string_view::npos) {
                output.emplace_back(str.substr(first, last - first));
                first = last + 1;
                last = str.find_first_of(delim, first);
        }

        output.emplace_back(str.substr(first));
        return output;
}

int to_int(std::string_view sv) {
        int value = 0;
        std::from_chars(sv.data(), sv.data() + sv.size(), value);
        return value;
}

std::string to_roman(int n) {
        if (n < 0) {
                return "-" + to_roman(-n);
        }

        struct romandata_t {
                int val;
                const char *res;
        };

        static const romandata_t data[] = {{1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"},
                                           {90, "XC"},  {50, "L"},   {40, "XL"}, {10, "X"},   {9, "IX"},
                                           {5, "V"},    {4, "IV"},   {1, "I"}};
        std::string res = "";
        for (const auto &entry : data) {
                while (n >= entry.val) {
                        res += entry.res;
                        n -= entry.val;
                }
        }
        return res;
}
