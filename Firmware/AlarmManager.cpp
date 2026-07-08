#include "AlarmManager.h"

AlarmManager::Alarmmanager()
{
  screen = HOME;

  alarmHour = 7;
  alarmMinute= 30;

  enabled = false;
}

AlarmScreen Alarmmanager::getScreen()
{
  return screen;
}

int AlarmManager::getHour()
{
  return alarmHour;
}

int Alarmmanager::getMinute()
{
  return alarmMinute;
}

bool AlarmManager::isEnabled()
{
  return enabled;
}

void AlarmManager::modePressed()
{
  switch(screen)
  {

    case HOME:

      if(enabled)
        screen = ALARM_VIEW;
      else
        screen=ALARM_EDIT_HOUR;
      break;

    case ALARM_VIEW:

      if(enabled)
        screen = HOME;
      else
        screen = ALARM_EDIT_HOUR;
      break;
    
    case ALARM_EDIT_HOUR:

      screen = ALARM_EDIT_MINUTE;
      break;
    
    case ALARM_EDIT_MINUTE:

      screen = ALARM_EDIT_ENABLE;
      break;

    case ALARM_EDIT_ENABLE:

      screen = ALARM_VIEW;
      break;
  }
}

void AlarmManager::increasePressed()
{
  switch(screen)
  {
    case ALARM_EDIT_HOUR:
      alarmHour++;

      if(alarmHour>23)
        alarmHour=0;
      break;

    case ALARM_EDIT_MINUTE:

      alarmMinute++;

      if(alarmMinute>59)
        alarmMinute=0;
      break;

    case ALARM_EDIT_ENABLE:

      enabled = !enabled;
      break;
    default:
    break;
  }
}

void AlarmManager::decreasePressed()
{
  switch(screen)
  {
    case ALARM_EDIT_HOUR:
    alarmHour--;
    if(alarmHour<0)
      alarmHour = 23;
    break;

    case ALARM_EDIT_MINUTE:
      alarmMinute--;

      if(alarmMinute<0)
        alarmMinute = 59;

      break;
    
    case ALARM_EDIT_ENABLE:

      enabled = !enabled;
      break;

    default:
      break;
  }
}