#include "Portal.h"
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>
#include "Screens.h"
#include "config.h"

static DNSServer dns;
static WebServer server(80);
static Settings* cfg = nullptr;
static Portal* self = nullptr;
static bool* savedFlag = nullptr;
static uint32_t* activity = nullptr;

static String esc(const String& in) {
  String out;
  out.reserve(in.length() + 8);
  for (char c : in) {
    switch (c) {
      case '&': out += F("&amp;"); break;
      case '<': out += F("&lt;"); break;
      case '>': out += F("&gt;"); break;
      case '"': out += F("&quot;"); break;
      default: out += c;
    }
  }
  return out;
}

static String option(int value, const char* label, int selected) {
  String s = "<option value='" + String(value) + "'";
  if (value == selected) s += " selected";
  s += ">";
  s += label;
  s += "</option>";
  return s;
}

static void sendPage(const char* notice) {
  String h;
  h.reserve(2600);
  h += F("<!doctype html><html><head><meta charset='utf-8'>"
         "<meta name='viewport' content='width=device-width,initial-scale=1'>"
         "<title>D&amp;D Badge</title><style>"
         "body{font-family:system-ui,sans-serif;background:#16121f;color:#eee;margin:0;padding:16px}"
         "h1{font-size:1.4em;margin:0 0 12px}label{display:block;margin:14px 0 4px;font-weight:600}"
         "input,select{width:100%;box-sizing:border-box;padding:10px;font-size:1em;border-radius:8px;"
         "border:1px solid #555;background:#241d33;color:#eee}"
         "button{margin-top:20px;width:100%;padding:12px;font-size:1.05em;border:0;border-radius:8px;"
         "background:#b33a3a;color:#fff;font-weight:700}"
         ".n{background:#2f5d34;padding:10px;border-radius:8px}</style></head><body>"
         "<h1>&#9876; D&amp;D Badge</h1>");
  if (notice) {
    h += F("<p class='n'>");
    h += notice;
    h += F("</p>");
  }
  h += F("<form method='POST' action='/save'>");
  h += F("<label>Name</label><input name='name' maxlength='32' value='");
  h += esc(cfg->name);
  h += F("'><label>Subtitle</label><input name='sub' maxlength='40' value='");
  h += esc(cfg->subtitle);
  h += F("'><label>Screen mode</label><select name='mode'>");
  for (int i = 0; i < (int)Mode::COUNT; i++) h += option(i, modeName((Mode)i), (int)cfg->mode);
  h += F("</select><label>Animation</label><select name='anim'>");
  for (int i = 0; i < Screens::ANIMATION_COUNT; i++)
    h += option(i, Screens::animationName(i), cfg->animation);
  h += F("</select><label>LEDs</label><select name='led'>");
  for (int i = 0; i < (int)LedMode::COUNT; i++) h += option(i, ledModeName((LedMode)i), (int)cfg->ledMode);
  h += F("</select><label>Screen brightness</label><input type='range' name='bright' min='1' max='255' value='");
  h += String(cfg->brightness);
  h += F("'><button type='submit'>Save</button></form></body></html>");
  server.send(200, "text/html", h);
}

static void handleRoot() {
  *activity = millis();
  sendPage(nullptr);
}

static void handleSave() {
  *activity = millis();
  if (server.hasArg("name")) {
    String n = server.arg("name");
    n.trim();
    if (n.length()) cfg->name = n.substring(0, 32);
  }
  if (server.hasArg("sub")) cfg->subtitle = server.arg("sub").substring(0, 40);
  if (server.hasArg("mode")) {
    int m = server.arg("mode").toInt();
    if (m >= 0 && m < (int)Mode::COUNT) cfg->mode = (Mode)m;
  }
  if (server.hasArg("anim")) {
    int a = server.arg("anim").toInt();
    if (a >= 0 && a < Screens::ANIMATION_COUNT) cfg->animation = a;
  }
  if (server.hasArg("led")) {
    int l = server.arg("led").toInt();
    if (l >= 0 && l < (int)LedMode::COUNT) cfg->ledMode = (LedMode)l;
  }
  if (server.hasArg("bright")) cfg->brightness = constrain(server.arg("bright").toInt(), 1, 255);
  cfg->save();
  *savedFlag = true;
  Serial.printf("[portal] saved: name='%s' mode=%s\n", cfg->name.c_str(), modeName(cfg->mode));
  sendPage("Saved. Hold the badge button to leave setup mode.");
}

// Any unknown URL (including the phone's captive-portal checks) is sent to
// the settings page, which is what makes the "sign in to network" pop-up appear.
static void handleRedirect() {
  *activity = millis();
  server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + "/", true);
  server.send(302, "text/plain", "");
}

void Portal::start(Settings* settings) {
  if (_active) return;
  cfg = settings;
  self = this;
  savedFlag = &_saved;
  activity = &_lastActivity;

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, strlen(AP_PASSWORD) >= 8 ? AP_PASSWORD : nullptr);
  delay(100);
  dns.setErrorReplyCode(DNSReplyCode::NoError);
  dns.start(53, "*", WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handleRedirect);
  server.begin();

  _active = true;
  _saved = false;
  _lastActivity = millis();
  Serial.printf("[portal] started: SSID '%s' at %s\n", AP_SSID, ip().c_str());
}

void Portal::stop() {
  if (!_active) return;
  server.stop();
  dns.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  _active = false;
  Serial.println("[portal] stopped");
}

void Portal::loop() {
  if (!_active) return;
  dns.processNextRequest();
  server.handleClient();
  if (WiFi.softAPgetStationNum() > 0) _lastActivity = millis();
}

bool Portal::timedOut() const { return _active && millis() - _lastActivity > PORTAL_TIMEOUT_MS; }

uint8_t Portal::clients() const { return _active ? WiFi.softAPgetStationNum() : 0; }

String Portal::ip() const { return WiFi.softAPIP().toString(); }

bool Portal::takeSaved() {
  bool s = _saved;
  _saved = false;
  return s;
}
