// Emscripten includes.
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>

#include "ems.h"
#endif

// Third party includes.
#include <raylib.h>

// Local includes.
#include "constants.h"
#include "screen.h"
#include "screen_battle.h"
#include "screen_debug.h"

#ifdef __EMSCRIPTEN__
ScreenStack *global_screen_stack_ptr = nullptr;

extern "C" {
EM_BOOL resize_event_callback(int event_type, const EmscriptenUiEvent *event,
                              void *ud) {
  if (event_type == EMSCRIPTEN_EVENT_RESIZE) {
    SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
  }
  return false;
}

void main_loop_update(void *ud) {
  global_screen_stack_ptr->update(GetFrameTime());
  global_screen_stack_ptr->draw();
}
}
#endif

int main(int argc, char **argv) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Gander Battle");

#ifdef NDEBUG
  SetTraceLogLevel(LOG_WARNING);
#endif

#ifdef __EMSCRIPTEN__
  auto stack = ScreenStack::new_instance();
  stack->push_constructing_screen<BattleScreen>();
  stack->set_overlay_screen<DebugScreen>();
  global_screen_stack_ptr = stack.get();
  SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, false,
                                 resize_event_callback);
  emscripten_set_main_loop_arg(main_loop_update, nullptr, 0, 1);
#else

  SetTargetFPS(60);

  {
    auto stack = ScreenStack::new_instance();
    stack->push_constructing_screen<BattleScreen>();
    stack->set_overlay_screen<DebugScreen>();

    while (!WindowShouldClose()) {
      stack->update(GetFrameTime());
      stack->draw();
    }
  }
#endif

  CloseAudioDevice();
  CloseWindow();

  return 0;
}
