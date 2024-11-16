#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <RBDdimmer.h>//

#define outputPin  9
#define zerocross  2
dimmerLamp dimmer(outputPin);
int dimmer_arr[3] = {50, 75, 100};
int index_dimmer = 3;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {14, 15, 8, 7};
byte colPins[COLS] = {6, 5, 4, 3};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
DHT dht;
LiquidCrystal_I2C lcd(0x27, 20, 4);
SoftwareSerial mySerial(12, 13);

unsigned long DHTMillis = 0;
unsigned long countdownMillis = 0;
unsigned long pmReadMillis = 0;

long interval = dht.getMinimumSamplingPeriod();

int hour[2] = {0, 0};
int minute[2] = {0, 0};
int index_lcd = 6;

float currentHumidity;
float currentTemperature;

bool previousCountDown = false;
bool settingTime = false;
bool countdown = false;

char H[20];
char T[20];

unsigned int pm2_5 = 0;

void setup() {
  dimmer.begin(NORMAL_MODE, ON);
  lcd.begin();
  dht.setup(2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("AutomaticAirPurifier");
  TIMELCD();

  Serial.begin(9600);
  while (!Serial);
  mySerial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();
  interval = dht.getMinimumSamplingPeriod();
  // Read DHT Sensor
  if (currentMillis - DHTMillis >= interval) {
    DHTMillis = currentMillis;
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
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
      dimmer.setPower(dimmer_arr[index_dimmer]);
    } else {
      dimmer.setPower(0);
      hour[0] = hour[1] = minute[0] = minute[1] = 0;
    }
    previousCountDown = countdown;
  }

  if (countdown && currentMillis - countdownMillis >= 1000) {
    countdownMillis = currentMillis;
    COUNTDOWNTIME();
    TIMELCD();
  }

  if (currentMillis - pmReadMillis >= 1000) {
    pmReadMillis = currentMillis;
    ReadPM2_5();
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

void ReadPM2_5() {
  int index_pm2_5 = 0;
  char value;
  char previousValue;

  while (mySerial.available()) {
    value = mySerial.read();

    if ((index_pm2_5 == 0 && value != 0x42) || (index_pm2_5 == 1 && value != 0x4d)) {
      Serial.println("Cannot find the data header.");
      break;
    }

    if (index_pm2_5 == 6) {
      previousValue = value;
    }
    else if (index_pm2_5 == 7) {
      pm2_5 = 256 * previousValue + value;
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 1);
      lcd.print("pm2.5: ");
      lcd.print(pm2_5);
      lcd.print(" ug/m3");
      break;
    }

    index_pm2_5++;
  }
  while (mySerial.available()) mySerial.read();
}

void handleKeypadInput(char key) {
  if (key == 'A' && !settingTime) {
    countdown = !countdown;
    if (countdown) {
      countdownMillis = millis();
    }
  }

  if (key == 'B' && !countdown) {
    settingTime = !settingTime;
    if (!settingTime) {
      index_lcd = 6;
      lcd.noBlink();
    }
  }

  if (key >= '0' && key <= '9' && settingTime) {
    int num = key - '0';
    int index_check = index_lcd - 6;

    if (index_check == 0 && num <= 2) {
      hour[0] = num;
    } else if (index_check == 1 && (hour[0] < 2 || (hour[0] == 2 && num <= 3))) {
      hour[1] = num;
    } else if (index_check == 3 && num <= 5) {
      minute[0] = num;
    } else if (index_check == 4 && num <= 9) {
      minute[1] = num;
    }
    TIMELCD();
  }

  if (key == '*' && settingTime) {
    if (index_lcd > 6) index_lcd--;
    if (index_lcd == 8) index_lcd--;
  }

  if (key == '#' && settingTime) {
    if (index_lcd < 10) index_lcd++;
    if (index_lcd == 8) index_lcd++;
  }

  if (key == 'C') {
    if (index_dimmer < 3) index_dimmer++;
    if (countdown)dimmer.setPower(dimmer_arr[index_dimmer]);
  }

  if (key == 'D') {
    if (index_dimmer > 0) index_dimmer--;
    if (countdown)dimmer.setPower(dimmer_arr[index_dimmer]);
  }
}

void TIMELCD() {
  lcd.setCursor(0, 3);
  lcd.print("                ");
  lcd.setCursor(0, 3);
  lcd.print("Time: ");
  lcd.print(hour[0]);
  lcd.print(hour[1]);
  lcd.print(":");
  lcd.print(minute[0]);
  lcd.print(minute[1]);
}

void COUNTDOWNTIME() {
  if (minute[1] > 0) {
    minute[1]--;
  } else {
    if (minute[0] > 0) {
      minute[0]--;
      minute[1] = 9;
    } else {
      if (hour[1] > 0) {
        hour[1]--;
        minute[0] = 5;
        minute[1] = 9;
      } else if (hour[0] > 0) {
        hour[0]--;
        hour[1] = 9;
        minute[0] = 5;
        minute[1] = 9;
      } else {
        countdown = false;
      }
    }
  }
}
