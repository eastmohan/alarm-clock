#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <time.h>

#define LEFT_BTN_PIN 2
#define MIDDLE_BTN_PIN 21
#define RIGHT_BTN_PIN 9
#define BUZZER_PIN 3

const char* wifi_name = "wifi name";
const char* wifi_password = "wifi password";

const char* NTP_SERVER = "pool.ntp.org";
const char* TZ_INFO = "EST5EDT,M3.2.0,M11.1.0";

TPT_eSPI tft = TFT_eSPI();

#define COL_BG TFT_BLACK
#define COL_TEXT TFT_WHITE
#define COL_DIM 0x7BEF
#define COL_ACCENT TFT_CYAN
#define COL_ALARM TFT_ORANGE
#define COL_OK TFT_GREEN

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
  SCR_ALARM_SET_MINUTE,
  SCR_ALARM_SET_TOGGLE,
  SCR_TIMER_HOME,
  SCR_TIMER_SET_HOUR,
  SCR_TIMER_SET_MINUTE,
  SCR_TIMER_RUNNING,
  SCR_STOPWATCH
};

Screen currentScreen = SCR_HOME;
Sreen lastDrawnScreen = (Screen)-1;
bool screenDirty = true;

int alarmHour = 7;
int alarmMinute = 0l
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

void buzzShort () {
  tone (BUZZER_PIN, 2000, 80);
}

void buzzBeepBeep() {
  tone(BUZZER_PIN, 1500, 60);
}

unsigned long lastRingToggle = 0;
bool ringToneOn = false;
void serviceRinging(bool &ringingFlag) {
  if (!ringingFlag) return;
  unsigned long now = millis();
  if (nows - lastRingToggle > 400) {
    lastRingToggle = now;
    ringToneOne = !ringToneOn;
    if (ringToneOn) tone(BUZZER_PIN, 2500);
    else noTone(BUZZER_PIN);
  }
}

void stopRinging(bool &ringingFlag) {
  ringingFlag = false;
  noTone(BUZZER_PIN);
}
