#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "AlarmManager.h"

class DisplayManager
{
public:

    DisplayManager();

    void begin();

    void drawHome(
        const String& currentTime, int alarmHour, int alarmMinute, bool enabled);

    void drawAlarmView(int alarmHour, int alarmMinute, bool enabled);

    void drawAlarmEditHour(int alarmHour, int alarmMinute);

    void drawAlarmEditMinute(int alarmHour, int alarmMinute);

    void drawAlarmEditEnable(
        bool enabled);

private:

    TFT_eSPI tft;

    AlarmScreen lastScreen;

    String lastTime;

    int lastAlarmHour;
    int lastAlarmMinute;

    bool lastAlarmEnabled;

    void clearIfNeeded(AlarmScreen screen);

    void drawTitle(const String& title);

    void drawLargeTime(
    const String& time);

    void drawClock(
        const String& currentTime);
};