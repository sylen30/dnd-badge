// LED test firmware for the web flasher.
//
// Blinks the board's own LED once a second so you can see the flash worked.
//   ESP8266 / ESP-12F (the badge): blue LED on the module, GPIO2.
//     Also cycles the badge's WS2812B ring (GPIO15) red -> green -> blue,
//     if the ring has power (battery switch on).
//   ESP32 / ESP32-S3 / ESP32-C3 dev boards: the board's built-in LED.
//
// Open the flasher's "Logs & Console" to see a "blink" message every second.

#include <Arduino.h>

#if defined(ESP8266)
  #include <Adafruit_NeoPixel.h>
  #define STATUS_LED 2          // ESP-12F on-board LED, lit when LOW
  #define STATUS_ON LOW
  #define STATUS_OFF HIGH
  #define RING_PIN 15           // badge LED_DATA (GPIO15, pulled low by R6)
  #define RING_COUNT 10
  Adafruit_NeoPixel ring(RING_COUNT, RING_PIN, NEO_GRB + NEO_KHZ800);
#endif

static bool on = false;
static uint8_t step = 0;

static void setStatusLed(bool lit) {
#if defined(ESP8266)
  digitalWrite(STATUS_LED, lit ? STATUS_ON : STATUS_OFF);
#elif defined(RGB_BUILTIN)
  // S3 / C3 dev boards have an addressable RGB LED instead of a plain one.
  if (lit) neopixelWrite(RGB_BUILTIN, 0, 0, 40);
  else neopixelWrite(RGB_BUILTIN, 0, 0, 0);
#elif defined(LED_BUILTIN)
  digitalWrite(LED_BUILTIN, lit ? HIGH : LOW);
#else
  digitalWrite(2, lit ? HIGH : LOW);   // most ESP32 dev boards
#endif
}

void setup() {
  Serial.begin(115200);
#if defined(ESP8266)
  pinMode(STATUS_LED, OUTPUT);
  ring.begin();
  ring.clear();
  ring.show();               // make sure the ring starts dark
#elif !defined(RGB_BUILTIN)
  #if defined(LED_BUILTIN)
    pinMode(LED_BUILTIN, OUTPUT);
  #else
    pinMode(2, OUTPUT);
  #endif
#endif
  delay(200);
  Serial.println();
  Serial.println("D&D Badge LED test running");
}

void loop() {
  on = !on;
  setStatusLed(on);

#if defined(ESP8266)
  if (on) {
    const uint32_t colours[3] = {ring.Color(40, 0, 0), ring.Color(0, 40, 0), ring.Color(0, 0, 40)};
    ring.fill(colours[step % 3]);
    step++;
  } else {
    ring.clear();
  }
  ring.show();
#endif

  Serial.println(on ? "blink: on" : "blink: off");
  delay(500);
}
