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

constexpr float FLOOR_TIME_MAX = 1.0F;

constexpr float SHADER_GROUND_SCALE = 0.1F;
constexpr int GROUND_PLANE_SIZE = 5;
constexpr float GROUND_PLANE_SIZE_F = (float)GROUND_PLANE_SIZE;

constexpr float MOVEMENT_SPEED = 1.0F;
constexpr float AUTOMOVE_DIR_VAR_MAX = 2.0F;
constexpr float AUTOMOVE_SPEED = 3.0F;

class BattleScreen : public Screen {
 public:
  BattleScreen(std::weak_ptr<ScreenStack> stack);
  virtual ~BattleScreen();

  virtual bool update(float dt, bool screen_resized) override;

  virtual bool draw(RenderTexture* render_texture) override;

  virtual std::list<std::string> get_known_flags() const override;

 private:
  Camera3D camera;
  float camera_orbit_timer;
  float floor_timer;
  SC_SACD_Sphere sphere[2];
  SC_SACD_Vec3 sphere_vel[2];
  SC_SACD_Vec3 sphere_acc[2];
  SC_SACD_Vec3 sphere_touch_point[2];
  SC_SACD_Vec3 sphere_prev_pos[2];
  SC_SACD_AABB_Box floor_box;
  Model ground_model;
  Shader ground_shader;
  Music battle_music;
  std::vector<char> music_data;
  int ground_shader_scale_idx;
  int ground_shader_pos_idx;
  int ground_shader_other_pos_idx;
  int ground_shader_radius_idx;
  int ground_shader_ground_size_idx;
  float ground_scale;
  float ground_pos[4];
  bool sphere_collided;
  bool prev_auto_move_flag_value;
  bool prev_music_play_value;
};

#endif
