# D&D Badge

Firmware for an ESP32 badge with a 128×64 OLED, one button and a ring of WS2812B LEDs.

## What works so far

- **Screen modes**
  - **Name:** big name (scrolls if it's too long) with a subtitle.
  - **Dice:** a d20 that rolls every 8 s, or when you double click. Shows "CRIT!" on a 20 and "FAIL" on a 1.
  - **Animation:** three built-in animations (spinning D20, torch, sparkles).
- **Button:** debounced, with click, double click and long press detection.
- **Wi-Fi setup mode:** the badge becomes a hotspot called `DnD-Badge`. When you join it, the settings page pops up on your phone. From there you can change the name, subtitle, mode, animation, LED mode and brightness.
- **Settings are saved** to flash and survive power-offs.
- **Serial console** for simulating the button without hardware.
- **LEDs:** not driven yet. The LED mode setting is stored and there's a TODO hook in `App.cpp`.

## Controls

| Input | Normal mode | Setup mode |
|---|---|---|
| Click | Next screen mode | – |
| Double click | Roll dice / next animation | – |
| Long press (1.5 s) | Enter Wi-Fi setup | Leave Wi-Fi setup |
| Hold at power-on | Boot straight into setup | |

Setup mode also closes itself after 10 minutes with no one connected.

### Simulating the button

Open the serial monitor at 115200 baud and send:

| Key | Does |
|---|---|
| `c` | click |
| `d` | double click |
| `l` | long press |
| `s` | print status (mode, settings, free heap) |
| `h` | help |

On a dev board, GPIO0 is the **BOOT** button, so that works as the badge button with no wiring.

## Pins

Everything is in `firmware/dnd-badge/config.h`. The defaults suit an ESP32 DevKit:

| Function | GPIO |
|---|---|
| OLED SDA | 21 |
| OLED SCL | 22 |
| Button (to GND) | 0 |
| LED data (unused for now) | 13 |

If the screen shows garbage or is shifted by 2 pixels, it's an SH1106. Uncomment `OLED_SH1106` in `config.h`.

## Building

**PlatformIO:** `pio run -t upload && pio device monitor`

**Arduino IDE:** open `firmware/dnd-badge/dnd-badge.ino`, install **U8g2** from the Library Manager, select **ESP32 Dev Module**, then upload.

The current build uses about 76% of the default 1.3 MB app partition. Most of that is the Wi-Fi stack. Before adding lots of bitmap animations, switch the partition scheme to **Huge APP** (Arduino), or set `board_build.partitions = huge_app.csv` (PlatformIO).

## Code layout

| File | What it does |
|---|---|
| `App.cpp` | `setup()`/`loop()`, button handling, mode switching, serial commands |
| `Button.*` | Debounce plus click, double click and long press detection |
| `Screens.*` | All OLED drawing; non-blocking at about 25 fps |
| `Portal.*` | Wi-Fi hotspot, DNS captive portal and settings web page |
| `Settings.*` | Settings struct, saved to flash with `Preferences` |
| `config.h` | Pins and timings |
