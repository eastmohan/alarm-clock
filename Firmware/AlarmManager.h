#pragma once
enum AlarmScreen
{
  HOME,
  ALARM_VIEW,
  ALARM_EDIT_HOUR,
  ALARM_EDIT_MINUTE,
  ALARM_EDIT_ENABLE
};

class AlarmManager
{
  public:
    AlarmManager();

    void modePressed();
    void increasePressed();
    void decreasePressed();

    AlarmScreen getScreen();

    int getHour();
    int getMinute();
    bool isEnabled();

  private:

    AlarmScreen screen;

    int alarmHour;
    int alarmMinute;

    bool enabled;
};