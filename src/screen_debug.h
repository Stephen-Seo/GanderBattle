#ifndef SEODISPARATE_COM_GANDER_BATTLE_SCREEN_DEBUG_H_
#define SEODISPARATE_COM_GANDER_BATTLE_SCREEN_DEBUG_H_

// Local includes.
#include "screen.h"

class DebugScreen : public Screen {
 public:
  DebugScreen(std::weak_ptr<ScreenStack> stack);
  virtual ~DebugScreen();

  virtual bool update(float dt, bool screen_resized);

  virtual bool draw(RenderTexture* render_texture);

 private:
};

#endif
