unsigned long debounce_lcd;

int lcd_buttons_manage()
{
  int adc_key_value = analogRead(0);

  if (inRange(adc_key_value, 100, 706)) {
    if (millis() - debounce_lcd > 250) {

      if (inRange(adc_key_value, 100, 130))   RTC.clearAlarm(ALARM_1);  // BACK
      if (inRange(adc_key_value, 176, 196))   onDecPressed(LOW);     // DOWN
      if (inRange(adc_key_value, 310, 340))   onModesetPressed(LOW); // MENU
      if (inRange(adc_key_value, 503, 533))   onStartPressed(LOW);  // START
      if (inRange(adc_key_value, 686, 706))   onIncPressed(LOW);    // UP

      debounce_lcd = millis();
    }
  }
  
  return 1;  
}
