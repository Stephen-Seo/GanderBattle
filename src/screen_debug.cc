#include "screen_debug.h"
#include "duktape.h"

// Standard library includes.
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>

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

using namespace std::string_literals;

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
    if (lua_isnil(l, idx)) {
      if (!output.empty()) {
        output.push_back(' ');
      }
      output.append("nil"s);
    } else if (lua_isinteger(l, idx)) {
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
      output.append(s);
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
        output.append("true"s);
      } else {
        output.append("false"s);
      }
    } else {
      output.append("unsupported_type"s);
    }
  }
  ss->get_shared_data().outputs.push_back(output);
  return 0;
}

int lua_get_flag(lua_State *l) {
  ScreenStack *ss = get_lua_screen_stack(l);

  int top = lua_gettop(l);
  if (top != 1) {
    ss->get_shared_data().outputs.push_back(
        "Not 1 arg! usage: get_flag(\"name\") returns boolean");
    return 0;
  } else if (!lua_isstring(l, 1)) {
    ss->get_shared_data().outputs.push_back(
        "1st arg not string! usage: get_flag(\"name\") returns boolean");
    return 0;
  }
  const char *name = lua_tostring(l, 1);
  auto result = ss->get_shared_data().get_flag(name);

  if (result.has_value() && result.value()) {
    lua_pushboolean(l, 1);
  } else {
    lua_pushboolean(l, 0);
  }
  return 1;
}

int lua_print_flags(lua_State *l) {
  ScreenStack *ss = get_lua_screen_stack(l);

  int top = lua_gettop(l);
  for (int idx = 1; idx <= top; ++idx) {
    if (lua_isstring(l, idx)) {
      const char *l_c_string = lua_tostring(l, idx);
      if (auto opt = ss->get_shared_data().get_flag(l_c_string);
          opt.has_value()) {
        ss->get_shared_data().outputs.push_back(std::format(
            "\"{}\" is {}", l_c_string, (opt.value() ? "true" : "false")));
      } else {
        ss->get_shared_data().outputs.push_back(
            std::format("\"{}\" does not exist.", l_c_string));
      }
    }
  }

  return 0;
}

int lua_set_flag(lua_State *l) {
  ScreenStack *ss = get_lua_screen_stack(l);

  int top = lua_gettop(l);
  if (top != 2) {
    ss->get_shared_data().outputs.push_back(
        "Not 2 args! usage: set_flag(\"name\", boolean) returns prev boolean");
    return 0;
  } else if (!lua_isstring(l, 1)) {
    ss->get_shared_data().outputs.push_back(
        "1st arg not string! usage: set_flag(\"name\", boolean) returns prev "
        "boolean");
    return 0;
  } else if (!lua_isboolean(l, 2)) {
    ss->get_shared_data().outputs.push_back(
        "2nd arg not boolean! usage: set_flag(\"name\", boolean) returns prev "
        "boolean");
    return 0;
  }

  const char *name = lua_tostring(l, 1);
  auto b = lua_toboolean(l, 2);
  auto result = ss->get_shared_data().set_flag_lua(name, b != 0);

  if (!result.has_value()) {
    ss->get_shared_data().outputs.push_back("set_flag(...) invalid name!");
    return 0;
  } else {
    lua_pushboolean(l, result.value() ? 1 : 0);
    return 1;
  }
}

int lua_toggle_flag(lua_State *l) {
  ScreenStack *ss = get_lua_screen_stack(l);

  int top = lua_gettop(l);
  if (top != 1) {
    ss->get_shared_data().outputs.push_back(
        "Not 1 arg! usage: toggle_flag(\"name\") returns boolean");
    return 0;
  } else if (!lua_isstring(l, 1)) {
    ss->get_shared_data().outputs.push_back(
        "1st arg not string! usage: toggle_flag(\"name\") returns boolean");
    return 0;
  }

  const char *name = lua_tostring(l, 1);
  auto result = ss->get_shared_data().toggle_flag_lua(name);

  if (!result.has_value()) {
    ss->get_shared_data().outputs.push_back("toggle_flag(...) invalid name!");
    return 0;
  } else {
    lua_pushboolean(l, result.value() ? 1 : 0);
    return 1;
  }
}

int lua_print_known_flags(lua_State *l) {
  ScreenStack *ss = get_lua_screen_stack(l);

  auto flags = ss->get_known_flags();
  ss->get_shared_data().outputs.push_back("  Known flags:");
  for (const auto &flag : flags) {
    ss->get_shared_data().outputs.push_back(flag);
  }

  return 0;
}

int lua_get_help(lua_State *l) {
  ScreenStack *ss = get_lua_screen_stack(l);

  ss->get_shared_data().outputs.push_back("  Functions:");
  ss->get_shared_data().outputs.push_back("help()");
  ss->get_shared_data().outputs.push_back("print_known_flags()");
  ss->get_shared_data().outputs.push_back("toggle_flag(\"name\")");
  ss->get_shared_data().outputs.push_back("get_flag(\"name\")");
  ss->get_shared_data().outputs.push_back("print_flags(\"name\", ...)");
  ss->get_shared_data().outputs.push_back("set_flag(\"name\", boolean)");
  ss->get_shared_data().outputs.push_back("gen_print(...)");
  ss->get_shared_data().outputs.push_back("reset_stack()");
  ss->get_shared_data().outputs.push_back("clear_stack()");

  return 0;
}
// #############################################################################
//  END Lua stuff
// #############################################################################

// #############################################################################
// BEGIN Duktape/JS stuff
// #############################################################################

constexpr const char *const duktape_hidden_symbol_screen_stack = "\xFFgander_screen_stack";
constexpr const char *const duktape_hidden_symbol_ss_object = "gander_screen_stack_object_d208f8c29833afefe848b0d3e6c40418c5bec16f";

ScreenStack *get_js_screen_stack(duk_context *ctx) {
  // +1
  duk_get_global_string(ctx, duktape_hidden_symbol_ss_object);
  // +1
  duk_get_prop_string(ctx, -1, duktape_hidden_symbol_screen_stack);
  ScreenStack *ss_ptr = reinterpret_cast<ScreenStack*>(duk_get_pointer(ctx, -1));
  // -2
  duk_pop_2(ctx);

  return ss_ptr;
}

// No args.
duk_ret_t js_reset_stack(duk_context *ctx) {
  ScreenStack *ss = get_js_screen_stack(ctx);
  ss->clear_screens();
  ss->push_constructing_screen<BattleScreen>();
  if (!ss->is_overlay_screen_set()) {
    ss->set_overlay_screen<DebugScreen>();
  }
  ss->get_shared_data().outputs.push_back("Reset Stack.");
  return 0;
}

// No args.
duk_ret_t js_clear_stack(duk_context *ctx) {
  ScreenStack *ss = get_js_screen_stack(ctx);
  ss->clear_screens();
  ss->get_shared_data().outputs.push_back("Cleared Stack.");
  return 0;
}

// "N" args.
duk_ret_t js_generic_print(duk_context *ctx) {
  ScreenStack *ss = get_js_screen_stack(ctx);
  std::string output;
  int size = duk_get_top(ctx);
  for (int idx = 0; idx < size; ++idx) {
    if (duk_is_null_or_undefined(ctx, idx)) {
      if (!output.empty()) {
        output.push_back(' ');
      }
      output.append("null/undefined"s);
    } else if (duk_is_number(ctx, idx)) {
      auto number = duk_to_number(ctx, idx);
      if (!output.empty()) {
        output.push_back(' ');
      }
      output.append(std::to_string(number));
    } else if (duk_is_boolean(ctx, idx)) {
      if (!output.empty()) {
        output.push_back(' ');
      }
      output.append(duk_get_boolean(ctx, idx) ? "true"s : "false"s);
    } else if (duk_is_string(ctx, idx)) {
      if (!output.empty()) {
        output.push_back(' ');
      }
      output.append(duk_get_string(ctx, idx));
    } else {
      output.append("unsupported_type"s);
    }
  }
  ss->get_shared_data().outputs.push_back(output);
  return 0;
}

// 1 arg.
duk_ret_t js_get_flag(duk_context *ctx) {
  ScreenStack *ss = get_js_screen_stack(ctx);

  int top = duk_get_top(ctx);
  if (top != 1) {
    ss->get_shared_data().outputs.push_back(
        "Not 1 arg! usage: get_flag(\"name\") returns boolean");
    return 0;
  } else if (!duk_is_string(ctx, 0)) {
    ss->get_shared_data().outputs.push_back(
        "1st arg not string! usage: get_flag(\"name\") returns boolean");
    return 0;
  }
  const char *name = duk_get_string(ctx, 0);
  auto result = ss->get_shared_data().get_flag(name);

  if (result.has_value() && result.value()) {
    duk_push_boolean(ctx, 1);
  } else {
    duk_push_boolean(ctx, 0);
  }
  return 1;
}

duk_ret_t js_print_flags(duk_context *ctx) {
  ScreenStack *ss = get_js_screen_stack(ctx);

  int top = duk_get_top(ctx);
  for (int idx = 0; idx < top; ++idx) {
    if (duk_is_string(ctx, idx)) {
      const char *l_c_string = duk_to_string(ctx, idx);
      if (auto opt = ss->get_shared_data().get_flag(l_c_string);
          opt.has_value()) {
        ss->get_shared_data().outputs.push_back(std::format(
            "\"{}\" is {}", l_c_string, (opt.value() ? "true" : "false")));
      } else {
        ss->get_shared_data().outputs.push_back(
            std::format("\"{}\" does not exist.", l_c_string));
      }
    }
  }

  return 0;
}

// 2 args.
duk_ret_t js_set_flag(duk_context *ctx) {
  ScreenStack *ss = get_js_screen_stack(ctx);

  int top = duk_get_top(ctx);
  if (top != 2) {
    ss->get_shared_data().outputs.push_back(
        "Not 2 args! usage: set_flag(\"name\", boolean) returns prev boolean");
    return 0;
  } else if (!duk_is_string(ctx, 0)) {
    ss->get_shared_data().outputs.push_back(
        "1st arg not string! usage: set_flag(\"name\", boolean) returns prev "
        "boolean");
    return 0;
  } else if (!duk_is_boolean(ctx, 1)) {
    ss->get_shared_data().outputs.push_back(
        "2nd arg not boolean! usage: set_flag(\"name\", boolean) returns prev "
        "boolean");
    return 0;
  }

  const char *name = duk_to_string(ctx, 0);
  auto b = duk_to_boolean(ctx, 1);
  auto result = ss->get_shared_data().set_flag_lua(name, b != 0);

  if (!result.has_value()) {
    ss->get_shared_data().outputs.push_back("set_flag(...) invalid name!");
    return 0;
  } else {
    duk_push_boolean(ctx, result.value() ? 1 : 0);
    return 1;
  }
}

// 1 arg.
duk_ret_t js_toggle_flag(duk_context *ctx) {
  ScreenStack *ss = get_js_screen_stack(ctx);

  int top = duk_get_top(ctx);
  if (top != 1) {
    ss->get_shared_data().outputs.push_back(
        "Not 1 arg! usage: toggle_flag(\"name\") returns boolean");
    return 0;
  } else if (!duk_is_string(ctx, 0)) {
    ss->get_shared_data().outputs.push_back(
        "1st arg not string! usage: toggle_flag(\"name\") returns boolean");
    return 0;
  }

  const char *name = duk_get_string(ctx, 0);
  auto result = ss->get_shared_data().toggle_flag_lua(name);

  if (!result.has_value()) {
    ss->get_shared_data().outputs.push_back("toggle_flag(...) invalid name!");
    return 0;
  } else {
    duk_push_boolean(ctx, result.value() ? 1 : 0);
    return 1;
  }
}

// No args.
duk_ret_t js_print_known_flags(duk_context *ctx) {
  ScreenStack *ss = get_js_screen_stack(ctx);

  auto flags = ss->get_known_flags();
  ss->get_shared_data().outputs.push_back("  Known flags:");
  for (const auto &flag : flags) {
    ss->get_shared_data().outputs.push_back(flag);
  }

  return 0;
}

// No args.
duk_ret_t js_get_help(duk_context *ctx) {
  ScreenStack *ss = get_js_screen_stack(ctx);

  ss->get_shared_data().outputs.push_back("  Functions:");
  ss->get_shared_data().outputs.push_back("help()");
  ss->get_shared_data().outputs.push_back("print_known_flags()");
  ss->get_shared_data().outputs.push_back("toggle_flag(\"name\")");
  ss->get_shared_data().outputs.push_back("get_flag(\"name\")");
  ss->get_shared_data().outputs.push_back("print_flags(\"name\", ...)");
  ss->get_shared_data().outputs.push_back("set_flag(\"name\", boolean)");
  ss->get_shared_data().outputs.push_back("gen_print(...)");
  ss->get_shared_data().outputs.push_back("reset_stack()");
  ss->get_shared_data().outputs.push_back("clear_stack()");

  return 0;
}

// Helper to set screen-stack-ptr and register c_func.
void js_register_c_func(duk_context *ctx, duk_ret_t(*c_func)(duk_context *ctx), duk_idx_t nargs, const char *name) {
  // +1
  duk_push_c_function(ctx, c_func, nargs);
  // -1
  duk_put_global_string(ctx, name);
}

// #############################################################################
// END Duktape/JS stuff
// #############################################################################

DebugScreen::DebugScreen(std::weak_ptr<ScreenStack> stack)
    : Screen(stack),
      embedded_state(),
      flags(),
      shared(&stack.lock()->get_shared_data()),
      console{"Use \"help()\" for available functions."s},
      console_current("> "s),
      console_x_offset(0),
      history_idx(std::nullopt),
      fps_enabled_cache(true) {
  flags.reset(1);

  initialize_lua_state();

  shared->init_flag(enable_fps_flag, true);
}

DebugScreen::~DebugScreen() {
  cleanup_embedded_state();
}

bool DebugScreen::update(float dt, bool screen_resized) {
  bool just_enabled = false;
  if (IsKeyPressed(KEY_GRAVE) && !IsKeyDown(KEY_LEFT_SHIFT) &&
      !IsKeyDown(KEY_RIGHT_SHIFT)) {
    just_enabled = shared->toggle_flag(enable_console_flag);
  }

  if (auto optb = shared->get_flag(enable_console_flag);
      optb.has_value() && optb.value()) {
    for (auto output : shared->outputs) {
      console.push_back(output);
    }
    shared->outputs.clear();
    if (IsKeyPressed(KEY_BACKSPACE)) {
      if (console_current.size() > 2) {
        console_current.pop_back();
        console_x_offset = std::nullopt;
      }
    } else if (IsKeyPressed(KEY_ENTER)) {
      console.push_back(console_current);

      if (console_current.size() > 2) {
        if (history.empty() || history.front() != console_current) {
          history.push_front(console_current);
          while (history.size() > 25) {
            history.pop_back();
          }
        }
        history_idx = std::nullopt;
        if (flags.test(0)) {
          // +1
          int result = luaL_loadstring(get_lua_state(), console_current.c_str() + 2);
          if (result != LUA_OK) {
            console.push_back(lua_tostring(get_lua_state(), -1));
            // -1
            lua_pop(get_lua_state(), 1);
          } else {
            // -1, +1 on error.
            result = lua_pcall(get_lua_state(), 0, 0, 0);
            if (result != LUA_OK) {
              console.push_back(lua_tostring(get_lua_state(), -1));
              // -1
              lua_pop(get_lua_state(), 1);
            }
          }
        } else {
          // +1
          duk_push_string(get_js_state(), console_current.c_str() + 2);
          // +1, -1
          if (duk_peval(get_js_state()) != 0) {
            console.push_back(std::format("{}", duk_safe_to_string(get_js_state(), -1)));
#ifndef NDEBUG
            std::clog << duk_safe_to_string(get_js_state(), -1) << '\n';
#endif
          } else {
/*            console.push_back(std::format("{}", duk_safe_to_string(get_js_state(), -1)));*/
          }
          // -1
          duk_pop(get_js_state());
        }
      } else {
        console.push_back("Empty input."s);
      }

      console_current = "> "s;
      console_x_offset = std::nullopt;
      while (console.size() > 25) {
        // Limit size of history.
        console.pop_front();
      }
    } else if (IsKeyPressed(KEY_UP)) {
      if (history_idx.has_value()) {
        history_idx = history_idx.value() + 1;
        if (history_idx.value() >= history.size()) {
          history_idx = history_idx.value() - 1;
        } else {
          console_current = history[history_idx.value()];
        }
      } else if (!history.empty()) {
        history_idx = 0;
        console_current = history[0];
      }
    } else if (IsKeyPressed(KEY_DOWN)) {
      if (history_idx.has_value() && history_idx.value() > 0) {
        history_idx = history_idx.value() - 1;
        console_current = history[history_idx.value()];
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
      if (text_width + 5 > SCREEN_WIDTH) {
        console_x_offset = SCREEN_WIDTH - 5 - text_width;
      } else {
        console_x_offset = 0;
      }
    }
  }

  // TODO: Maybe set up a way to not get this every frame?
  fps_enabled_cache = shared->get_flag(enable_fps_flag).value();

  {
    auto toggle_embedded = shared->get_flag(toggle_embedded_flag);
    if (toggle_embedded.has_value() && toggle_embedded.value()) {
      shared->set_flag(toggle_embedded_flag, false);
      if (flags.test(0)) {
        initialize_js_state();
        flags.reset(0);
      } else {
        initialize_lua_state();
        flags.set(0);
      }
    }
  }

  auto optb = shared->get_flag(enable_console_flag);
  return !(optb.has_value() && optb.value());
}

bool DebugScreen::draw(RenderTexture *render_texture) {
  BeginTextureMode(*render_texture);

  if (auto optb = shared->get_flag(enable_console_flag);
      optb.has_value() && optb.value()) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{0, 0, 0, 64});

    int offset_y = 24;
    DrawText(console_current.c_str(), 5 + console_x_offset.value(),
             SCREEN_HEIGHT - offset_y, 20, RAYWHITE);
    offset_y += 24;
    for (auto riter = console.crbegin(); riter != console.crend(); ++riter) {
      DrawText(riter->c_str(), 5, SCREEN_HEIGHT - offset_y, 20, RAYWHITE);
      offset_y += 24;
    }
  } else {
    if (fps_enabled_cache) {
      std::string draw_text = std::to_string(GetFPS());
      DrawText(draw_text.c_str(), 10, 10, 30, RAYWHITE);
    }
  }

  EndTextureMode();
  return true;
}

std::list<std::string> DebugScreen::get_known_flags() const {
  return {enable_console_flag, enable_fps_flag, toggle_embedded_flag};
}

void DebugScreen::cleanup_embedded_state() {
  if (flags.test(1)) {
    if (flags.test(0)) {
      lua_close(get_lua_state());
    } else {
      duk_destroy_heap(get_js_state());
    }
    flags.reset(1);
  }
}

void DebugScreen::initialize_lua_state() {
  cleanup_embedded_state();

  embedded_state = lua_newstate(alloc_for_lua, stack.lock().get());
  flags.set(0);

  luaL_requiref(get_lua_state(), "string", luaopen_string, 1);
  luaL_requiref(get_lua_state(), "table", luaopen_table, 1);
  luaL_requiref(get_lua_state(), "math", luaopen_math, 1);

  // Put stack ptr in lua registry table.
  // +1
  lua_pushstring(get_lua_state(), "user_stack_ptr");
  // +1
  lua_pushlightuserdata(get_lua_state(), stack.lock().get());
  // -2
  lua_settable(get_lua_state(), LUA_REGISTRYINDEX);

  // Put "reset_stack()" into global.
  // +1
  lua_pushcfunction(get_lua_state(), lua_reset_stack);
  // -1
  lua_setglobal(get_lua_state(), "reset_stack");

  // Put "clear_stack()" into global.
  // +1
  lua_pushcfunction(get_lua_state(), lua_clear_stack);
  // -1
  lua_setglobal(get_lua_state(), "clear_stack");

  // Put "gen_print()" into global.
  // +1
  lua_pushcfunction(get_lua_state(), lua_generic_print);
  // -1
  lua_setglobal(get_lua_state(), "gen_print");

  // Put flag related fns into global.
  // +1
  lua_pushcfunction(get_lua_state(), lua_get_flag);
  // -1
  lua_setglobal(get_lua_state(), "get_flag");

  // +1
  lua_pushcfunction(get_lua_state(), lua_print_flags);
  // -1
  lua_setglobal(get_lua_state(), "print_flags");

  // +1
  lua_pushcfunction(get_lua_state(), lua_set_flag);
  // -1
  lua_setglobal(get_lua_state(), "set_flag");

  // +1
  lua_pushcfunction(get_lua_state(), lua_toggle_flag);
  // -1
  lua_setglobal(get_lua_state(), "toggle_flag");

  // +1
  lua_pushcfunction(get_lua_state(), lua_print_known_flags);
  // -1
  lua_setglobal(get_lua_state(), "print_known_flags");

  // +1
  lua_pushcfunction(get_lua_state(), lua_get_help);
  // -1
  lua_setglobal(get_lua_state(), "help");

  flags.set(1);

  stack.lock()->get_shared_data().outputs.push_back("Loaded Lua"s);
}

void DebugScreen::initialize_js_state() {
  cleanup_embedded_state();

  embedded_state = duk_create_heap_default();
  flags.reset(0);

  ScreenStack *ss = this->stack.lock().get();

  // Create global object to hold the ScreenStack pointer.
  // +1
  duk_push_object(get_js_state());
  // +1
  duk_push_pointer(get_js_state(), ss);
  // -1
  duk_put_prop_string(get_js_state(), -2, duktape_hidden_symbol_screen_stack);
  // -1
  duk_put_global_string(get_js_state(), duktape_hidden_symbol_ss_object);

  js_register_c_func(get_js_state(), js_reset_stack, 0, "reset_stack");
  js_register_c_func(get_js_state(), js_clear_stack, 0, "clear_stack");
  js_register_c_func(get_js_state(), js_generic_print, DUK_VARARGS, "gen_print");
  js_register_c_func(get_js_state(), js_get_flag, 1, "get_flag");
  js_register_c_func(get_js_state(), js_print_flags, DUK_VARARGS, "print_flags");
  js_register_c_func(get_js_state(), js_set_flag, 2, "set_flag");
  js_register_c_func(get_js_state(), js_toggle_flag, 1, "toggle_flag");
  js_register_c_func(get_js_state(), js_print_known_flags, 0, "print_known_flags");
  js_register_c_func(get_js_state(), js_get_help, 0, "help");

  flags.set(1);

  stack.lock()->get_shared_data().outputs.push_back("Loaded Duktape"s);
}

lua_State *DebugScreen::get_lua_state() {
  if (flags.test(0)) {
    return std::get<lua_State*>(embedded_state);
  }
  return nullptr;
}

duk_context *DebugScreen::get_js_state() {
  if (!flags.test(0)) {
    return std::get<duk_context*>(embedded_state);
  }
  return nullptr;
}
