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
int window_size = 5000, countdown_h, countdown_m, run_icon_state = 1, modeset_selector;
char buff[2];
bool dht_error, prev_dht_error = false, heater_state = false, is_running = false, is_modeset = false, lcd_modeset_loaded = false, is_alarm = false;
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
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  rtc_setup();
  ir_setup();
  lcd_setup();

  dht.begin();

  t_pid.SetOutputLimits(0, window_size);
  t_pid.SetMode(AUTOMATIC);

  delay(1000);

  lcd_main_layout();
  window_start_time = millis();
}

/**
   Main Loop \o/
*/
void loop() {
  lcd_buttons_manage();
  lcd_manage();
  ir_manage();
  rtc_check_alarm();

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

/**
    Gestione loop in modalita Modeset
*/
void thread_modeset() {

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
