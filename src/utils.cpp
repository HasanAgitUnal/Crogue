#include <charconv>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// regex special chars
static bool isRegexSpecial(char c) {
        switch (c) {
                case '.':
                case '^':
                case '$':
                case '*':
                case '+':
                case '?':
                case '(':
                case ')':
                case '[':
                case '{':
                case '}':
                case '\\':
                case '|':
                case ']':
                case '-':
                        return true;
                default:
                        return false;
        }
}

static std::string convertPattern(const std::string &pat) {
        std::string result;
        size_t i = 0;
        while (i < pat.size()) {
                char c = pat[i];

                if (c == '\\') {
                        if (i + 1 < pat.size()) {
                                char next = pat[i + 1];
                                if (isRegexSpecial(next))
                                        result.push_back('\\');
                                result.push_back(next);
                                i += 2;
                        } else {
                                result.push_back('\\');
                                i++;
                        }
                        continue;
                }

                if (c == '[') {
                        size_t start = i;
                        size_t end = pat.find(']', i + 1);
                        if (end != std::string::npos) {
                                std::string cls = pat.substr(i, end - i + 1);
                                // negation
                                if (cls.size() >= 2 && cls[1] == '!') {
                                        cls[1] = '^';
                                }
                                result += cls;
                                i = end + 1;
                        } else {
                                result += "\\[";
                                i++;
                        }
                        continue;
                }

                // **
                if (c == '*' && i + 1 < pat.size() && pat[i + 1] == '*') {
                        result += ".*";
                        i += 2;
                        continue;
                }

                // *
                if (c == '*') {
                        result += "[^/]*";
                        i++;
                        continue;
                }

                // ?
                if (c == '?') {
                        result += "[^/]";
                        i++;
                        continue;
                }

                // normal chars
                if (isRegexSpecial(c))
                        result.push_back('\\');
                result.push_back(c);
                i++;
        }
        return result;
}

// .gitignore wildcard -> regex
// return: {regex, negated}
std::pair<std::string, bool> wildcard2regex(const std::string &wildcard) {
        std::string pat = wildcard;
        bool negated = false;

        // remove negation
        if (!pat.empty() && pat[0] == '!') {
                negated = true;
                pat = pat.substr(1);
        }

        bool anchored = false;
        if (!pat.empty() && pat[0] == '/') {
                anchored = true;
                pat = pat.substr(1);
        }

        bool hasSlash = (pat.find('/') != std::string::npos);

        if (pat.empty()) {
                return {"^$", negated};
        }

        std::string regexStr;

        if (!anchored && !hasSlash) {
                regexStr = "^(?:.*/)?";
        } else {
                regexStr = "^";
        }

        regexStr += convertPattern(pat);
        regexStr += "$";

        return {regexStr, negated};
}

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
