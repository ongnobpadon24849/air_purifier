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

const uint8_t ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

uint8_t rowPins[ROWS] = {19, 18, 17, 16};
uint8_t colPins[COLS] = {15, 14, 13, 12};

DHT dht(DHTPIN, DHTTYPE);
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 20, 4);
SoftwareSerial pmsSerial(5, 6);

uint32_t DHTMillis, countdownMillis, pmReadMillis;

int16_t speed_arr[4] = {0, 127, 191, 255};
int16_t index_arr = 3;
uint16_t pm2_5 = 0;

int8_t hour[2] = {0, 0};
int8_t minute[2] = {0, 0};
int8_t index_lcd = 6;

float currentHumidity, currentTemperature;

bool previousCountDown = false, settingTime = false, countdown = false;

char H[20], T[20];

void setup() {
  pinMode(IN13, OUTPUT);
  pinMode(IN24, OUTPUT);
  pinMode(ENAB, OUTPUT);
  TCCR1B = TCCR1B & 0b11111000 | 0x02;

  Serial.begin(115200);

  Serial.print("void setup");
  for (uint8_t count = 0; count <= 4; count++) {
    delay(1000);
    Serial.print((count != 4) ? "." : ".\n");
  }

  lcd.begin();
  lcd.backlight();
  Serial.println("LCD begin!");

  dht.begin();
  Serial.println("DHT11 begin!");

  lcd.setCursor(0, 0);
  lcd.print("AutomaticAirPurifier");
  DISPLAY_TIME();

  while (!Serial);
  pmsSerial.begin(9600);
}

void loop() {
  uint32_t currentMillis = millis();

  if (currentMillis - DHTMillis >= 2000) {
    DHTMillis = currentMillis;
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    if (!isnan(humidity) && !isnan(temperature)) {
      currentHumidity = humidity;
      currentTemperature = temperature;
    }
    dtostrf(currentHumidity, 4, 1, H);
    dtostrf(currentTemperature, 4, 1, T);

    lcd.setCursor(0, 2);
    lcd.print("RH: ");
    lcd.print(H);
    lcd.print("%");
    lcd.setCursor(10, 2);
    lcd.print("T: ");
    lcd.print(T);
    lcd.print((char)223);
    lcd.print("C");
    lcd.noBlink();
  }

  if (countdown != previousCountDown) {
    if (countdown) {
      DRIVE_MOTOR(HIGH, LOW, index_arr);
    } else {
      DRIVE_MOTOR(LOW, LOW, 0);
      hour[0] = hour[1] = minute[0] = minute[1] = 0;
    }
    previousCountDown = countdown;
  }

  if (countdown && currentMillis - countdownMillis >= 1000) {
    countdownMillis = currentMillis;
    COUNTDOWNTIME();
    DISPLAY_TIME();
  }

  if (currentMillis - pmReadMillis >= 1000) {
    pmReadMillis = currentMillis;
    ReadPMS3003();
  }

  if (settingTime) {
    lcd.setCursor(index_lcd, 3);
    lcd.blink();
  }

  char key = keypad.getKey();
  if (key != NO_KEY) {
    handleKeypadInput(key);
  }
}
