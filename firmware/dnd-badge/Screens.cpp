#include "Screens.h"
#include "config.h"

#ifdef OLED_SH1106
static U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA);
#else
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA);
#endif

static const uint16_t FRAME_MS = 40;  // ~25 fps
static const uint8_t W = 128, H = 64;

const char* Screens::animationName(uint8_t i) {
  switch (i) {
    case 0: return "D20";
    case 1: return "Torch";
    case 2: return "Sparkles";
    default: return "?";
  }
}

void Screens::begin(uint8_t contrast) {
  u8g2.setBusClock(400000);
  u8g2.begin();
  u8g2.setContrast(contrast);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB10_tr);
  u8g2.drawStr(20, 30, "D&D Badge");
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(30, 46, "booting...");
  u8g2.sendBuffer();
  randomSeed(esp_random());
  _modeStart = millis();
}

void Screens::setContrast(uint8_t c) { u8g2.setContrast(c); }

bool Screens::due() {
  uint32_t now = millis();
  if (now - _lastDraw < FRAME_MS) return false;
  _lastDraw = now;
  _frame++;
  return true;
}

void Screens::resetMode() {
  _modeStart = millis();
  _rolling = false;
}

void Screens::toast(const String& msg, uint16_t ms) {
  _toast = msg;
  _toastUntil = millis() + ms;
}

void Screens::drawToast() {
  if ((int32_t)(millis() - _toastUntil) >= 0) return;
  u8g2.setFont(u8g2_font_6x12_tr);
  int w = u8g2.getStrWidth(_toast.c_str()) + 8;
  int x = (W - w) / 2;
  u8g2.setDrawColor(0);
  u8g2.drawBox(x - 1, 21, w + 2, 20);
  u8g2.setDrawColor(1);
  u8g2.drawRFrame(x, 22, w, 18, 3);
  u8g2.drawStr(x + 4, 35, _toast.c_str());
}

// ---------------------------------------------------------------------------

void Screens::render(const Settings& s) {
  if (!due()) return;
  u8g2.clearBuffer();
  switch (s.mode) {
    case Mode::NAME: drawName(s); break;
    case Mode::DICE: drawDice(); break;
    case Mode::ANIMATION: drawAnimation(s.animation); break;
    default: break;
  }
  drawToast();
  u8g2.sendBuffer();
}

void Screens::action(const Settings& s) {
  if (s.mode == Mode::DICE) {
    _rolling = true;
    _rollStart = millis();
    _tickGap = 30;
    _nextTick = 0;
  }
}

// --- Name badge: big name, scrolls if too wide; subtitle underneath --------
void Screens::drawName(const Settings& s) {
  u8g2.setFont(u8g2_font_helvB18_tr);
  int w = u8g2.getStrWidth(s.name.c_str());
  int x;
  if (w <= W) {
    x = (W - w) / 2;
  } else {
    // scroll right-to-left, with a gap before it repeats
    int span = w + 40;
    x = W - (int)(((millis() - _modeStart) / 25) % (W + span));
  }
  u8g2.drawStr(x, 30, s.name.c_str());

  u8g2.drawHLine(8, 38, W - 16);

  u8g2.setFont(u8g2_font_6x12_tr);
  int sw = u8g2.getStrWidth(s.subtitle.c_str());
  u8g2.drawStr(max(0, (W - sw) / 2), 54, s.subtitle.c_str());
}

// --- Dice: double click (or every 8s) rolls a d20 ---------------------------
static void drawHex(int cx, int cy, int r, float rot) {
  int px[6], py[6];
  for (int i = 0; i < 6; i++) {
    float a = rot + i * PI / 3.0f;
    px[i] = cx + (int)(r * cosf(a));
    py[i] = cy + (int)(r * sinf(a));
  }
  for (int i = 0; i < 6; i++) u8g2.drawLine(px[i], py[i], px[(i + 1) % 6], py[(i + 1) % 6]);
}

void Screens::drawDice() {
  uint32_t now = millis();
  if (!_rolling && now - _modeStart > 8000 && now - _rollStart > 8000) {
    _rolling = true;
    _rollStart = now;
    _tickGap = 30;
    _nextTick = 0;
  }

  float rot = -PI / 2;
  if (_rolling) {
    if (now >= _nextTick) {
      _diceValue = random(1, 21);
      _nextTick = now + _tickGap;
      _tickGap = _tickGap * 115 / 100;  // slow down like a real roll
    }
    rot += (now - _rollStart) / 120.0f;
    if (_tickGap > 400) _rolling = false;
  }

  const int cx = 40, cy = 32;
  drawHex(cx, cy, 28, rot);
  drawHex(cx, cy, 16, rot + PI / 6);

  u8g2.setFont(u8g2_font_logisoso16_tn);
  char buf[4];
  snprintf(buf, sizeof(buf), "%u", _diceValue);
  int w = u8g2.getStrWidth(buf);
  u8g2.drawStr(cx - w / 2, cy + 8, buf);

  u8g2.setFont(u8g2_font_6x12_tr);
  if (_rolling) {
    u8g2.drawStr(78, 36, "Rolling");
  } else if (_diceValue == 20) {
    if ((_frame / 6) % 2) u8g2.drawStr(80, 30, "CRIT!");
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(76, 44, "Natural 20");
  } else if (_diceValue == 1) {
    u8g2.drawStr(80, 30, "FAIL");
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(76, 44, "Natural 1");
  } else {
    u8g2.drawStr(78, 36, "You rolled");
  }
}

// --- Animations --------------------------------------------------------------
void Screens::drawAnimation(uint8_t which) {
  switch (which % ANIMATION_COUNT) {
    case 0: animD20(); break;
    case 1: animTorch(); break;
    case 2: animSparkles(); break;
  }
}

void Screens::animD20() {
  float t = _frame * 0.06f;
  int bob = (int)(4 * sinf(t * 2));
  drawHex(64, 30 + bob, 24, t);
  drawHex(64, 30 + bob, 13, -t);
  // spokes between the two hexes give it a faceted look
  for (int i = 0; i < 6; i += 2) {
    float a = t + i * PI / 3.0f, b = -t + (i + 1) * PI / 3.0f;
    u8g2.drawLine(64 + 24 * cosf(a), 30 + bob + 24 * sinf(a), 64 + 13 * cosf(b), 30 + bob + 13 * sinf(b));
  }
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(30, 63, "Roll for initiative");
}

void Screens::animTorch() {
  // handle
  u8g2.drawBox(60, 40, 8, 24);
  u8g2.drawFrame(56, 36, 16, 6);
  // flame: stacked ellipses whose size flickers
  for (int i = 0; i < 3; i++) {
    int rx = 10 - i * 3 + random(-1, 2);
    int ry = 14 - i * 3 + random(-2, 2);
    int cy = 24 - i * 3;
    if (i == 1) {
      u8g2.drawEllipse(64 + random(-1, 2), cy, rx, ry);
    } else {
      u8g2.drawFilledEllipse(64 + random(-1, 2), cy, rx, ry);
    }
  }
  // embers drifting up
  for (int i = 0; i < 5; i++) {
    int y = 30 - (int)((_frame * (2 + i) + i * 17) % 32);
    int x = 64 + (int)(8 * sinf((_frame + i * 30) * 0.15f));
    u8g2.drawPixel(x, y);
  }
}

void Screens::animSparkles() {
  // deterministic star field so stars twinkle rather than jump around
  for (int i = 0; i < 24; i++) {
    uint32_t h = (i * 2654435761UL);
    int x = h % W, y = (h >> 8) % H;
    uint8_t phase = (_frame + (h >> 16)) % 40;
    if (phase < 3) {
      u8g2.drawPixel(x, y);
      u8g2.drawPixel(x - 1, y); u8g2.drawPixel(x + 1, y);
      u8g2.drawPixel(x, y - 1); u8g2.drawPixel(x, y + 1);
    } else if (phase < 20) {
      u8g2.drawPixel(x, y);
    }
  }
}

// --- Wi-Fi setup screen ------------------------------------------------------
void Screens::renderPortal(const char* ssid, const String& ip, uint8_t clients, bool justSaved) {
  if (!due()) return;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 10, "Wi-Fi setup");
  // little blinking indicator
  if ((_frame / 12) % 2) u8g2.drawDisc(122, 6, 3);
  u8g2.drawHLine(0, 13, W);

  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(0, 25, "Join:");
  u8g2.drawStr(30, 25, ssid);
  u8g2.drawStr(0, 36, "Open:");
  u8g2.drawStr(30, 36, ip.c_str());

  char buf[24];
  snprintf(buf, sizeof(buf), "Connected: %u", clients);
  u8g2.drawStr(0, 47, buf);

  u8g2.drawStr(0, 62, justSaved ? "Saved! Hold btn to exit" : "Hold button to exit");
  drawToast();
  u8g2.sendBuffer();
}
