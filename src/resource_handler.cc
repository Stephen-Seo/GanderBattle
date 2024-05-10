#include "resource_handler.h"

// Standard library includes.
#include <cstring>
#include <filesystem>
#include <fstream>

// Third-party includes.
#ifndef __EMSCRIPTEN__
#ifdef SEODISPARATE_RESOURCE_PACKER_AVAILABLE
#include <ResourcePacker.hpp>
#endif
#endif
#include <raylib.h>

std::vector<char> ResourceHandler::load(const char *filename) {
  std::vector<char> data;
  do {
#ifndef NDEBUG
    TraceLog(LOG_INFO, "Attempting to load \"%s\"...", filename);
#endif
    std::ifstream ifs(filename);
    if (!ifs.good()) {
      break;
    }

    ifs.seekg(0, std::ios_base::end);
    auto size = ifs.tellg();
    data.resize((unsigned long)size);

    ifs.seekg(0);
    ifs.read(data.data(), size);
    if (!ifs.fail()) {
#ifndef NDEBUG
      TraceLog(LOG_INFO, "Loaded \"%s\".", filename);
#endif
      return data;
    }
  } while (false);

#ifndef __EMSCRIPTEN__
#ifdef SEODISPARATE_RESOURCE_PACKER_AVAILABLE
#ifndef NDEBUG
  TraceLog(LOG_INFO, "Attempting to load \"%s\" from packfile...", filename);
#endif
  if (RP::checkIfPackfile("data")) {
    std::unique_ptr<char[]> data_ptr{};
    std::uint64_t size;
    if (RP::getFileData(data_ptr, size, std::string("data"),
                        std::filesystem::path(filename).filename().string())) {
      data.resize(size);
      std::memcpy(data.data(), data_ptr.get(), size);
#ifndef NDEBUG
      TraceLog(LOG_INFO, "Loaded \"%s\" from packfile.", filename);
#endif
      return data;
    }
  }
#endif  // RESOURCE_PACKER_AVAILABLE
#endif  // Not __EMSCRIPTEN__

  TraceLog(LOG_WARNING, "Failed to load resource: %s", filename);

  return data;
}
