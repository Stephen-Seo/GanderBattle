#include "screen_battle.h"

#include "sc_sacd.h"

// Standard library includes.
#include <cmath>
#include <numbers>

#ifndef NDEBUG
#include <iostream>
#endif

// Local includes.
#include "constants.h"
#include "ems.h"
#include "resource_handler.h"

static const char *BATTLE_SCREEN_GROUND_SHADER_VS =
    // Default vertex shader from Raylib.
    "#version 100                       \n"
    "precision mediump float;           \n"  // Precision required for OpenGL
                                             // ES2 (WebGL) (on some browsers)
    "attribute vec3 vertexPosition;     \n"
    "attribute vec2 vertexTexCoord;     \n"
    "attribute vec4 vertexColor;        \n"
    "varying vec2 fragTexCoord;         \n"
    "varying vec4 fragColor;            \n"
    "uniform mat4 mvp;                  \n"
    "void main()                        \n"
    "{                                  \n"
    "    fragTexCoord = vertexTexCoord; \n"
    "    fragColor = vertexColor;       \n"
    "    gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
    "}                                  \n";
;
static const char *BATTLE_SCREEN_GROUND_SHADER_FS =
    "#version 100\n"
    "precision mediump float;\n"
    "// Input vertex attributes (from vertex shader)\n"
    "varying vec2 fragTexCoord;\n"
    "varying vec4 fragColor;\n"
    "// Input uniform values\n"
    "uniform sampler2D texture0;\n"
    "uniform vec4 colDiffuse;\n"
    "uniform float ground_scale;\n"
    "uniform vec2 pos;\n"
    "uniform vec2 other_pos;\n"
    "uniform float radius;\n"
    "uniform float ground_size;\n"
    "void main() {\n"
    // Scale by ground_scale. Smaller the scale, the larger the texture.
    "vec2 offset = (fragTexCoord + pos / ground_size) * ground_scale;\n"
    // Ensure texture wraps-around.
    "if (offset.x < 0.0) {\n"
    "  offset.x = offset.x + floor(abs(offset.x) + 1.0);\n"
    "} else if (offset.x > 1.0) {\n"
    "  offset.x = offset.x - floor(offset.x);\n"
    "}\n"
    "if (offset.y < 0.0) {\n"
    "  offset.y = offset.y + floor(abs(offset.y) + 1.0);\n"
    "} else if (offset.y > 1.0) {\n"
    "  offset.y = offset.y - floor(offset.y);\n"
    "}\n"
    "vec4 texelColor = texture2D(texture0, offset)*colDiffuse*fragColor;\n"
    // Ensure no color values below 0.3 .
    "texelColor *= 0.7;\n"
    "texelColor += vec4(0.3, 0.3, 0.3, 0.3);\n"
    // Ensure a "circle" of the ground is visible.
    "vec2 diff = fragTexCoord - vec2(0.5, 0.5);\n"
    "float diff_length = length(diff);\n"
    "vec2 pos_diff = (other_pos - pos) / ground_size;\n"
    "if (diff_length > 0.45) {\n"
    "  texelColor.a = 0.0;\n"
    "} else if (diff_length > 0.35) {\n"
    "  if (distance(diff, pos_diff) > 0.35) {\n"
    "    float value = 1.0 - (diff_length - 0.35) * 10.0;\n"
    "    texelColor.r = texelColor.r * value;\n"
    "    texelColor.g = texelColor.g * value;\n"
    "    texelColor.b = texelColor.b * value;\n"
    "    texelColor.a = texelColor.a * value;\n"
    "  }\n"
    "}\n"
    "gl_FragColor = texelColor;\n"
    "}\n";

BattleScreen::BattleScreen(std::weak_ptr<ScreenStack> stack)
    : Screen(stack),
      camera_orbit_timer(0.0F),
      sphere{{-1.0F, 0.21F, 0.0F, 0.2F}, {0.0F, 0.21F, 0.0F, 0.2F}},
      sphere_vel{{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
      sphere_acc{{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
      sphere_touch_point{{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F, 0.0F}},
      floor_box{0.0F, -1.0F, 0.0F, 10.0F, 2.0F, 10.0F},
      ground_pos{0.0F, 0.0F, 0.0F, 0.0F},
      prev_auto_move_flag_value(false) {
  camera.up.x = 0.0F;
  camera.up.y = 1.0F;
  camera.up.z = 0.0F;

  camera.fovy = 45.0F;

  camera.target.x = 0.0F;
  camera.target.y = 0.0F;
  camera.target.z = 0.0F;

  camera.position.x = CAMERA_ORBIT_XZ;
  camera.position.y = CAMERA_HEIGHT;
  camera.position.z = CAMERA_ORBIT_XZ;

  camera.projection = CAMERA_PERSPECTIVE;

  ground_model = LoadModelFromMesh(
      GenMeshPlane(GROUND_PLANE_SIZE, GROUND_PLANE_SIZE, 1, 1));

  {
    auto blue_noise_data = ResourceHandler::load("res/blue_noise_256x256.png");

    if (blue_noise_data.size() != 0) {
      auto image = LoadImageFromMemory(
          ".png", (const unsigned char *)blue_noise_data.data(),
          (int)blue_noise_data.size());
      ground_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
          LoadTextureFromImage(image);
      UnloadImage(image);
    }
  }

  ground_shader = LoadShaderFromMemory(BATTLE_SCREEN_GROUND_SHADER_VS,
                                       BATTLE_SCREEN_GROUND_SHADER_FS);
  ground_shader_scale_idx = GetShaderLocation(ground_shader, "ground_scale");
  ground_scale = SHADER_GROUND_SCALE;
  SetShaderValue(ground_shader, ground_shader_scale_idx, &ground_scale,
                 SHADER_UNIFORM_FLOAT);

  ground_shader_pos_idx = GetShaderLocation(ground_shader, "pos");
  SetShaderValue(ground_shader, ground_shader_pos_idx, ground_pos,
                 SHADER_UNIFORM_VEC2);

  ground_shader_other_pos_idx = GetShaderLocation(ground_shader, "other_pos");
  SetShaderValue(ground_shader, ground_shader_pos_idx, ground_pos + 2,
                 SHADER_UNIFORM_VEC2);

  ground_shader_radius_idx = GetShaderLocation(ground_shader, "radius");
  float temp = GROUND_PLANE_SIZE_F / 2.2F;
  SetShaderValue(ground_shader, ground_shader_radius_idx, &temp,
                 SHADER_UNIFORM_FLOAT);

  ground_shader_ground_size_idx =
      GetShaderLocation(ground_shader, "ground_size");
  temp = GROUND_PLANE_SIZE_F;
  SetShaderValue(ground_shader, ground_shader_ground_size_idx, &temp,
                 SHADER_UNIFORM_FLOAT);

  ground_model.materials[0].shader = ground_shader;

#ifndef NDEBUG
  TraceLog(LOG_INFO, "Shader is ready: %s",
           (IsShaderReady(ground_shader) ? "yes" : "no"));
  TraceLog(LOG_INFO, "Initialized BattleScreen.");
#endif
}

BattleScreen::~BattleScreen() {
  UnloadModel(ground_model);
  UnloadShader(ground_shader);
}

bool BattleScreen::update(float dt, bool screen_resized) {
  auto &shared_data = stack.lock()->get_shared_data();
  if (auto flag_opt = shared_data.get_flag(enable_auto_move_flag);
      flag_opt.has_value() && prev_auto_move_flag_value != flag_opt.value()) {
    prev_auto_move_flag_value = flag_opt.value();
    if (flag_opt.value()) {
      sphere_vel[0].x =
          (call_js_get_random() - 0.5F) * 2.0F * AUTOMOVE_DIR_VAR_MAX;
      sphere_vel[0].y =
          (call_js_get_random() - 0.5F) * 2.0F * AUTOMOVE_DIR_VAR_MAX;
      sphere_vel[0].z =
          (call_js_get_random() - 0.5F) * 2.0F * AUTOMOVE_DIR_VAR_MAX;

      sphere_vel[0] = SC_SACD_Vec3_Mult(SC_SACD_Vec3_Normalize(sphere_vel[0]),
                                        AUTOMOVE_SPEED);

      sphere_vel[1].x =
          (call_js_get_random() - 0.5F) * 2.0F * AUTOMOVE_DIR_VAR_MAX;
      sphere_vel[1].y =
          (call_js_get_random() - 0.5F) * 2.0F * AUTOMOVE_DIR_VAR_MAX;
      sphere_vel[1].z =
          (call_js_get_random() - 0.5F) * 2.0F * AUTOMOVE_DIR_VAR_MAX;

      sphere_vel[1] = SC_SACD_Vec3_Mult(SC_SACD_Vec3_Normalize(sphere_vel[1]),
                                        AUTOMOVE_SPEED);

      sphere_acc[0].y = -SPHERE_DROP_ACC;
      sphere_acc[1].y = -SPHERE_DROP_ACC;
      sphere[0].y = 1.0F;
      sphere[1].y = 1.0F;
    } else {
      sphere_acc[0].x = 0.0F;
      sphere_acc[0].y = 0.0F;
      sphere_acc[0].z = 0.0F;
      sphere_acc[1].x = 0.0F;
      sphere_acc[1].y = 0.0F;
      sphere_acc[1].z = 0.0F;

      sphere_vel[0].x = 0.0F;
      sphere_vel[0].y = 0.0F;
      sphere_vel[0].z = 0.0F;
      sphere_vel[1].x = 0.0F;
      sphere_vel[1].y = 0.0F;
      sphere_vel[1].z = 0.0F;

      sphere[0].y = 0.21F;
      sphere[1].y = 0.21F;
    }
  }

  if (auto flag_opt = shared_data.get_flag(enable_auto_move_flag);
      !flag_opt.has_value() || !flag_opt.value()) {
    if (IsKeyDown(KEY_D)) {
      sphere_vel[0].x = MOVEMENT_SPEED;
    } else if (IsKeyDown(KEY_A)) {
      sphere_vel[0].x = -MOVEMENT_SPEED;
    } else {
      sphere_vel[0].x = 0.0F;
    }

    if (IsKeyDown(KEY_W)) {
      sphere_vel[0].z = -MOVEMENT_SPEED;
    } else if (IsKeyDown(KEY_S)) {
      sphere_vel[0].z = MOVEMENT_SPEED;
    } else {
      sphere_vel[0].z = 0.0F;
    }

    if (IsKeyDown(KEY_RIGHT)) {
      sphere_vel[1].x = MOVEMENT_SPEED;
    } else if (IsKeyDown(KEY_LEFT)) {
      sphere_vel[1].x = -MOVEMENT_SPEED;
    } else {
      sphere_vel[1].x = 0.0F;
    }

    if (IsKeyDown(KEY_UP)) {
      sphere_vel[1].z = -MOVEMENT_SPEED;
    } else if (IsKeyDown(KEY_DOWN)) {
      sphere_vel[1].z = MOVEMENT_SPEED;
    } else {
      sphere_vel[1].z = 0.0F;
    }
  }
  // camera_orbit_timer += dt;
  // if (camera_orbit_timer > CAMERA_ORBIT_TIME) {
  //   camera_orbit_timer -= CAMERA_ORBIT_TIME;
  // }

  floor_timer += dt;

  /*  camera.position.z = std::cos(camera_orbit_timer / CAMERA_ORBIT_TIME **/
  /*                               std::numbers::pi_v<float> * 2.0F) **/
  /*                      CAMERA_ORBIT_XZ;*/
  /*  camera.position.x = std::sin(camera_orbit_timer / CAMERA_ORBIT_TIME **/
  /*                               std::numbers::pi_v<float> * 2.0F) **/
  /*                      CAMERA_ORBIT_XZ;*/

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

  if (auto flag_opt = shared_data.get_flag(enable_auto_move_flag);
      flag_opt.has_value() && flag_opt.value()) {
    // Check collision with other.
    bool collided = SC_SACD_Sphere_Collision(sphere[0], sphere[1]) != 0;
    if (sphere_collided && !collided) {
      sphere_collided = false;
    } else if (collided) {
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

      float dot_product[2] = {
          SC_SACD_Dot_Product(normal, sphere_vel[0]) / temp,
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

    if (sphere[idx].z - sphere[idx].radius < -SPACE_DEPTH) {
      sphere[idx].z = sphere_prev_pos[idx].z;
      sphere_vel[idx].z = std::abs(sphere_vel[idx].z);
    } else if (sphere[idx].z + sphere[idx].radius > SPACE_DEPTH) {
      sphere[idx].z = sphere_prev_pos[idx].z;
      sphere_vel[idx].z = -std::abs(sphere_vel[idx].z);
    }

    // Check collision with ground.
    /*    if (sphere[idx].y - sphere[idx].radius < 0.0F) {*/
    if (SC_SACD_Sphere_AABB_Box_Collision(sphere[idx], floor_box)) {
      sphere_touch_point[idx].x = sphere[idx].x;
      sphere_touch_point[idx].y = sphere_prev_pos[idx].y - sphere[idx].radius;
      sphere_touch_point[idx].z = sphere[idx].z;
      sphere[idx].y = sphere_prev_pos[idx].y;
      sphere_vel[idx].y = std::abs(sphere_vel[idx].y);
      floor_timer = 0.0F;
    }
  }

  ground_pos[0] = sphere[0].x;
  ground_pos[1] = sphere[0].z;
  ground_pos[2] = sphere[1].x;
  ground_pos[3] = sphere[1].z;

  // TODO DEBUG
  camera.target.x = sphere[0].x;
  camera.position.x = sphere[0].x;
  camera.target.z = sphere[0].z;
  camera.position.z = sphere[0].z + CAMERA_ORBIT_XZ;

  return false;
}

bool BattleScreen::draw(RenderTexture *render_texture) {
  BeginTextureMode(*render_texture);
  ClearBackground(Color{0, 64, 0, 255});
  BeginMode3D(camera);

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

  SetShaderValue(ground_shader, ground_shader_pos_idx, ground_pos,
                 SHADER_UNIFORM_VEC2);
  SetShaderValue(ground_shader, ground_shader_other_pos_idx, ground_pos + 2,
                 SHADER_UNIFORM_VEC2);
  DrawModel(ground_model, Vector3{sphere[0].x, -0.0101F, sphere[0].z}, 1.0F,
            Color{0, 128, 0, 255});

  SetShaderValue(ground_shader, ground_shader_pos_idx, ground_pos + 2,
                 SHADER_UNIFORM_VEC2);
  SetShaderValue(ground_shader, ground_shader_other_pos_idx, ground_pos,
                 SHADER_UNIFORM_VEC2);
  DrawModel(ground_model, Vector3{sphere[1].x, -0.01F, sphere[1].z}, 1.0F,
            Color{0, 128, 0, 255});

  EndMode3D();
  EndTextureMode();
  return true;
}

std::list<std::string> BattleScreen::get_known_flags() const {
  return {enable_auto_move_flag};
}
