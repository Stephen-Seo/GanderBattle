#include "ems.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/fetch.h>
#include <emscripten/html5.h>

EM_JS(int, canvas_get_width, (),
      { return document.getElementById("canvas").clientWidth; });

EM_JS(int, canvas_get_height, (),
      { return document.getElementById("canvas").clientHeight; });

EM_JS(float, js_get_random, (), { return Math.random(); });

int call_js_get_canvas_width() { return canvas_get_width(); }

int call_js_get_canvas_height() { return canvas_get_height(); }

float call_js_get_random() { return js_get_random(); }

#else
#include <random>

int call_js_get_canvas_width() { return 800; }

int call_js_get_canvas_height() { return 800; }

float call_js_get_random() {
  static std::default_random_engine re{std::random_device()()};
  static std::uniform_real_distribution<float> dist(0.0F, 1.0F);
  return dist(re);
}
#endif
