#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Keypad.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11

#define IN13 7
#define IN24 8
#define ENAB 9

// Define keypad
const uint8_t ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

uint8_t rowPins[ROWS] = {14, 15, 16, 17};
uint8_t colPins[COLS] = {10, 11, 12, 13};

// Prototypes
DHT dht(DHTPIN, DHTTYPE);
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 20, 4);
SoftwareSerial pmsSerial(5, 6);

uint32_t DHTMillis, countdownMillis, pmReadMillis, printMillis;

int16_t speed_arr[4] = {0, 127, 191, 255};
int16_t index_arr = 3;
uint16_t pm2_5 = 0, currentPm2_5 = 0;

int8_t hour[2] = {0, 0};
int8_t minute[2] = {0, 0};
int8_t index_lcd = 6;

float currentHumidity, currentTemperature;

bool previousCountDown = false, settingTime = false, countdown = false;

void setup() {
  pinMode(IN13, OUTPUT);
  pinMode(IN24, OUTPUT);
  pinMode(ENAB, OUTPUT);
  // TCCR1B = TCCR1B & 0b11111000 | 0x02; // Set PWM frequency to 31250 Hz

  Serial.begin(9600);

  lcd.begin();
  lcd.backlight();

  dht.begin();

  lcd.setCursor(2, 1);
  lcd.print("Program Starting");
  lcd.setCursor(2, 2);
  lcd.print("Initializing");
  for (uint8_t count = 0; count <= 3; count++) {
    delay(500);
    lcd.print(".");
  }
  delay(1000);
  lcd.clear();

  lcd.setCursor(4, 0);
  lcd.print("AirPurifier");

  lcd.setCursor(0, 1);
  lcd.print("pm2.5: ");
  lcd.print("420");
  lcd.print(" ug/m3");

  lcd.setCursor(0, 2);
  lcd.print("RH:");
  lcd.print("    ");
  lcd.print("%");
  lcd.setCursor(9, 2);
  lcd.print("T:");
  lcd.print("    ");
  lcd.print((char)223);
  lcd.print("C");

  DISPLAY_TIME();

  lcd.setCursor(12, 3);
  lcd.print("FAN:");
  lcd.print(index_arr);
  while (!Serial);
  pmsSerial.begin(9600);
}

void loop() {
  uint32_t currentMillis = millis();

  if (currentMillis - DHTMillis >= 2000) {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    if (!isnan(humidity) && !isnan(temperature)) {
      currentHumidity = humidity;
      currentTemperature = temperature;
    }

    char humStr[10];
    char tempStr[10];

    dtostrf(currentHumidity, 4, 1, humStr);
    dtostrf(currentTemperature, 4, 1, tempStr);

    lcd.setCursor(0, 2);
    lcd.print("RH:");
    lcd.print(humStr);
    lcd.print("%");
    lcd.setCursor(9, 2);
    lcd.print("T:");
    lcd.print(tempStr);
    lcd.print((char)223);
    lcd.print("C");
    DHTMillis = currentMillis;
  }

  if (countdown != previousCountDown) {
    if (countdown == true) {
      DRIVE_MOTOR(HIGH, LOW, index_arr);
      countdownMillis = millis();
    } else {
      DRIVE_MOTOR(LOW, LOW, 0);
    }
    previousCountDown = countdown;
  }

  if (countdown && currentMillis - countdownMillis >= 1000) {
    COUNTDOWNTIME();
    DISPLAY_TIME();
    countdownMillis = currentMillis;
  }

  if (currentMillis - pmReadMillis >= 5000) {
    ReadPMS3003();
    pmReadMillis = currentMillis;
  }

  if (settingTime) {
    lcd.setCursor(index_lcd, 3);
    lcd.blink();
  }

//  if (currentMillis - printMillis >= 1000) {
//    PRINTSERIAL();
//    printMillis = currentMillis;
//  }
  char key = keypad.getKey();
  if (key != NO_KEY) {
    handleKeypadInput(key);
  }
}

// Functions
// Read PM2.5 value from PMS3003 sensor
void ReadPMS3003() {
  int8_t index_pm2_5 = 0;
  uint8_t value, previousValue;

  while (pmsSerial.available()) {
    value = pmsSerial.read();

    if ((index_pm2_5 == 0 && value != 0x42) || (index_pm2_5 == 1 && value != 0x4d)) {
      break;
    }

    if (index_pm2_5 == 6) {
      previousValue = value;
    }
    else if (index_pm2_5 == 7) {
      pm2_5 = (previousValue << 8) | value;
      currentPm2_5 = pm2_5;
      if (pm2_5 < 1000) {
        currentPm2_5 = pm2_5;
      }
      if (currentPm2_5 < 10) {
        lcd.setCursor(7, 1);
        lcd.print(currentPm2_5);
        lcd.setCursor(8, 1);
        lcd.print("            ");
        lcd.setCursor(9, 1);
        lcd.print("ug/m3");
      }
      else if (currentPm2_5 < 100) {
        lcd.setCursor(7, 1);
        lcd.print(currentPm2_5);
        lcd.setCursor(9, 1);
        lcd.print("           ");
        lcd.setCursor(10, 1);
        lcd.print("ug/m3");
      }
      else if (currentPm2_5 < 1000) {
        lcd.setCursor(7, 1);
        lcd.print(currentPm2_5);
        lcd.setCursor(10, 1);
        lcd.print("          ");
        lcd.setCursor(11, 1);
        lcd.print("ug/m3");
      }
      break;
    }

    index_pm2_5++;
  }
  while (pmsSerial.available()) pmsSerial.read();
}

// Handle keypad input
void handleKeypadInput(char key) {
  Serial.println(key);
  switch (key) {
    case 'A':
      if (settingTime == false) {
        countdown = !countdown;
      }
      break;

    case 'B':
      if (countdown == false) {
        settingTime = !settingTime;
        if (!settingTime) {
          index_lcd = 6;
          lcd.noBlink();
        }else{
          hour[0] = hour[1] = minute[0] = minute[1] = 0;
          DISPLAY_TIME();
        }
      }
      break;

    case '*':
      if (settingTime == true && index_lcd > 6) {
        index_lcd--;
        if (index_lcd == 8) index_lcd--;
      }
      break;

    case '#':
      if (settingTime == true && index_lcd < 10) {
        index_lcd++;
        if (index_lcd == 8) index_lcd++;
      }
      break;

    case 'C':
      if (index_arr < 3) index_arr++;
      if (countdown) {
        DRIVE_MOTOR(HIGH, LOW, index_arr);
      }else{
        DRIVE_MOTOR(LOW, LOW, 0);
      }
      lcd.setCursor(16, 3);
      lcd.print(index_arr);
      break;

    case 'D':
      if (index_arr > 0) index_arr--;
      if (countdown) {
        DRIVE_MOTOR(HIGH, LOW, index_arr);
      }else{
        DRIVE_MOTOR(LOW, LOW, 0);
      }
      lcd.setCursor(16, 3);
      lcd.print(index_arr);
      break;

    default:
      if (key >= '0' && key <= '9' && settingTime) {
        int8_t num = key - '0';
        int8_t index_check = index_lcd - 6;
        if (index_check == 0) {
          if (num < 3) {
            hour[0] = num;
          }
        } else if (index_check == 1) {
          if (hour[0] == 2 && num < 4) {
            hour[1] = num;
          } else if (hour[0] < 2) {
            hour[1] = num;
          }
        } else if (index_check == 3) {
          if (num < 6) {
            minute[0] = num;
          }
        } else if (index_check == 4) {
          minute[1] = num;
        }
        DISPLAY_TIME();
      }
      break;
  }
}

// Display time on LCD
void DISPLAY_TIME() {
  lcd.setCursor(0, 3);
  lcd.print("Time: ");
  lcd.print(hour[0]);
  lcd.print(hour[1]);
  lcd.print(":");
  lcd.print(minute[0]);
  lcd.print(minute[1]);
  lcd.print(" ");
}

void COUNTDOWNTIME() {
  if (hour[0] == 0 && hour[1] == 0 && minute[0] == 0 && minute[1] == 0) {
    countdown = false;
    return;
  }

  if (minute[1] == 0) {
    if (minute[0] == 0) {
      if (hour[1] == 0) {
        if (hour[0] == 0) {
          countdown = false;
          return;
        }
        hour[0]--;
        hour[1] = 9;
      } else {
        hour[1]--;
      }
      minute[0] = 5;
      minute[1] = 9;
    } else {
      minute[0]--;
      minute[1] = 9;
    }
  } else {
    minute[1]--;
  }
}

// Drive motor
void DRIVE_MOTOR(bool VCC, bool GND, uint8_t INDEX) {
  digitalWrite(IN13, VCC);
  digitalWrite(IN24, GND);
  analogWrite(ENAB, speed_arr[INDEX]);
}

void PRINTSERIAL(){
  Serial.print("pm2.5: ");
  Serial.print(currentPm2_5);
  Serial.print(" ug/m3, ");
  Serial.print("RH: ");
  Serial.print(currentHumidity);
  Serial.print("%, ");
  Serial.print("T: ");
  Serial.print(currentTemperature);
  Serial.print((char)223);
  Serial.print("C, ");
  Serial.print("FAN: ");
  Serial.print(index_arr);
  Serial.print(", ");
  Serial.print("Time: ");
  Serial.print(hour[0]);
  Serial.print(hour[1]);
  Serial.print(":");
  Serial.print(minute[0]);
  Serial.print(minute[1]);
  Serial.println();
}
