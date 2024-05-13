#include "shared_data.h"

SharedData::SharedData() : outputs(), flags() {}

std::optional<bool> SharedData::set_flag(std::string name, bool value) {
  if (auto iter = flags.find(name); iter != flags.end()) {
    bool prev = iter->second;
    iter->second = value;
    return prev;
  } else {
    flags.insert(std::make_pair(name, value));
    return std::nullopt;
  }
}

std::optional<bool> SharedData::unset_flag(std::string name) {
  if (auto iter = flags.find(name); iter != flags.end()) {
    bool prev = iter->second;
    flags.erase(iter);
    return prev;
  } else {
    return std::nullopt;
  }
}

std::optional<bool> SharedData::get_flag(std::string name) {
  if (auto iter = flags.find(name); iter != flags.end()) {
    return iter->second;
  } else {
    return std::nullopt;
  }
}

bool SharedData::toggle_flag(std::string name) {
  if (auto iter = flags.find(name); iter != flags.end()) {
    iter->second = !iter->second;
    return iter->second;
  } else {
    flags.insert(std::make_pair(name, true));
    return true;
  }
}

std::optional<bool> SharedData::set_flag_lua(std::string name, bool value) {
  if (auto iter = flags.find(name); iter != flags.end()) {
    bool prev = iter->second;
    iter->second = value;
    return prev;
  } else {
    return std::nullopt;
  }
}

std::optional<bool> SharedData::toggle_flag_lua(std::string name) {
  if (auto iter = flags.find(name); iter != flags.end()) {
    iter->second = !iter->second;
    return iter->second;
  } else {
    return std::nullopt;
  }
}
