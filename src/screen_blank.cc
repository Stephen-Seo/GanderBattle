#include "screen_blank.h"

// Third-party includes.
#include <raylib.h>

// Local includes.
#include "constants.h"

BlankScreen::BlankScreen(ScreenStack::Weak ss) : Screen(ss) {}

BlankScreen::~BlankScreen() {}

bool BlankScreen::update(float /*dt*/, bool /*screen_resized*/) { return true; }

bool BlankScreen::draw(RenderTexture* render_texture) {
  BeginTextureMode(*render_texture);
  ClearBackground(BLACK);
  DrawText("Blank Screen", 10, SCREEN_HEIGHT / 2, 40, RAYWHITE);
  EndTextureMode();
  return true;
}

std::list<std::string> BlankScreen::get_known_flags() const { return {}; }
