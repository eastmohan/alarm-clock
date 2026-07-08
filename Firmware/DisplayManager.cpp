#include "DisplayManager.h"


DisplayManager::DisplayManager()
{
    lastScreen = HOME;

    lastTime = "";

    lastAlarmHour = -1;
    lastAlarmMinute = -1;

    lastAlarmEnabled = false;
}


void DisplayManager::begin()
{
    tft.init();

    tft.setRotation(1);

    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(
        TFT_WHITE,
        TFT_BLACK
    );

    tft.setTextDatum(MC_DATUM);

    lastScreen = HOME;
}


void DisplayManager::clearIfNeeded(AlarmScreen screen)
{
    if(lastScreen != screen)
    {
        tft.fillScreen(TFT_BLACK);

        lastTime = "";

        lastAlarmHour = -1;
        lastAlarmMinute = -1;
        lastAlarmEnabled = false;

        lastScreen = screen;
    }
}


void DisplayManager::drawTitle(const String& text)
{
    tft.setTextDatum(TC_DATUM);

    tft.setTextFont(4);

    tft.setTextColor(
        TFT_CYAN,
        TFT_BLACK
    );

    tft.drawString(
        text,
        160,
        10
    );

    tft.setTextColor(
        TFT_WHITE,
        TFT_BLACK
    );
}


void DisplayManager::drawClock(const String& currentTime)
{
    tft.setTextDatum(MC_DATUM);

    tft.setTextFont(7);

    tft.fillRect(
        20,
        65,
        280,
        70,
        TFT_BLACK
    );

    tft.drawString(
        currentTime,
        160,
        100
    );
}


void DisplayManager::drawLargeTime(
    const String& time
)
{
    drawClock(time);
}


void DisplayManager::drawHome(
    const String& currentTime,
    int alarmHour,
    int alarmMinute,
    bool enabled
)
{

    clearIfNeeded(HOME);


    if(lastTime != currentTime)
    {
        drawClock(currentTime);

        lastTime = currentTime;
    }


    if(
        lastAlarmHour != alarmHour ||
        lastAlarmMinute != alarmMinute ||
        lastAlarmEnabled != enabled
    )
    {

        tft.fillRect(
            0,
            165,
            320,
            70,
            TFT_BLACK
        );


        tft.setTextDatum(MC_DATUM);

        tft.setTextFont(4);


        if(enabled)
        {

            char buffer[6];

            sprintf(
                buffer,
                "%02d:%02d",
                alarmHour,
                alarmMinute
            );


            tft.drawString(
                "ALARM",
                160,
                170
            );


            tft.drawString(
                buffer,
                160,
                205
            );

        }
        else
        {

            tft.drawString(
                "ALARM OFF",
                160,
                190
            );

        }


        lastAlarmHour = alarmHour;
        lastAlarmMinute = alarmMinute;
        lastAlarmEnabled = enabled;
    }

}



void DisplayManager::drawAlarmView(
    int alarmHour,
    int alarmMinute,
    bool enabled
)
{

    clearIfNeeded(ALARM_VIEW);


    drawTitle("ALARM");


    char buffer[6];

    sprintf(
        buffer,
        "%02d:%02d",
        alarmHour,
        alarmMinute
    );


    tft.setTextDatum(MC_DATUM);

    tft.setTextFont(7);


    tft.drawString(
        buffer,
        160,
        100
    );


    tft.setTextFont(4);


    if(enabled)
    {
        tft.drawString(
            "ON",
            160,
            170
        );
    }
    else
    {
        tft.drawString(
            "OFF",
            160,
            170
        );
    }


    tft.setTextFont(2);

    tft.drawString(
        "MODE = EDIT",
        160,
        220
    );

}



void DisplayManager::drawAlarmEditHour(
    int alarmHour,
    int alarmMinute
)
{

    clearIfNeeded(ALARM_EDIT_HOUR);


    drawTitle("SET HOUR");


    char buffer[6];

    sprintf(
        buffer,
        "%02d:%02d",
        alarmHour,
        alarmMinute
    );


    tft.setTextDatum(MC_DATUM);

    tft.setTextFont(7);


    tft.drawString(
        buffer,
        160,
        100
    );


    // highlight hour
    tft.drawFastHLine(
        75,
        145,
        70,
        TFT_GREEN
    );


    tft.setTextFont(2);


    tft.drawString(
        "+ / - CHANGE",
        160,
        210
    );


    tft.drawString(
        "MODE NEXT",
        160,
        230
    );

}



void DisplayManager::drawAlarmEditMinute(
    int alarmHour,
    int alarmMinute
)
{

    clearIfNeeded(ALARM_EDIT_MINUTE);


    drawTitle("SET MINUTE");


    char buffer[6];

    sprintf(
        buffer,
        "%02d:%02d",
        alarmHour,
        alarmMinute
    );


    tft.setTextDatum(MC_DATUM);

    tft.setTextFont(7);


    tft.drawString(
        buffer,
        160,
        100
    );


    // highlight minute
    tft.drawFastHLine(
        175,
        145,
        70,
        TFT_GREEN
    );


    tft.setTextFont(2);


    tft.drawString(
        "+ / - CHANGE",
        160,
        210
    );


    tft.drawString(
        "MODE NEXT",
        160,
        230
    );

}



void DisplayManager::drawAlarmEditEnable(
    bool enabled
)
{

    clearIfNeeded(ALARM_EDIT_ENABLE);


    drawTitle("ENABLE ALARM");


    tft.setTextDatum(MC_DATUM);

    tft.setTextFont(7);


    if(enabled)
    {
        tft.drawString(
            "ON",
            160,
            110
        );
    }
    else
    {
        tft.drawString(
            "OFF",
            160,
            110
        );
    }


    tft.setTextFont(2);


    tft.drawString(
        "+ / - TOGGLE",
        160,
        190
    );


    tft.drawString(
        "MODE SAVE",
        160,
        220
    );

}