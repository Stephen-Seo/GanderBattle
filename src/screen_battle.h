#ifndef SEODISPARATE_COM_GANDER_BATTLE_SCREEN_BATTLE_H_
#define SEODISPARATE_COM_GANDER_BATTLE_SCREEN_BATTLE_H_

#include "screen.h"

// Third party includes.
#include <raylib.h>
#include <sc_sacd.h>

// Constants.
constexpr float CAMERA_ORBIT_TIME = 20.0F;
constexpr float CAMERA_HEIGHT = 3.0F;
constexpr float CAMERA_ORBIT_XZ = 5.0F;

constexpr float SPHERE_DROP_ACC = 9.8F;

constexpr float SPACE_WIDTH = 2.5F;
constexpr float SPACE_DEPTH = 2.5F;

class BattleScreen : public Screen {
 public:
  BattleScreen(std::weak_ptr<ScreenStack> stack);

  virtual bool update(float dt, bool screen_resized);

  virtual bool draw(RenderTexture* render_texture);

 private:
  Camera3D camera;
  float camera_orbit_timer;
  SC_SACD_Sphere sphere[2];
  SC_SACD_Vec3 sphere_vel[2];
  SC_SACD_Vec3 sphere_acc[2];
  SC_SACD_Vec3 sphere_touch_point[2];
  SC_SACD_Vec3 sphere_prev_pos[2];
  bool sphere_collided;
};

#endif
