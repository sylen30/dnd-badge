#include "Settings.h"
#include <Preferences.h>

static const char* NS = "badge";

void Settings::load() {
  Preferences p;
  p.begin(NS, true);
  name = p.getString("name", name);
  subtitle = p.getString("sub", subtitle);
  uint8_t m = p.getUChar("mode", (uint8_t)mode);
  mode = m < (uint8_t)Mode::COUNT ? (Mode)m : Mode::NAME;
  animation = p.getUChar("anim", animation);
  uint8_t l = p.getUChar("led", (uint8_t)ledMode);
  ledMode = l < (uint8_t)LedMode::COUNT ? (LedMode)l : LedMode::SYNC;
  brightness = p.getUChar("bright", brightness);
  p.end();
}

void Settings::save() const {
  Preferences p;
  p.begin(NS, false);
  p.putString("name", name);
  p.putString("sub", subtitle);
  p.putUChar("mode", (uint8_t)mode);
  p.putUChar("anim", animation);
  p.putUChar("led", (uint8_t)ledMode);
  p.putUChar("bright", brightness);
  p.end();
}

const char* modeName(Mode m) {
  switch (m) {
    case Mode::NAME: return "Name";
    case Mode::DICE: return "Dice";
    case Mode::ANIMATION: return "Animation";
    default: return "?";
  }
}

const char* ledModeName(LedMode m) {
  switch (m) {
    case LedMode::OFF: return "Off";
    case LedMode::SYNC: return "Sync with screen";
    case LedMode::SEPARATE: return "Separate";
    default: return "?";
  }
}
