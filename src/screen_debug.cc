#include "screen_debug.h"

#include "lua.h"

// Standard library includes.
#include <cstdlib>
#include <cstring>

// Third party includes.
#include <raylib.h>
// lua
extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

// Local includes.
#include "constants.h"
#include "screen.h"
#include "screen_battle.h"

// #############################################################################
//  BEGIN Lua stuff
// #############################################################################
ScreenStack *get_lua_screen_stack(lua_State *l) {
  // +1
  lua_pushstring(l, "user_stack_ptr");
  // +1, -1
  lua_gettable(l, LUA_REGISTRYINDEX);
  // +0
  void *ptr = lua_touserdata(l, -1);
  // -1
  lua_pop(l, 1);

  return reinterpret_cast<ScreenStack *>(ptr);
}

void *alloc_for_lua(void *ud, void *ptr, std::size_t osize, std::size_t nsize) {
  if (nsize == 0) {
    if (ptr) {
      std::free(ptr);
    }
    return nullptr;
  }

  return std::realloc(ptr, nsize);
}

int lua_reset_stack(lua_State *l) {
  ScreenStack *ss = get_lua_screen_stack(l);
  ss->clear_screens();
  ss->push_constructing_screen<BattleScreen>();
  if (!ss->is_overlay_screen_set()) {
    ss->set_overlay_screen<DebugScreen>();
  }
  ss->get_shared_data().outputs.push_back("Reset Stack.");
  return 0;
}

int lua_clear_stack(lua_State *l) {
  ScreenStack *ss = get_lua_screen_stack(l);
  ss->clear_screens();
  ss->get_shared_data().outputs.push_back("Cleared Stack.");
  return 0;
}

int lua_generic_print(lua_State *l) {
  ScreenStack *ss = get_lua_screen_stack(l);
  std::string output;
  int size = lua_gettop(l);
  for (int idx = 1; idx <= size; ++idx) {
    if (lua_isinteger(l, idx)) {
      auto integer = lua_tointeger(l, idx);
      if (!output.empty()) {
        output.push_back(' ');
      }
      output.append(std::to_string(integer));
    } else if (lua_isstring(l, idx)) {
      const char *s = lua_tostring(l, idx);
      if (!output.empty()) {
        output.push_back(' ');
      }
      output.append(std::string(s));
    } else if (lua_isnumber(l, idx)) {
      auto number = lua_tonumber(l, idx);
      if (!output.empty()) {
        output.push_back(' ');
      }
      output.append(std::to_string(number));
    } else if (lua_isboolean(l, idx)) {
      int b = lua_toboolean(l, idx);
      if (!output.empty()) {
        output.push_back(' ');
      }
      if (b) {
        output.append(std::string("true"));
      } else {
        output.append(std::string("false"));
      }
    } else {
      output.append(std::string("unsupported_type"));
    }
  }
  ss->get_shared_data().outputs.push_back(output);
  return 0;
}
// #############################################################################
//  END Lua stuff
// #############################################################################

DebugScreen::DebugScreen(std::weak_ptr<ScreenStack> stack)
    : Screen(stack),
      lua_state(lua_newstate(alloc_for_lua, stack.lock().get())),
      shared(&stack.lock()->get_shared_data()),
      console_x_offset(0) {
  luaL_requiref(lua_state, "string", luaopen_string, 1);
  luaL_requiref(lua_state, "table", luaopen_table, 1);
  luaL_requiref(lua_state, "math", luaopen_math, 1);

  // Put stack ptr in lua registry table.
  // +1
  lua_pushstring(lua_state, "user_stack_ptr");
  // +1
  lua_pushlightuserdata(lua_state, stack.lock().get());
  // -2
  lua_settable(lua_state, LUA_REGISTRYINDEX);

  // Put "reset_stack()" into global.
  // +1
  lua_pushcfunction(lua_state, lua_reset_stack);
  // -1
  lua_setglobal(lua_state, "reset_stack");

  // Put "clear_stack()" into global.
  // +1
  lua_pushcfunction(lua_state, lua_clear_stack);
  // -1
  lua_setglobal(lua_state, "clear_stack");

  // Put "gen_print()" into global.
  // +1
  lua_pushcfunction(lua_state, lua_generic_print);
  // -1
  lua_setglobal(lua_state, "gen_print");
}

DebugScreen::~DebugScreen() { lua_close(lua_state); }

bool DebugScreen::update(float dt, bool screen_resized) {
  bool just_enabled = false;
  if (IsKeyPressed(KEY_GRAVE)) {
    shared->enable_console = !shared->enable_console;
    just_enabled = shared->enable_console;
  }

  if (shared->enable_console) {
    for (auto output : shared->outputs) {
      console.push_back(output);
    }
    shared->outputs.clear();
    if (IsKeyPressed(KEY_BACKSPACE)) {
      if (!console_current.empty()) {
        console_current.pop_back();
        console_x_offset = std::nullopt;
      }
    } else if (IsKeyPressed(KEY_ENTER)) {
      console.push_back(console_current);

      // +1
      int result = luaL_loadstring(lua_state, console_current.c_str());
      if (result != LUA_OK) {
        console.push_back(std::string(lua_tostring(lua_state, -1)));
        // -1
        lua_pop(lua_state, 1);
      } else {
        // -1, +1 on error.
        result = lua_pcall(lua_state, 0, 0, 0);
        if (result != LUA_OK) {
          console.push_back(std::string(lua_tostring(lua_state, -1)));
          // -1
          lua_pop(lua_state, 1);
        }
      }

      console_current.clear();
      console_x_offset = std::nullopt;
      while (console.size() > 25) {
        // Limit size of history.
        console.pop_front();
      }
    }

    int c;
    do {
      c = GetCharPressed();
      if (just_enabled && c == '`') {
      } else if (c != 0 && c < 128) {
        console_current.push_back((char)c);
        console_x_offset = std::nullopt;
      }
    } while (c != 0);

    if (!console_x_offset.has_value()) {
      auto text_width = MeasureText(console_current.c_str(), 20);
      if (text_width + 12 > SCREEN_WIDTH) {
        console_x_offset = SCREEN_WIDTH - 12 - text_width;
      } else {
        console_x_offset = 0;
      }
    }
  }

  return !shared->enable_console;
}

bool DebugScreen::draw(RenderTexture *render_texture) {
  BeginTextureMode(*render_texture);

  if (shared->enable_console) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 64});

    int offset_y = 24;
    DrawText(">", 5 + console_x_offset.value(), SCREEN_HEIGHT - offset_y, 20,
             RAYWHITE);
    DrawText(console_current.c_str(), 12 + console_x_offset.value(),
             SCREEN_HEIGHT - offset_y, 20, RAYWHITE);
    offset_y += 24;
    for (auto riter = console.crbegin(); riter != console.crend(); ++riter) {
      DrawText(riter->c_str(), 5, SCREEN_HEIGHT - offset_y, 20, RAYWHITE);
      offset_y += 24;
    }
  } else {
    std::string draw_text = std::to_string(GetFPS());
    DrawText(draw_text.c_str(), 10, 10, 30, RAYWHITE);

    if (shared->enable_auto_movement) {
      DrawText("Auto movement enabled.", 10, SCREEN_HEIGHT - 40, 30, RAYWHITE);
    } else {
      DrawText("Auto movement disabled.", 10, SCREEN_HEIGHT - 40, 30, RAYWHITE);
    }
  }

  EndTextureMode();
  return true;
}
