#ifndef SEODISPARATE_COM_GANDER_BATTLE_SHARED_DATA_H_
#define SEODISPARATE_COM_GANDER_BATTLE_SHARED_DATA_H_

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class SharedData {
 public:
  SharedData();

  /// Returns prev value.
  std::optional<bool> set_flag(std::string name, bool value);
  /// Returns prev value.
  std::optional<bool> unset_flag(std::string name);
  /// Returns value.
  std::optional<bool> get_flag(std::string name);

  /// Returns value.
  bool toggle_flag(std::string name);

  /// Returns prev value.
  std::optional<bool> set_flag_lua(std::string name, bool value);
  /// Returns value.
  std::optional<bool> toggle_flag_lua(std::string name);

  std::vector<std::string> outputs;
  std::unordered_map<std::string, bool> flags;
};

#endif
