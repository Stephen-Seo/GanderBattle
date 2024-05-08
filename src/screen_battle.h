#ifndef SEODISPARATE_COM_GANDER_BATTLE_SCREEN_BATTLE_H_
#define SEODISPARATE_COM_GANDER_BATTLE_SCREEN_BATTLE_H_

#include "screen.h"

// Third party includes.
#include <raylib.h>

// Constants.
constexpr float CAMERA_ORBIT_TIME = 9.0F;
constexpr float CAMERA_HEIGHT = 3.0F;
constexpr float CAMERA_ORBIT_XZ = 5.0F;

class BattleScreen : public Screen {
 public:
  BattleScreen(std::weak_ptr<ScreenStack> stack);

  virtual bool update(float dt, bool screen_resized);

  virtual bool draw(RenderTexture* render_texture);

 private:
  Camera3D camera;
  float camera_orbit_timer;
};

#endif
