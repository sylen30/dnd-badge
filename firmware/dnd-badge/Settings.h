#pragma once
#include <Arduino.h>

enum class Mode : uint8_t { NAME, DICE, ANIMATION, COUNT };
enum class LedMode : uint8_t { OFF, SYNC, SEPARATE, COUNT };

struct Settings {
  String name = "Adventurer";
  String subtitle = "Level 1 Badge";
  Mode mode = Mode::NAME;
  uint8_t animation = 0;
  LedMode ledMode = LedMode::SYNC;
  uint8_t brightness = 128;  // screen contrast 0-255

  void load();   // from NVS flash; keeps defaults for anything missing
  void save() const;
};

const char* modeName(Mode m);
const char* ledModeName(LedMode m);
