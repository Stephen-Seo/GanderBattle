#include "screen_debug.h"

// Third party includes.
#include <raylib.h>

DebugScreen::DebugScreen(std::weak_ptr<ScreenStack> stack) : Screen(stack) {}

DebugScreen::~DebugScreen() {}

bool DebugScreen::update(float dt, bool screen_resized) { return false; }

bool DebugScreen::draw(RenderTexture *render_texture) {
  BeginTextureMode(*render_texture);
  std::string draw_text = std::to_string(GetFPS());
  ClearBackground(BLACK);
  DrawText(draw_text.c_str(), 10, 10, 30, RAYWHITE);
  EndTextureMode();
  return false;
}
