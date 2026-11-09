#pragma once
#include <Arduino.h>
#include "Settings.h"

// Wi-Fi access point + captive portal for changing the badge settings from a
// phone. Joining the AP pops up the settings page automatically on most phones.
class Portal {
 public:
  void start(Settings* settings);
  void stop();
  void loop();

  bool active() const { return _active; }
  bool timedOut() const;
  uint8_t clients() const;
  String ip() const;

  // True once after the settings form has been saved.
  bool takeSaved();

 private:
  bool _active = false;
  bool _saved = false;
  uint32_t _lastActivity = 0;
};
