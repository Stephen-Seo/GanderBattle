#include "screen_battle.h"

#include "sc_sacd.h"

// Standard library includes.
#include <cmath>
#include <numbers>

#ifndef NDEBUG
#include <iostream>
#endif

BattleScreen::BattleScreen(std::weak_ptr<ScreenStack> stack)
    : Screen(stack),
      camera_orbit_timer(0.0F),
      sphere{{0.0F, 1.0F, 0.0F, 0.2F}, {1.0F, 1.0F, 1.0F, 0.2F}},
      sphere_vel{{1.105F, 1.0F, 3.3F}, {2.105F, 1.0F, 2.3F}},
      sphere_acc{{0.0F, -SPHERE_DROP_ACC, 0.0F},
                 {0.0F, -SPHERE_DROP_ACC, 0.0F}},
      sphere_touch_point{{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}} {
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

  for (unsigned int idx = 0; idx < 2; ++idx) {
    sphere_vel[idx].x += sphere_acc[idx].x * dt;
    sphere_vel[idx].y += sphere_acc[idx].y * dt;
    sphere_vel[idx].z += sphere_acc[idx].z * dt;

    sphere_prev_pos[idx].x = sphere[idx].x;
    sphere_prev_pos[idx].y = sphere[idx].y;
    sphere_prev_pos[idx].z = sphere[idx].z;

    sphere[idx].x += sphere_vel[idx].x * dt;
    sphere[idx].y += sphere_vel[idx].y * dt;
    sphere[idx].z += sphere_vel[idx].z * dt;
  }

  // Check collision with other.
  if (sphere_collided && !SC_SACD_Sphere_Collision(sphere[0], sphere[1])) {
    sphere_collided = false;
  } else if (SC_SACD_Sphere_Collision(sphere[0], sphere[1])) {
    SC_SACD_Vec3 normal =
        SC_SACD_Vec3{sphere[0].x - sphere[1].x, sphere[0].y - sphere[1].y,
                     sphere[0].z - sphere[1].z};
    float normal_len = SC_SACD_Vec3_Length(normal);
    normal.x /= normal_len;
    normal.y /= normal_len;
    normal.z /= normal_len;

    sphere_vel[0].x = -sphere_vel[0].x;
    sphere_vel[0].z = -sphere_vel[0].z;
    sphere_vel[1].x = -sphere_vel[1].x;
    sphere_vel[1].z = -sphere_vel[1].z;

    sphere[0].x = sphere_prev_pos[0].x;
    sphere[0].y = sphere_prev_pos[0].y;
    sphere[0].z = sphere_prev_pos[0].z;
    sphere[1].x = sphere_prev_pos[1].x;
    sphere[1].y = sphere_prev_pos[1].y;
    sphere[1].z = sphere_prev_pos[1].z;

    // Reflection of velocity over normal.

    float dot_zero = SC_SACD_Dot_Product(normal, sphere_vel[0]);
    float dot_one = SC_SACD_Dot_Product(normal, sphere_vel[1]);

    SC_SACD_Vec3 par_zero = normal;
    par_zero.x *= dot_zero * 2.0F;
    par_zero.y *= dot_zero * 2.0F;
    par_zero.z *= dot_zero * 2.0F;
    SC_SACD_Vec3 par_one = normal;
    par_one.x *= dot_one * 2.0F;
    par_one.y *= dot_one * 2.0F;
    par_one.z *= dot_one * 2.0F;

    sphere_vel[0].x = par_zero.x - sphere_vel[0].x;
    sphere_vel[0].y = par_zero.y - sphere_vel[0].y;
    sphere_vel[0].z = par_zero.z - sphere_vel[0].z;
    sphere_vel[1].x = par_one.x - sphere_vel[1].x;
    sphere_vel[1].y = par_one.y - sphere_vel[1].y;
    sphere_vel[1].z = par_one.z - sphere_vel[1].z;

    sphere_collided = true;
  }

  // Check collision with wall.
  for (unsigned int idx = 0; idx < 2; ++idx) {
    if (sphere[idx].x - sphere[idx].radius < -SPACE_WIDTH) {
      sphere[idx].x = sphere_prev_pos[idx].x;
      sphere_vel[idx].x = std::abs(sphere_vel[idx].x);
    } else if (sphere[idx].x + sphere[idx].radius > SPACE_WIDTH) {
      sphere[idx].x = sphere_prev_pos[idx].x;
      sphere_vel[idx].x = -std::abs(sphere_vel[idx].x);
    }

    if (sphere[idx].y - sphere[idx].radius < 0.0F) {
      sphere_touch_point[idx].x = sphere[idx].x;
      sphere_touch_point[idx].y = sphere_prev_pos[idx].y - sphere[idx].radius;
      sphere_touch_point[idx].z = sphere[idx].z;
      sphere[idx].y = sphere_prev_pos[idx].y;
      sphere_vel[idx].y = std::abs(sphere_vel[idx].y);
    }

    if (sphere[idx].z - sphere[idx].radius < -SPACE_DEPTH) {
      sphere[idx].z = sphere_prev_pos[idx].z;
      sphere_vel[idx].z = std::abs(sphere_vel[idx].z);
    } else if (sphere[idx].z + sphere[idx].radius > SPACE_DEPTH) {
      sphere[idx].z = sphere_prev_pos[idx].z;
      sphere_vel[idx].z = -std::abs(sphere_vel[idx].z);
    }
  }

  return false;
}

bool BattleScreen::draw(RenderTexture *render_texture) {
  BeginTextureMode(*render_texture);
  ClearBackground(BLUE);
  BeginMode3D(camera);

  DrawPlane(Vector3{0.0F, -0.01F, 0.0F},
            Vector2{SPACE_WIDTH * 2.0F, SPACE_DEPTH * 2.0F}, RAYWHITE);
  DrawGrid(20, 0.2F);
  for (unsigned int idx = 0; idx < 2; ++idx) {
    DrawSphere(Vector3{sphere[idx].x, sphere[idx].y, sphere[idx].z},
               sphere[idx].radius, GREEN);
    DrawSphere(Vector3{sphere_touch_point[idx].x, sphere_touch_point[idx].y,
                       sphere_touch_point[idx].z},
               0.02F, RED);
  }

  EndMode3D();
  EndTextureMode();
  return false;
}
