#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <time.h>

#define LEFT_BTN_PIN 2
#define MIDDLE_BTN_PIN 4
#define RIGHT_BTN_PIN 5
#define BUZZER_PIN 3

const char* WIFI_SSID = "wifi name";
const char* WIFI_PASSWORD = "wifi password";

const char* NTP_SERVER = "pool.ntp.org";
const char* TZ_INFO = "EST5EDT,M3.2.0,M11.1.0";

TFT_eSPI tft = TFT_eSPI();

#define COL_BG TFT_BLACK
#define COL_TEXT TFT_WHITE
#define COL_DIM 0x7BEF
#define COL_ACCENT TFT_CYAN
#define COL_ALARM TFT_ORANGE
#define COL_OK TFT_GREEN
#define COL_OFF TFT_RED

struct Button {
  uint8_t pin;
  bool lastState;
  bool stableState;
  unsigned long lastChange;
  bool pressedEvent;
};

Button btnLeft = {LEFT_BTN_PIN, HIGH, HIGH, 0, false};
Button btnMiddle = {MIDDLE_BTN_PIN, HIGH, HIGH, 0, false};
Button btnRight = {RIGHT_BTN_PIN, HIGH, HIGH, 0, false};

const unsigned long DEBOUNCE_MS = 30;

unsigned long lastMiddlePressTime = 0;
const unsigned long DOUBLE_PRESS_WINDOW_MS = 400;
bool middleDoublePressEvent = false;

enum Screen {
  SCR_HOME,
  SCR_ALARM_HOME,
  SCR_ALARM_SET_HOUR,
  SCR_ALARM_SET_MIN,
  SCR_ALARM_SET_TOGGLE,
  SCR_TIMER_HOME,
  SCR_TIMER_SET_HOUR,
  SCR_TIMER_SET_MIN,
  SCR_TIMER_RUNNING,
  SCR_STOPWATCH
};

Screen currentScreen = SCR_HOME;
Screen lastDrawnScreen = (Screen)-1;
bool screenDirty = true;

int alarmHour = 7;
int alarmMinute = 0;
bool alarmEnabled = false;
bool alarmRinging = false;

int timerSetHour = 0;
int timerSetMinute = 5;
bool timerRunning = false;
unsigned long timerEndMillis = 0;
long timerRemainingSeconds = 0;
bool timerRinging = false;

bool stopwatchRunning = false;
unsigned long stopwatchStartMillis = 0;
unsigned long stopwatchAccumMillis = 0;

void buzzShort() {
  tone(BUZZER_PIN, 2000, 80);
}

void buzzBeepBeep() {
  tone(BUZZER_PIN, 1500, 60);
}

unsigned long lastRingToggle = 0;
bool ringToneOn = false;
void serviceRinging(bool &ringingFlag) {
  if (!ringingFlag) return;
  unsigned long now = millis();
  if (now - lastRingToggle > 400) {
    lastRingToggle = now;
    ringToneOn = !ringToneOn;
    if (ringToneOn) tone(BUZZER_PIN, 2500);
    else noTone(BUZZER_PIN);
  }
}

void stopRinging(bool &ringingFlag) {
  ringingFlag = false;
  noTone(BUZZER_PIN);
}

void updateButton(Button &b) {
  bool reading = digitalRead(b.pin);
  b.pressedEvent = false;

  if (reading != b.lastState) {
    b.lastChange = millis();
  }

  if ((millis() - b.lastChange) > DEBOUNCE_MS) {
    if (reading != b.stableState) {
      b.stableState = reading;
      if (b.stableState == LOW) {
        b.pressedEvent = true;
      }
    }
  }
  b.lastState = reading;
}

void readButtons() {
  updateButton(btnLeft);
  updateButton(btnMiddle);
  updateButton(btnRight);

  middleDoublePressEvent = false;
  if (btnMiddle.pressedEvent) {
    unsigned long now = millis();
    if (now - lastMiddlePressTime <= DOUBLE_PRESS_WINDOW_MS) {
      middleDoublePressEvent = true;
    }
    lastMiddlePressTime = now;
  }
}

bool getTimeNow(struct tm &timeinfo) {
  return getLocalTime(&timeinfo, 5);
}

String formatTime12(int h, int m, bool withAmPm = true) {
  int displayHour = h % 12;
  if (displayHour == 0) displayHour = 12;
  char buf[16];
  if (withAmPm) {
    snprintf(buf, sizeof(buf), "%d:%02d %s", displayHour, m, (h >= 12) ? "PM" : "AM");
  } else {
    snprintf(buf, sizeof(buf), "%d:%02d", displayHour, m);
  }
  return String(buf);
}

String formatDate(struct tm &t) {
  static const char* wdays[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  static const char* mons[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  char buf[24];
  snprintf(buf, sizeof(buf), "%s, %s %d", wdays[t.tm_wday], mons[t.tm_mon], t.tm_mday);
  return String(buf);
}

void clearScreen() {
  tft.fillScreen(COL_BG);
}

void drawFooterHint(const char* left, const char* mid, const char* right) {
  tft.setTextDatum(BL_DATUM);
  tft.setTextColor(COL_DIM, COL_BG);
  tft.setTextSize(1);
  tft.drawString(left, 6, tft.height() - 4, 2);
  tft.setTextDatum(BC_DATUM);
  tft.drawString(mid, tft.width() / 2, tft.height() - 4, 2);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(right, tft.width() - 6, tft.height() - 4, 2);
}

void drawHome(bool fullRedraw) {
  struct tm t;
  bool haveTime = getTimeNow(t);

  if (fullRedraw) {
    clearScreen();
    drawFooterHint("", "", "Next >");
  }

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COL_TEXT, COL_BG);
  String timeStr = haveTime ? formatTime12(t.tm_hour, t.tm_min) : "--:--";
  tft.fillRect(0, 70, tft.width(), 60, COL_BG);
  tft.drawString(timeStr, tft.width() / 2, 100, 7);

  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(COL_DIM, COL_BG);
  tft.fillRect(0, 150, 150, 20, COL_BG);
  String dateStr = haveTime ? formatDate(t) : "--- -- --";
  tft.drawString(dateStr, 10, 150, 2);

  tft.fillRect(150, 145, tft.width() - 150, 30, COL_BG);
  if (alarmEnabled) {
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(COL_ALARM, COL_BG);
    String a = "@ " + formatTime12(alarmHour, alarmMinute);
    tft.drawString(a, tft.width() - 10, 150, 2);
  }
}

void drawTimeEditor(const char* title, int hourVal, int minVal, bool editingHour) {
  clearScreen();
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COL_ACCENT, COL_BG);
  tft.drawString(title, tft.width() / 2, 20, 4);

  tft.setTextDatum(MC_DATUM);
  char hbuf[3], mbuf[3];
  snprintf(hbuf, sizeof(hbuf), "%02d", hourVal);
  snprintf(mbuf, sizeof(mbuf), "%02d", minVal);

  int cx = tft.width() / 2;
  int cy = 110;

  tft.setTextColor(editingHour ? COL_OK : COL_DIM, COL_BG);
  tft.drawString(hbuf, cx - 45, cy, 7);

  tft.setTextColor(COL_DIM, COL_BG);
  tft.drawString(":", cx, cy, 7);

  tft.setTextColor(!editingHour ? COL_OK : COL_DIM, COL_BG);
  tft.drawString(mbuf, cx + 45, cy, 7);

  drawFooterHint("< Dec", "Next v", "Inc >");
}

void drawToggleScreen(const char* title, bool val) {
  clearScreen();
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COL_ACCENT, COL_BG);
  tft.drawString(title, tft.width() / 2, 20, 4);

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(val ? COL_OK : COL_OFF, COL_BG);
  tft.drawString(val ? "ON" : "OFF", tft.width() / 2, 110, 7);

  drawFooterHint("< Toggle", "Save v", "Toggle >");
}

void drawAlarmHome(bool fullRedraw) {
  if (fullRedraw) clearScreen();
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COL_ACCENT, COL_BG);
  tft.fillRect(0, 15, tft.width(), 30, COL_BG);
  tft.drawString("Alarm", tft.width() / 2, 20, 4);

  tft.setTextDatum(MC_DATUM);
  tft.fillRect(0, 70, tft.width(), 90, COL_BG);
  tft.setTextColor(alarmEnabled ? COL_OK : COL_DIM, COL_BG);
  tft.drawString(formatTime12(alarmHour, alarmMinute), tft.width() / 2, 100, 7);

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(alarmEnabled ? COL_OK : COL_OFF, COL_BG);
  tft.drawString(alarmEnabled ? "ENABLED" : "DISABLED", tft.width() / 2, 150, 2);

  drawFooterHint("< Home", "Edit", "Timer >");
}

void drawTimerHome(bool fullRedraw) {
  if (fullRedraw) clearScreen();
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COL_ACCENT, COL_BG);
  tft.fillRect(0, 15, tft.width(), 30, COL_BG);
  tft.drawString("Timer", tft.width() / 2, 20, 4);

  tft.setTextDatum(MC_DATUM);
  tft.fillRect(0, 70, tft.width(), 90, COL_BG);
  char buf[9];
  snprintf(buf, sizeof(buf), "%02d:%02d", timerSetHour, timerSetMinute);
  tft.setTextColor(COL_DIM, COL_BG);
  tft.drawString(buf, tft.width() / 2, 100, 7);

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COL_DIM, COL_BG);
  tft.drawString("Not running", tft.width() / 2, 150, 2);

  drawFooterHint("< Alarm", "Edit", "Stopwatch >");
}

void drawTimerRunning(bool fullRedraw) {
  if (fullRedraw) clearScreen();
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COL_ACCENT, COL_BG);
  tft.fillRect(0, 15, tft.width(), 30, COL_BG);
  tft.drawString("Timer Running", tft.width() / 2, 20, 4);

  long secs = timerRemainingSeconds;
  if (secs < 0) secs = 0;
  int hh = secs / 3600;
  int mm = (secs % 3600) / 60;
  int ss = secs % 60;
  char buf[12];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hh, mm, ss);

  tft.setTextDatum(MC_DATUM);
  tft.fillRect(0, 70, tft.width(), 60, COL_BG);
  tft.setTextColor(timerRinging ? COL_OFF : COL_OK, COL_BG);
  tft.drawString(buf, tft.width() / 2, 100, 6);

  drawFooterHint("< Alarm", "", "Stopwatch >");
}

void drawStopwatch(bool fullRedraw) {
  if (fullRedraw) clearScreen();
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COL_ACCENT, COL_BG);
  tft.fillRect(0, 15, tft.width(), 30, COL_BG);
  tft.drawString("Stopwatch", tft.width() / 2, 20, 4);

  unsigned long elapsed = stopwatchAccumMillis;
  if (stopwatchRunning) {
    elapsed += millis() - stopwatchStartMillis;
  }
  unsigned long totalSec = elapsed / 1000;
  int mm = (totalSec / 60) % 60;
  int ss = totalSec % 60;
  int cs = (elapsed % 1000) / 10;

  char buf[12];
  snprintf(buf, sizeof(buf), "%02d:%02d.%02d", mm, ss, cs);

  tft.setTextDatum(MC_DATUM);
  tft.fillRect(0, 70, tft.width(), 60, COL_BG);
  tft.setTextColor(stopwatchRunning ? COL_OK : COL_TEXT, COL_BG);
  tft.drawString(buf, tft.width() / 2, 100, 6);

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(COL_DIM, COL_BG);
  tft.fillRect(0, 150, tft.width(), 20, COL_BG);
  tft.drawString(stopwatchRunning ? "Running" : "Paused", tft.width() / 2, 150, 2);

  drawFooterHint("< Timer", "Start/Pause", "Home >");
}

void goToScreen(Screen s) {
  currentScreen = s;
  screenDirty = true;
}

void handleHomeScreen() {
  if (btnRight.pressedEvent) {
    goToScreen(SCR_ALARM_HOME);
  }
  if (alarmRinging && (btnLeft.pressedEvent || btnMiddle.pressedEvent || btnRight.pressedEvent)) {
    stopRinging(alarmRinging);
  }
}

void handleAlarmHome() {
  if (alarmRinging && (btnLeft.pressedEvent || btnMiddle.pressedEvent || btnRight.pressedEvent)) {
    stopRinging(alarmRinging);
    return;
  }
  if (btnLeft.pressedEvent) goToScreen(SCR_HOME);
  else if (btnRight.pressedEvent) goToScreen(SCR_TIMER_HOME);
  else if (btnMiddle.pressedEvent) goToScreen(SCR_ALARM_SET_HOUR);
}

void handleAlarmSetHour() {
  if (btnRight.pressedEvent) { alarmHour = (alarmHour + 1) % 24; screenDirty = true; buzzShort(); }
  if (btnLeft.pressedEvent)  { alarmHour = (alarmHour + 23) % 24; screenDirty = true; buzzShort(); }
  if (btnMiddle.pressedEvent) goToScreen(SCR_ALARM_SET_MIN);
}

void handleAlarmSetMin() {
  if (btnRight.pressedEvent) { alarmMinute = (alarmMinute + 1) % 60; screenDirty = true; buzzShort(); }
  if (btnLeft.pressedEvent)  { alarmMinute = (alarmMinute + 59) % 60; screenDirty = true; buzzShort(); }
  if (btnMiddle.pressedEvent) goToScreen(SCR_ALARM_SET_TOGGLE);
}

void handleAlarmSetToggle() {
  if (btnRight.pressedEvent || btnLeft.pressedEvent) { alarmEnabled = !alarmEnabled; screenDirty = true; buzzShort(); }
  if (btnMiddle.pressedEvent) goToScreen(SCR_ALARM_HOME);
}

void handleTimerHome() {
  if (timerRinging && (btnLeft.pressedEvent || btnMiddle.pressedEvent || btnRight.pressedEvent)) {
    stopRinging(timerRinging);
    return;
  }
  if (btnLeft.pressedEvent) goToScreen(SCR_ALARM_HOME);
  else if (btnRight.pressedEvent) goToScreen(SCR_STOPWATCH);
  else if (btnMiddle.pressedEvent) goToScreen(SCR_TIMER_SET_HOUR);
}

void handleTimerSetHour() {
  if (btnRight.pressedEvent) { timerSetHour = (timerSetHour + 1) % 24; screenDirty = true; buzzShort(); }
  if (btnLeft.pressedEvent)  { timerSetHour = (timerSetHour + 23) % 24; screenDirty = true; buzzShort(); }
  if (btnMiddle.pressedEvent) goToScreen(SCR_TIMER_SET_MIN);
}

void handleTimerSetMin() {
  if (btnRight.pressedEvent) { timerSetMinute = (timerSetMinute + 1) % 60; screenDirty = true; buzzShort(); }
  if (btnLeft.pressedEvent)  { timerSetMinute = (timerSetMinute + 59) % 60; screenDirty = true; buzzShort(); }
  if (btnMiddle.pressedEvent) {
    long totalSeconds = (long)timerSetHour * 3600 + (long)timerSetMinute * 60;
    if (totalSeconds <= 0) totalSeconds = 60;
    timerRemainingSeconds = totalSeconds;
    timerEndMillis = millis() + totalSeconds * 1000UL;
    timerRunning = true;
    timerRinging = false;
    goToScreen(SCR_TIMER_RUNNING);
  }
}

void handleTimerRunning() {
  if (timerRinging && (btnLeft.pressedEvent || btnMiddle.pressedEvent || btnRight.pressedEvent)) {
    stopRinging(timerRinging);
    timerRunning = false;
    goToScreen(SCR_TIMER_HOME);
    return;
  }
  if (btnLeft.pressedEvent) goToScreen(SCR_ALARM_HOME);
  else if (btnRight.pressedEvent) goToScreen(SCR_STOPWATCH);
}

void handleStopwatch() {
  if (btnLeft.pressedEvent) { goToScreen(SCR_TIMER_HOME); return; }
  if (btnRight.pressedEvent) { goToScreen(SCR_HOME); return; }

  if (middleDoublePressEvent) {
    stopwatchRunning = false;
    stopwatchAccumMillis = 0;
    screenDirty = true;
    buzzBeepBeep();
  } else if (btnMiddle.pressedEvent) {
    if (stopwatchRunning) {
      stopwatchAccumMillis += millis() - stopwatchStartMillis;
      stopwatchRunning = false;
    } else {
      stopwatchStartMillis = millis();
      stopwatchRunning = true;
    }
    screenDirty = true;
    buzzShort();
  }
}

int lastCheckedMinute = -1;

void checkAlarmTrigger() {
  if (!alarmEnabled || alarmRinging) return;
  struct tm t;
  if (!getTimeNow(t)) return;
  if (t.tm_min != lastCheckedMinute) {
    lastCheckedMinute = t.tm_min;
    if (t.tm_hour == alarmHour && t.tm_min == alarmMinute) {
      alarmRinging = true;
    }
  }
}

void updateTimerCountdown() {
  if (!timerRunning) return;
  long remainingMs = (long)(timerEndMillis - millis());
  if (remainingMs <= 0) {
    timerRemainingSeconds = 0;
    if (!timerRinging) {
      timerRinging = true;
    }
  } else {
    timerRemainingSeconds = remainingMs / 1000;
  }
  if (currentScreen == SCR_TIMER_RUNNING) screenDirty = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(LEFT_BTN_PIN, INPUT_PULLUP);
  pinMode(MIDDLE_BTN_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COL_BG);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(COL_TEXT, COL_BG);
  tft.drawString("Connecting WiFi...", tft.width() / 2, tft.height() / 2, 2);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 15000) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    configTzTime(TZ_INFO, NTP_SERVER);
    struct tm t;
    getLocalTime(&t, 10000);
  } else {
    tft.fillScreen(COL_BG);
    tft.drawString("WiFi failed - check creds", tft.width() / 2, tft.height() / 2, 2);
    delay(1500);
  }

  goToScreen(SCR_HOME);
}

void loop() {
  readButtons();
  checkAlarmTrigger();
  updateTimerCountdown();
  serviceRinging(alarmRinging);
  serviceRinging(timerRinging);

  switch (currentScreen) {
    case SCR_HOME:             handleHomeScreen();     break;
    case SCR_ALARM_HOME:       handleAlarmHome();      break;
    case SCR_ALARM_SET_HOUR:   handleAlarmSetHour();   break;
    case SCR_ALARM_SET_MIN:    handleAlarmSetMin();    break;
    case SCR_ALARM_SET_TOGGLE: handleAlarmSetToggle(); break;
    case SCR_TIMER_HOME:       handleTimerHome();      break;
    case SCR_TIMER_SET_HOUR:   handleTimerSetHour();   break;
    case SCR_TIMER_SET_MIN:    handleTimerSetMin();    break;
    case SCR_TIMER_RUNNING:    handleTimerRunning();   break;
    case SCR_STOPWATCH:        handleStopwatch();      break;
  }

  bool fullRedraw = (currentScreen != lastDrawnScreen);
  if (fullRedraw || screenDirty || currentScreen == SCR_HOME || currentScreen == SCR_STOPWATCH || currentScreen == SCR_TIMER_RUNNING) {
    switch (currentScreen) {
      case SCR_HOME:             drawHome(fullRedraw); break;
      case SCR_ALARM_HOME:       drawAlarmHome(fullRedraw); break;
      case SCR_ALARM_SET_HOUR:   drawTimeEditor("Set Alarm", alarmHour, alarmMinute, true); break;
      case SCR_ALARM_SET_MIN:    drawTimeEditor("Set Alarm", alarmHour, alarmMinute, false); break;
      case SCR_ALARM_SET_TOGGLE: drawToggleScreen("Alarm Enabled?", alarmEnabled); break;
      case SCR_TIMER_HOME:       drawTimerHome(fullRedraw); break;
      case SCR_TIMER_SET_HOUR:   drawTimeEditor("Set Timer", timerSetHour, timerSetMinute, true); break;
      case SCR_TIMER_SET_MIN:    drawTimeEditor("Set Timer", timerSetHour, timerSetMinute, false); break;
      case SCR_TIMER_RUNNING:    drawTimerRunning(fullRedraw); break;
      case SCR_STOPWATCH:        drawStopwatch(fullRedraw); break;
    }
    lastDrawnScreen = currentScreen;
    screenDirty = false;
  }

  delay(10);
}
