void rtc_setup() {
  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);

  // Reset
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  RTC.set(compileTime());
  setSyncProvider(RTC.get); 
  
  // set Alarm 1 to occur at 5 seconds after every minute
  RTC.setAlarm(ALM1_EVERY_SECOND, 0, 0, 0, 0);
  // clear the alarm flag
  RTC.alarm(ALARM_1);
  // enable interrupt output for Alarm 1
  RTC.alarmInterrupt(ALARM_1, true);
}

void rtc_check_alarm() {

  if ( !digitalRead(CLOCK_INTERRUPT_PIN) ) {
    RTC.alarm(ALARM_1);    // reset the alarm flag
    if(is_alarm == false) {
      lcd.setCursor(0, 2);
      lcd.print(is_alarm);     
      is_alarm = true;
    } else {
      lcd.setCursor(0, 2);
      lcd.print(is_alarm);
      is_alarm = false;
    }
  }
}
