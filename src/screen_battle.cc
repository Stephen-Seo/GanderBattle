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
      sphere_touch_point{{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
      floor_box{0.0F, -1.0F, 0.0F, 10.0F, 2.0F, 10.0F} {
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

  floor_timer += dt;

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
    SC_SACD_Vec3 normal{sphere[0].x - sphere[1].x, sphere[0].y - sphere[1].y,
                        sphere[0].z - sphere[1].z};

    // Move spheres to point before collision.

    sphere[0].x = sphere_prev_pos[0].x;
    sphere[0].y = sphere_prev_pos[0].y;
    sphere[0].z = sphere_prev_pos[0].z;
    sphere[1].x = sphere_prev_pos[1].x;
    sphere[1].y = sphere_prev_pos[1].y;
    sphere[1].z = sphere_prev_pos[1].z;

    // Get projection onto normal.

    float temp = SC_SACD_Dot_Product(normal, normal);

    float dot_product[2] = {SC_SACD_Dot_Product(normal, sphere_vel[0]) / temp,
                            SC_SACD_Dot_Product(normal, sphere_vel[1]) / temp};
    SC_SACD_Vec3 proj[2] = {
        SC_SACD_Vec3{dot_product[0] * normal.x, dot_product[0] * normal.y,
                     dot_product[0] * normal.z},
        SC_SACD_Vec3{dot_product[1] * normal.x, dot_product[1] * normal.y,
                     dot_product[1] * normal.z}};

    // Get reflection over normal, and negate it to get desired result.

    sphere_vel[0].x = -(proj[0].x * 2.0F - sphere_vel[0].x);
    sphere_vel[0].y = -(proj[0].y * 2.0F - sphere_vel[0].y);
    sphere_vel[0].z = -(proj[0].z * 2.0F - sphere_vel[0].z);
    sphere_vel[1].x = -(proj[1].x * 2.0F - sphere_vel[1].x);
    sphere_vel[1].y = -(proj[1].y * 2.0F - sphere_vel[1].y);
    sphere_vel[1].z = -(proj[1].z * 2.0F - sphere_vel[1].z);

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

    if (SC_SACD_Sphere_AABB_Box_Collision(sphere[idx], floor_box)) {
      sphere_touch_point[idx].x = sphere[idx].x;
      sphere_touch_point[idx].y = sphere_prev_pos[idx].y - sphere[idx].radius;
      sphere_touch_point[idx].z = sphere[idx].z;
      sphere[idx].y = sphere_prev_pos[idx].y;
      sphere_vel[idx].y = std::abs(sphere_vel[idx].y);
      floor_timer = 0.0F;
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

  unsigned char floor_red;
  if (floor_timer >= 0 && floor_timer < FLOOR_TIME_MAX) {
    floor_red = (unsigned char)(floor_timer / FLOOR_TIME_MAX * 255.0F);
  } else {
    floor_red = 255;
  }
  DrawPlane(Vector3{0.0F, -0.01F, 0.0F},
            Vector2{SPACE_WIDTH * 2.0F, SPACE_DEPTH * 2.0F},
            Color{floor_red, 255, 255, 255});
  DrawGrid(20, 0.2F);
  DrawSphere(Vector3{sphere[0].x, sphere[0].y, sphere[0].z}, sphere[0].radius,
             GREEN);
  DrawSphere(Vector3{sphere[1].x, sphere[1].y, sphere[1].z}, sphere[1].radius,
             RED);
  for (unsigned int idx = 0; idx < 2; ++idx) {
    DrawSphere(Vector3{sphere_touch_point[idx].x, sphere_touch_point[idx].y,
                       sphere_touch_point[idx].z},
               0.02F, RED);
  }

  EndMode3D();
  EndTextureMode();
  return false;
}
