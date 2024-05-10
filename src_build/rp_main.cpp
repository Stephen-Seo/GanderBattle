#include <iostream>
#include <filesystem>

#include <ResourcePacker.hpp>

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cout << "binary <pack_name> <dir_containing_files>\n";
    return 1;
  }

  const std::filesystem::path res{argv[2]};
  std::list<std::string> files;

  for (auto const & dir_entry : std::filesystem::directory_iterator{res}) {
    if (dir_entry.is_regular_file()) {
      files.push_back(dir_entry.path().string());
      std::cout << "Adding file \"" << files.back() << "\"...\n";
    }
  }

  return RP::createPackfile(files, std::string(argv[1])) ? 0 : 1;
}
