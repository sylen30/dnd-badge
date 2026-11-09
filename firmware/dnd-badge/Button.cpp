#include "Button.h"

Button::Button(uint8_t pin, bool activeLow, uint16_t debounceMs, uint16_t longPressMs,
               uint16_t doubleClickMs)
    : _pin(pin),
      _activeLow(activeLow),
      _debounceMs(debounceMs),
      _longPressMs(longPressMs),
      _doubleClickMs(doubleClickMs) {}

void Button::begin() {
  pinMode(_pin, _activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
  _lastRaw = _stable = readRaw();
  _rawChangedAt = _pressedAt = millis();
  // If the button is already held at boot, swallow that hold so it doesn't
  // also produce a click or long press when it's released.
  _longFired = _stable;
}

bool Button::readRaw() const {
  bool level = digitalRead(_pin);
  return _activeLow ? !level : level;
}

void Button::inject(Event e) { _injected = e; }

Button::Event Button::poll() {
  if (_injected != NONE) {
    Event e = _injected;
    _injected = NONE;
    return e;
  }

  const uint32_t now = millis();

  // 1. Debounce: only accept a new state once the raw input has been
  //    steady for DEBOUNCE_MS. Contact bounce resets the timer.
  bool raw = readRaw();
  if (raw != _lastRaw) {
    _lastRaw = raw;
    _rawChangedAt = now;
  }

  if (raw != _stable && (now - _rawChangedAt) >= _debounceMs) {
    _stable = raw;
    if (_stable) {  // press edge
      _pressedAt = now;
      _longFired = false;
    } else {        // release edge
      if (!_longFired) {
        _pendingClicks++;
        _releasedAt = now;
      }
    }
  }

  // 2. Long press fires while still held, once per hold.
  if (_stable && !_longFired && (now - _pressedAt) >= _longPressMs) {
    _longFired = true;
    _pendingClicks = 0;
    return LONG_PRESS;
  }

  // 3. Clicks: wait briefly after release to see if it becomes a double click.
  if (_pendingClicks >= 2) {
    _pendingClicks = 0;
    return DOUBLE_CLICK;
  }
  if (_pendingClicks == 1 && !_stable && (now - _releasedAt) >= _doubleClickMs) {
    _pendingClicks = 0;
    return CLICK;
  }

  return NONE;
}

const char* Button::name(Event e) {
  switch (e) {
    case CLICK: return "click";
    case DOUBLE_CLICK: return "double click";
    case LONG_PRESS: return "long press";
    default: return "none";
  }
}
