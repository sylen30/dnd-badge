// D&D Badge firmware
//
// Button:
//   click         -> next screen mode (Name / Dice / Animation)
//   double click  -> mode action (roll the dice, or next animation)
//   long press    -> enter / leave Wi-Fi setup mode
//
// Serial console (115200 baud) can simulate the button:
//   c = click, d = double click, l = long press, s = status, h = help

#include <Arduino.h>
#include "Button.h"
#include "Portal.h"
#include "Screens.h"
#include "Settings.h"
#include "config.h"

static Button button(PIN_BUTTON, BUTTON_ACTIVE_LOW, DEBOUNCE_MS, LONG_PRESS_MS, DOUBLE_CLICK_MS);
static Screens screens;
static Portal portal;
static Settings settings;
static bool portalSaved = false;

static void printHelp() {
  Serial.println(F("Commands: c=click  d=double click  l=long press  s=status  h=help"));
}

static void printStatus() {
  Serial.printf("mode=%s anim=%s leds=%s name='%s' sub='%s' wifi=%s heap=%u\n",
                modeName(settings.mode), Screens::animationName(settings.animation),
                ledModeName(settings.ledMode), settings.name.c_str(), settings.subtitle.c_str(),
                portal.active() ? "setup" : "off", ESP.getFreeHeap());
}

static void readSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'c': button.inject(Button::CLICK); break;
      case 'd': button.inject(Button::DOUBLE_CLICK); break;
      case 'l': button.inject(Button::LONG_PRESS); break;
      case 's': printStatus(); break;
      case 'h': case '?': printHelp(); break;
      default: break;  // ignore newlines etc.
    }
  }
}

static void enterPortal() {
  portal.start(&settings);
  portalSaved = false;
  screens.toast("Setup mode");
}

static void leavePortal(const char* why) {
  portal.stop();
  screens.setContrast(settings.brightness);
  screens.resetMode();
  screens.toast(why);
}

static void handleNormal(Button::Event e) {
  switch (e) {
    case Button::CLICK:
      settings.mode = (Mode)(((uint8_t)settings.mode + 1) % (uint8_t)Mode::COUNT);
      settings.save();
      screens.resetMode();
      screens.toast(modeName(settings.mode));
      break;
    case Button::DOUBLE_CLICK:
      if (settings.mode == Mode::ANIMATION) {
        settings.animation = (settings.animation + 1) % Screens::ANIMATION_COUNT;
        settings.save();
        screens.toast(Screens::animationName(settings.animation));
      } else {
        screens.action(settings);
      }
      break;
    case Button::LONG_PRESS:
      enterPortal();
      break;
    default:
      break;
  }
}

static void handlePortal(Button::Event e) {
  if (e == Button::LONG_PRESS) leavePortal("Setup closed");
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(50);
  Serial.println(F("\n=== D&D Badge ==="));

  settings.load();
  button.begin();
  screens.begin(settings.brightness);

  // Holding the button while powering on goes straight to setup mode.
  delay(300);
  if (button.isHeld()) {
    Serial.println(F("Button held at boot -> setup mode"));
    enterPortal();
  }

  printStatus();
  printHelp();
}

void loop() {
  readSerial();

  Button::Event e = button.poll();
  if (e != Button::NONE) Serial.printf("[button] %s\n", Button::name(e));

  if (portal.active()) {
    handlePortal(e);
  } else {
    handleNormal(e);
  }

  if (portal.active()) {
    portal.loop();
    if (portal.takeSaved()) {
      portalSaved = true;
      screens.setContrast(settings.brightness);
      screens.toast("Saved!");
    }
    if (portal.timedOut()) {
      leavePortal("Setup timed out");
    } else {
      screens.renderPortal(AP_SSID, portal.ip(), portal.clients(), portalSaved);
    }
  } else {
    screens.render(settings);
  }

  // TODO LEDs: when the ring is fitted, update it here. In SYNC mode follow
  // screens.frame(); in SEPARATE mode run an independent effect.
}
