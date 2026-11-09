#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "Settings.h"

// Everything that draws on the OLED. All drawing is non-blocking: call
// render() every loop and it redraws at a fixed frame rate.
class Screens {
 public:
  static const uint8_t ANIMATION_COUNT = 3;
  static const char* animationName(uint8_t i);

  void begin(uint8_t contrast);
  void setContrast(uint8_t c);

  // Normal badge modes
  void render(const Settings& s);
  void action(const Settings& s);      // double click: roll dice etc.
  void resetMode();                    // call when the mode changes

  // Wi-Fi setup screen
  void renderPortal(const char* ssid, const String& ip, uint8_t clients, bool justSaved);

  // Short overlay message, e.g. the new mode name
  void toast(const String& msg, uint16_t ms = 1000);

  // Increments once per drawn frame. The LED code can follow this to stay in
  // time with the screen.
  uint32_t frame() const { return _frame; }

 private:
  bool due();
  void drawToast();
  void drawName(const Settings& s);
  void drawDice();
  void drawAnimation(uint8_t which);
  void animD20();
  void animTorch();
  void animSparkles();

  uint32_t _lastDraw = 0;
  uint32_t _frame = 0;
  uint32_t _modeStart = 0;

  String _toast;
  uint32_t _toastUntil = 0;

  // dice state
  uint8_t _diceValue = 20;
  uint32_t _rollStart = 0;
  uint32_t _nextTick = 0;
  uint16_t _tickGap = 0;
  bool _rolling = false;
};
