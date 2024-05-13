#ifndef SEODISPARATE_COM_GANDER_BATTLE_SCREEN_DEBUG_H_
#define SEODISPARATE_COM_GANDER_BATTLE_SCREEN_DEBUG_H_

// Standard library includes.
#include <deque>
#include <optional>

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

  virtual bool update(float dt, bool screen_resized) override;

  virtual bool draw(RenderTexture *render_texture) override;

  virtual std::list<std::string> get_known_flags() const override;

 private:
  lua_State *lua_state;
  SharedData *shared;
  std::deque<std::string> console;
  std::string console_current;
  std::optional<int> console_x_offset;
};

#endif
