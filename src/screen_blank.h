#ifndef SEODISPARATE_COM_GANDER_BATTLE_BLANK_SCREEN_H_
#define SEODISPARATE_COM_GANDER_BATTLE_BLANK_SCREEN_H_

#include "screen.h"

class BlankScreen : public Screen {
 public:
  BlankScreen(ScreenStack::Weak ss);
  virtual ~BlankScreen();

  virtual bool update(float dt, bool screen_resized) override;
  virtual bool draw(RenderTexture* render_texture) override;

  virtual std::list<std::string> get_known_flags() const override;
};

#endif
