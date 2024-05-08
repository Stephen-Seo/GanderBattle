#include "screen_battle.h"

// Standard library includes.
#include <cmath>
#include <numbers>

#ifndef NDEBUG
#include <iostream>
#endif

BattleScreen::BattleScreen(std::weak_ptr<ScreenStack> stack)
    : Screen(stack), camera_orbit_timer(0.0F) {
  camera.up.x = 0.0F;
  camera.up.y = 1.0F;
  camera.up.z = 0.0F;

  camera.fovy = 45.0F;

  camera.target.x = 0.0F;
  camera.target.y = 0.0F;
  camera.target.z = 0.0F;

  camera.position.x = 0.0F;
  camera.position.y = CAMERA_HEIGHT;
  camera.position.z = CAMERA_ORBIT_XZ;

  camera.projection = CAMERA_PERSPECTIVE;
#ifndef NDEBUG
  std::clog << "Initialized BattleScreen.\n";
#endif
}

bool BattleScreen::update(float dt, bool screen_resized) {
  camera_orbit_timer += dt;
  if (camera_orbit_timer > CAMERA_ORBIT_TIME) {
    camera_orbit_timer -= CAMERA_ORBIT_TIME;
  }

  camera.position.z = std::cos(camera_orbit_timer / CAMERA_ORBIT_TIME *
                               std::numbers::pi_v<float> * 2.0F) *
                      CAMERA_ORBIT_XZ;
  camera.position.x = std::sin(camera_orbit_timer / CAMERA_ORBIT_TIME *
                               std::numbers::pi_v<float> * 2.0F) *
                      CAMERA_ORBIT_XZ;

  return false;
}

bool BattleScreen::draw(RenderTexture *render_texture) {
  BeginTextureMode(*render_texture);
  ClearBackground(BLUE);
  BeginMode3D(camera);

  DrawPlane(Vector3{0.0F, -0.01F, 0.0F}, Vector2{5.0F, 5.0F}, RAYWHITE);
  DrawGrid(20, 0.2F);

  EndMode3D();
  EndTextureMode();
  return false;
}
