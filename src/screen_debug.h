#ifndef SEODISPARATE_COM_GANDER_BATTLE_SCREEN_DEBUG_H_
#define SEODISPARATE_COM_GANDER_BATTLE_SCREEN_DEBUG_H_

// Third-party includes.

// lua
extern "C" {
#include "lua.h"
}

// Local includes.
#include "screen.h"

class DebugScreen : public Screen {
 public:
  DebugScreen(std::weak_ptr<ScreenStack> stack);
  virtual ~DebugScreen();

  virtual bool update(float dt, bool screen_resized);

  virtual bool draw(RenderTexture *render_texture);

 private:
  lua_State *lua_state;
  SharedData *shared;
  std::vector<std::string> console;
  std::string console_current;
};

#endif
