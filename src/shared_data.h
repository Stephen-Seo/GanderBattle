#ifndef SEODISPARATE_COM_GANDER_BATTLE_SHARED_DATA_H_
#define SEODISPARATE_COM_GANDER_BATTLE_SHARED_DATA_H_

#include <string>
#include <vector>

class SharedData {
 public:
  SharedData();

  std::vector<std::string> outputs;
  bool enable_auto_movement;
  bool enable_console;
};

#endif
