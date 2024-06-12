#ifndef SEODISPARATE_COM_GANDER_BATTLE_SCREEN_DEBUG_H_
#define SEODISPARATE_COM_GANDER_BATTLE_SCREEN_DEBUG_H_

// Standard library includes.
#include <bitset>
#include <deque>
#include <optional>
#include <variant>

// Third-party includes.

// lua
extern "C" {
#include "lua.h"
}

#include <duktape.h>

// Local includes.
#include "screen.h"

class DebugScreen : public Screen {
 public:
  DebugScreen(std::weak_ptr<ScreenStack> stack);
  virtual ~DebugScreen();

  virtual bool update(float dt, bool screen_resized) override;

  virtual bool draw(RenderTexture* render_texture) override;

  virtual std::list<std::string> get_known_flags() const override;

 private:
  void cleanup_embedded_state();
  void initialize_lua_state();
  void initialize_js_state();

  lua_State* get_lua_state();
  duk_context* get_js_state();

  std::variant<lua_State*, duk_context*> embedded_state;
  /*
   * 0 - If set, using lua. If unset, using javascript.
   * 1 - If set, the embedded state has been previously initialized.
   */
  std::bitset<32> flags;
  SharedData* shared;
  std::deque<std::string> console;
  std::deque<std::string> history;
  std::string console_current;
  std::optional<int> console_x_offset;
  std::optional<unsigned int> history_idx;
  bool fps_enabled_cache;
};

#endif
