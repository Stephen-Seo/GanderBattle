#include "screen_debug.h"

// Third party includes.
#include <raylib.h>

DebugScreen::DebugScreen(std::weak_ptr<ScreenStack> stack) : Screen(stack) {}

DebugScreen::~DebugScreen() {}

bool DebugScreen::update(float dt, bool screen_resized) { return true; }

bool DebugScreen::draw(RenderTexture *render_texture) {
  BeginTextureMode(*render_texture);
  std::string draw_text = std::to_string(GetFPS());
  DrawText(draw_text.c_str(), 10, 10, 30, RAYWHITE);

  if (stack.lock()->get_shared_data().enable_auto_movement) {
    DrawText("Auto movement enabled.", 10, 600 - 40, 30, RAYWHITE);
  } else {
    DrawText("Auto movement disabled.", 10, 600 - 40, 30, RAYWHITE);
  }

  EndTextureMode();
  return true;
}
