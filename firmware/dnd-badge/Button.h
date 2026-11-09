#pragma once
#include <Arduino.h>

// Debounced push button with click / double click / long press detection.
//
// Events are produced by poll(), which must be called every loop.
// inject() lets you feed in an event from elsewhere (e.g. the serial console)
// so presses can be simulated without touching the hardware.
class Button {
 public:
  enum Event : uint8_t { NONE, CLICK, DOUBLE_CLICK, LONG_PRESS };

  Button(uint8_t pin, bool activeLow, uint16_t debounceMs, uint16_t longPressMs,
         uint16_t doubleClickMs);

  void begin();
  Event poll();              // call every loop; returns at most one event
  void inject(Event e);      // queue a simulated event
  bool isHeld() const { return _stable; }

  static const char* name(Event e);

 private:
  bool readRaw() const;

  uint8_t _pin;
  bool _activeLow;
  uint16_t _debounceMs, _longPressMs, _doubleClickMs;

  bool _lastRaw = false;      // last raw reading
  bool _stable = false;       // debounced state (true = pressed)
  uint32_t _rawChangedAt = 0; // when the raw reading last changed
  uint32_t _pressedAt = 0;    // when the debounced press started
  bool _longFired = false;    // long press already reported for this hold
  uint8_t _pendingClicks = 0; // clicks waiting to see if a second one follows
  uint32_t _releasedAt = 0;
  Event _injected = NONE;
};
