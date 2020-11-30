#include <LiquidCrystal.h>
#include <IRremote.h>

#include <Wire.h>
#include <DS3232RTC.h>
#include "DHT.h"
#include <PID_v1.h>


#define CLOCK_INTERRUPT_PIN 2

#define LCD_EN_PIN 8
#define LCD_RS_PIN 12
#define RELAY_PIN 9
#define IR_PIN 10
#define DHT_PIN 11
#define ANALOG_BUTTONS_PIN A0

#define DHT_TYPE DHT11
#define Kp 2
#define Ki 5
#define Kd 1
#define TEMP_THRESHOLD 10

double setpoint = 24, temperature, humidity, pid_output;
unsigned long timer, th1 = 0, window_start_time, key_value = 0;
int window_size = 5000, countdown_h, countdown_m;
char buff[2];
bool dht_error, prev_dht_error = false, heater_state = false, is_running = false, is_modeset = false, lcd_modeset_loaded = false;
int run_icon_state = 1, modeset_selector;
byte hot_icon[] = { B01001, B10010, B01001, B10010, B01001, B10010, B01001, B11111 };
byte run_icon_1[] = { B00000, B00100, B00100, B11111, B00100, B00100, B00000, B00000 };
byte run_icon_2[] = { B00000, B10001, B01010, B00100, B01010, B10001, B00000, B00000 };
byte celsius_icon[] = { B01100, B10010, B10010, B01100, B00000, B00000, B00000, B00000 };
byte stop_icon[] = { B00000, B11111, B11111, B11111, B11111, B11111, B00000, B00000 };
byte alarm_icon[] = { B11011, B11011, B01110, B11001, B10101, B10001, B01110, B00000 };
byte clock_icon[] = { B00000, B00000, B01110, B11001, B10111, B10001, B01110, B00000 };

LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, 4, 5, 6, 7);
IRrecv irrecv(IR_PIN);
decode_results results;
DHT dht(DHT_PIN, DHT_TYPE);
PID t_pid(&temperature, &pid_output, &setpoint, Kp, Ki, Kd, DIRECT);


/**
   Setup
*/
void setup() {
  //Serial.begin(115200);
  setup_rtc();

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

  irrecv.enableIRIn();
  irrecv.blink13(true);

  dht.begin();

  t_pid.SetOutputLimits(0, window_size);
  t_pid.SetMode(AUTOMATIC);

  delay(1000);

  lcd_main_layout();
  setupPin();
  window_start_time = millis();
}

/**
   Main Loop \o/
*/
void loop() {
  lcd_manage();
  ir_manage();
  check_alarm();

  if (is_modeset) {
    thread_modeset();
  } else {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    if (is_running) {
      thread_running();
    } else {
      thread_stop();
    }
  }

  lcd_update_values();
}

/**
   Gestione loop quando attivo
*/
void thread_running() {
  timer = millis();

  if (timer - th1 >= 1500) {
    th1 = timer;

    if (run_icon_state == 1) {
      run_icon_state = 2;
    } else {
      run_icon_state = 1;
    }
  }

  // Verifico la lettura della temperatura
  if (isnan(temperature) || (temperature < TEMP_THRESHOLD)) {
    dht_error = true;
    return;
  } else {
    dht_error = false;
  }

  t_pid.Compute();

  // turn the output pin on/off based on pid output
  if (timer - window_start_time > window_size) {
    // time to shift the Relay Window
    window_start_time += window_size;
  }

  heater_state = (pid_output > (timer - window_start_time));
  digitalWrite(RELAY_PIN, !heater_state);
}

/**
   Gestione loop quanto in pausa
*/
void thread_stop() {
  run_icon_state = 4;

}

void ir_manage() {
  if (irrecv.decode(&results)) {

    if (results.value == 0XFFFFFFFF)
      results.value = key_value;

    switch (results.value) {
      case 0xFFA25D:
        //Serial.println("CH-");
        break;
      case 0xFF629D:
        //Serial.println("CH");
        break;
      case 0xFFE21D:
        //Serial.println("CH+");
        break;
      case 0xFF22DD:
        //Serial.println("|<<");
        break;
      case 0xFF02FD:
        //Serial.println(">>|");
        break ;
      case 0xFFC23D:
        //Serial.println(">|");
        onStartPressed(LOW);
        break ;
      case 0xFFE01F:
        //Serial.println("-");
        onDecPressed(LOW);
        break ;
      case 0xFFA857:
        //Serial.println("+");
        onIncPressed(LOW);
        break ;
      case 0xFF906F:
        //Serial.println("EQ");
        onModesetPressed(LOW);
        break ;
      case 0xFF6897:
        //Serial.println("0");
        break ;
      case 0xFF9867:
        //Serial.println("100+");
        break ;
      case 0xFFB04F:
        //Serial.println("200+");
        break ;
      case 0xFF30CF:
        //Serial.println("1");
        break ;
      case 0xFF18E7:
        //Serial.println("2");
        break ;
      case 0xFF7A85:
        //Serial.println("3");
        break ;
      case 0xFF10EF:
        //Serial.println("4");
        break ;
      case 0xFF38C7:
        //Serial.println("5");
        break ;
      case 0xFF5AA5:
        //Serial.println("6");
        break ;
      case 0xFF42BD:
        //Serial.println("7");
        break ;
      case 0xFF4AB5:
        //Serial.println("8");
        break ;
      case 0xFF52AD:
        //Serial.println("9");
        break ;
    }
    key_value = results.value;
    irrecv.resume();
  }
}

/**
    Gestione loop in modalita Modeset
*/
void thread_modeset() {

}

/**
   Setup PIN Arduino
*/
void setupPin() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
}

/**
   Callback
*/
void onIncPressed(const int state) {

  if (state == LOW) {
    if (is_modeset) {
      if (modeset_selector == 0) {
        countdown_h = countdown_h + 1;
      } else {
        if (countdown_m < 45) {
          countdown_m = countdown_m + 15;
        }
      }

    } else if (!is_running) {
      setpoint = setpoint + 1;
    }
  }
}
/**
   Callback
*/
void onDecPressed(const int state) {
  if (state == LOW) {
    if (is_modeset) {
      if (modeset_selector == 0) {
        if (countdown_h > 0) {
          countdown_h = countdown_h - 1;
        }
      } else {
        if (countdown_m > 0) {
          countdown_m = countdown_m - 15;
        }
      }
    } else if (!is_running) {
      setpoint = setpoint - 1;
    }
  }
}
/**
   Callback
*/
void onStartPressed(const int state) {

  if (state == LOW) {
    if (is_modeset) {
      modeset_selector++;
      if (modeset_selector > 1) {
        modeset_selector = 0;
      }

    } else {
      is_running = !is_running;
    }
  }
}

/**
   Callback
*/
void onModesetPressed(const int state) {
  if (state == LOW) is_modeset = !is_modeset;
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

/**
   Helper formattazione Minuti
*/
String pad(int n) {
  sprintf(buff, "%02d", n);

  return String(buff);
}
