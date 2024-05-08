// Local includes.
#include "screen.h"
#include <raylib.h>

int main(int argc, char **argv) {
  InitWindow(800, 600, "Gander Battle");
  SetTargetFPS(60);

  {
    auto stack = ScreenStack::new_instance();

    while(!WindowShouldClose()) {
      stack->update(GetFrameTime());
      stack->draw();
    }
  }

  CloseWindow();

  return 0;
}
