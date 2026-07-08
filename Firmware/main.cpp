#include <WiFi.h>
#include <time.h>

#include "AlarmManager.h"
#include "DisplayManager.h"


const char* ssid = "wifi name";
const char* password = "wifi password";


const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -5 * 3600;
const int daylightOffset_sec = 3600;

const int BTN_DEC = 2;
const int BTN_MODE = 21;
const int BTN_INC = 9;

DisplayManager display;
AlarmManager alarm;

String currentTime = "";


bool lastMode = HIGH;
bool lastInc = HIGH;
bool lastDec = HIGH;



void connectWifi()
{
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
    }
}

void setupTime()
{
    configTime(
        gmtOffset_sec,
        daylightOffset_sec,
        ntpServer
    );
}

void updateTime()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
        return;

    char buffer[9];

    strftime(buffer,
             sizeof(buffer),
             "%H:%M:%S",
             &timeinfo);

    currentTime = String(buffer);
}

void readButtons()
{
    bool mode = digitalRead(BTN_MODE);
    bool inc = digitalRead(BTN_INC);
    bool dec = digitalRead(BTN_DEC);

    if(lastMode == HIGH && mode == LOW)
        alarm.modePressed();

    if(lastInc == HIGH && inc == LOW)
        alarm.increasePressed();

    if(lastDec == HIGH && dec == LOW)
        alarm.decreasePressed();

    lastMode = mode;
    lastInc = inc;
    lastDec = dec;
}

void drawCurrentScreen()
{
    switch(alarm.getScreen())
    {

    case HOME:

        display.drawHome(
            currentTime,
            alarm.getHour(),
            alarm.getMinute(),
            alarm.isEnabled());

        break;

    case ALARM_VIEW:

        display.drawAlarmView(
            alarm.getHour(),
            alarm.getMinute(),
            alarm.isEnabled());

        break;

    case ALARM_EDIT_HOUR:

        display.drawAlarmEditHour(
            alarm.getHour(),
            alarm.getMinute());

        break;

    case ALARM_EDIT_MINUTE:

        display.drawAlarmEditMinute(
            alarm.getHour(),
            alarm.getMinute());

        break;

    case ALARM_EDIT_ENABLE:

        display.drawAlarmEditEnable(
            alarm.isEnabled());

        break;

    }

}

void setup()
{
    pinMode(BTN_DEC, INPUT_PULLUP);
    pinMode(BTN_MODE, INPUT_PULLUP);
    pinMode(BTN_INC, INPUT_PULLUP);

    display.begin();

    connectWifi();

    setupTime();
}

void loop()
{
    updateTime();

    readButtons();

    drawCurrentScreen();

    delay(50);
}