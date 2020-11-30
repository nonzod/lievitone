void setup_rtc() {
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet) {
    lcd.setCursor(1, 1);
    lcd.print("Err.");
  }

  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);
  // set Alarm 2 for 06:30:00
  RTC.setAlarm(ALM2_MATCH_HOURS, 15, 20, 10, 0);
  // clear the alarm flags
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  // configure the INT/SQW pin for "interrupt" operation (disable square wave output)
  RTC.squareWave(SQWAVE_NONE);
  // enable interrupt output for Alarm 2 only
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, true);
}

String get_current_huors() {
  return pad(hour());
}

String get_current_minutes() {
  return pad(minute());
}

void check_alarm() {
  if ( !digitalRead(CLOCK_INTERRUPT_PIN) ) {
    RTC.alarm(ALARM_2);    // reset the alarm flag
    lcd.setCursor(8, 1);
    lcd.print("DRIIN");
  }
}
