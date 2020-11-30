void lcd_setup() {
  lcd.begin(20, 4);

  lcd.createChar(0, hot_icon);
  lcd.createChar(1, run_icon_1);
  lcd.createChar(2, run_icon_2);
  lcd.createChar(3, celsius_icon);
  lcd.createChar(4, stop_icon);
  lcd.createChar(5, clock_icon);
  lcd.createChar(6, alarm_icon);
  lcd.setCursor(0, 0);
  lcd.print("Lievitone v0.1");
}

/**
   Gestione Layout LCD
*/
void lcd_manage() {
  if (is_modeset && !lcd_modeset_loaded) {
    lcd_modeset_layout();
    lcd_modeset_loaded = true;
  } else if (!is_modeset) {
    if (dht_error == true && prev_dht_error == false) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Errore Temp");

      prev_dht_error = true;
    } else if (lcd_modeset_loaded == true) {
      lcd_main_layout();

      lcd_modeset_loaded = false;
    } else if (dht_error == false && prev_dht_error == true) {
      lcd_main_layout();

      prev_dht_error = false;
    }
  }
}

/**
   Layout di default per LCD
*/
void lcd_main_layout() {
  lcd.clear();
  lcd.home();

  lcd.print("T:");
  lcd.setCursor(4, 0);
  lcd.write(3);
  lcd.setCursor(5, 0);
  lcd.print("S:");
  lcd.setCursor(9, 0);
  lcd.write(3);
  lcd.setCursor(11, 0);
  lcd.print("U:");
  lcd.setCursor(15, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.write(5);
  lcd.setCursor(3, 1);
  lcd.print(":");
  lcd.setCursor(7, 1);
  lcd.write(6);
  lcd.setCursor(10, 1);
  lcd.print(":");
}

/**
   Layout modalità Modeset
*/
void lcd_modeset_layout() {
  lcd.clear();
  lcd.home();
  lcd.print("Imposta Timer:");
  lcd.setCursor(0, 1);
  lcd.print("Ore: 00 Min: 00");
}

/**
   Aggiornamento valori LCD
*/
void lcd_update_values() {
  if (is_modeset) {

    lcd.setCursor(5, 1);
    lcd.print(pad(countdown_h));
    lcd.setCursor(13, 1);
    lcd.print(pad(countdown_m));

    if (modeset_selector == 0) {
      lcd.setCursor(4, 1);
      lcd.print(">");
      lcd.setCursor(12, 1);
      lcd.print(" ");
    } else {
      lcd.setCursor(4, 1);
      lcd.print(" ");
      lcd.setCursor(12, 1);
      lcd.print(">");
    }

  } else {
    lcd.setCursor(2, 0);
    lcd.print((int)temperature);
    lcd.setCursor(7, 0);
    lcd.print((int)setpoint);
    lcd.setCursor(13, 0);
    lcd.print((int)humidity);

    lcd.setCursor(1, 1);
    lcd.print(get_current_huors());
    lcd.setCursor(4, 1);
    lcd.print(get_current_minutes());

    lcd.setCursor(8, 1);
    /*lcd.print(pad(countdown_h));
      lcd.setCursor(11, 1);
      lcd.print(pad(countdown_m));*/

    lcd.setCursor(14, 1);
    if (heater_state) {
      lcd.write(byte(0));
    } else {
      lcd.print(" ");
    }
    lcd.setCursor(15, 1);
    lcd.write(run_icon_state);
  }
}
