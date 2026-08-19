#include <filesystem>

namespace fs = std::filesystem;

namespace saves {

std::string save(const json &save_data);
std::string save_curr();
json load(const fs::path path);
void apply_save(const json save);
json get_saves();
bool saves_tui();
void sync_with_plugins(json &saves);

// may other files need that helper
std::string get_formatted_date(const uint64_t timestamp);
}  // namespace saves
