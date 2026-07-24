#include <filesystem>

namespace fs = std::filesystem;

fs::path get_data_dir();
fs::path get_cache_dir();

namespace package {

void pack();

void install_file(const std::string &path, bool force);
void install_git(const std::string &repo_url, bool force);

void remove(const std::string &package, bool force);

void reset(const std::string &package);

void list();
}  // namespace package
