#pragma once
// ---------------------------------------------------------------------------
// Board configuration. Change these to match the badge PCB.
// Defaults suit a standard "ESP32 Dev Module" / DevKit V1 for bench testing.
// ---------------------------------------------------------------------------

// --- OLED (I2C) ---
#define PIN_OLED_SDA 21
#define PIN_OLED_SCL 22
// Uncomment if the screen is an SH1106 rather than an SSD1306.
// (A 1.3" 128x64 is usually SH1106, a 0.96" is usually SSD1306.)
// #define OLED_SH1106

// --- Button ---
// GPIO0 is the BOOT button on most dev boards, so you can test without wiring
// anything. Button connects the pin to GND (active low, internal pull-up).
#define PIN_BUTTON 0
#define BUTTON_ACTIVE_LOW true

// --- LEDs (not driven yet; reserved for the WS2812B ring) ---
#define PIN_LED_DATA 13
#define LED_COUNT 10

// --- Button timing (ms) ---
#define DEBOUNCE_MS 25        // input must be stable this long to count
#define LONG_PRESS_MS 1500    // hold this long for a long press
#define DOUBLE_CLICK_MS 350   // second click within this window = double click

// --- Wi-Fi setup portal ---
#define AP_SSID "DnD-Badge"
#define AP_PASSWORD ""        // empty = open network (8+ chars to secure it)
#define PORTAL_TIMEOUT_MS (10UL * 60UL * 1000UL)  // auto-close after 10 minutes idle

// --- Serial debug ---
#define SERIAL_BAUD 115200
