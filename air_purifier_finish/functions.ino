//void ReadPMS3003() {
//  int8_t index_pm2_5 = 0;
//  uint8_t value, previousValue;
//
//  while (pmsSerial.available()) {
//    value = pmsSerial.read();
//
//    if ((index_pm2_5 == 0 && value != 0x42) || (index_pm2_5 == 1 && value != 0x4d)) {
//      break;
//    }
//
//    if (index_pm2_5 == 6) {
//      previousValue = value;
//    }
//    else if (index_pm2_5 == 7) {
//      pm2_5 = (previousValue << 8) | value;
//      currentPm2_5 = pm2_5;
//      if (pm2_5 < 1000) {
//        currentPm2_5 = pm2_5;
//      }
//      lcd.setCursor(0, 1);
//      lcd.print("                    ");
//      lcd.setCursor(0, 1);
//      lcd.print("pm2.5: ");
//      lcd.print(currentPm2_5);
//      lcd.print(" ug/m3");
//      pmReadMillis = currentMillis;
//      break;
//    }
//
//    index_pm2_5++;
//  }
//  while (pmsSerial.available()) pmsSerial.read();
//}
//
//
//void handleKeypadInput(char key) {
//  switch (key) {
//    case 'A':
//      if (settingTime == false) {
//        countdown = !countdown;
//        if (countdown) {
//          countdownMillis = millis();
//        }
//      }
//      break;
//
//    case 'B':
//      if (countdown == false) {
//        settingTime = !settingTime;
//        if (!settingTime) {
//          index_lcd = 6;
//          lcd.noBlink();
//        }
//      }
//      break;
//
//    case '*':
//      if (settingTime && index_lcd > 6) {
//        index_lcd--;
//        if (index_lcd == 8) index_lcd--;
//      }
//      break;
//
//    case '#':
//      if (settingTime && index_lcd < 10) {
//        index_lcd++;
//        if (index_lcd == 8) index_lcd++;
//      }
//      break;
//
//    case 'C':
//      if (index_arr < 3) index_arr++;
//      break;
//
//    case 'D':
//      if (index_arr > 0) index_arr--;
//      break;
//
//    default:
//      if (key >= '0' && key <= '9' && settingTime) {
//        int8_t num = key - '0';
//        int8_t index_check = index_lcd - 6;
//
//        if (index_check == 0 && num <= 2) {
//          hour[0] = num;
//        } else if (index_check == 1 && (hour[0] < 2 || (hour[0] == 2 && num <= 3))) {
//          hour[1] = num;
//        } else if (index_check == 3 && num <= 5) {
//          minute[0] = num;
//        } else if (index_check == 4 && num <= 9) {
//          minute[1] = num;
//        }
//        DISPLAY_TIME();
//      }
//      break;
//  }
//}
//
//
//void DISPLAY_TIME() {
//  lcd.setCursor(0, 3);
//  lcd.print("Time: ");
//  lcd.print(hour[0]);
//  lcd.print(hour[1]);
//  lcd.print(":");
//  lcd.print(minute[0]);
//  lcd.print(minute[1]);
//  lcd.print("  ");
//}
//
//void COUNTDOWNTIME() {
//  if (minute[1] > 0) {
//    minute[1]--;
//  } else {
//    if (minute[0] > 0) {
//      minute[0]--;
//      minute[1] = 9;
//    } else {
//      if (hour[1] > 0) {
//        hour[1]--;
//        minute[0] = 5;
//        minute[1] = 9;
//      } else if (hour[0] > 0) {
//        hour[0]--;
//        hour[1] = 9;
//        minute[0] = 5;
//        minute[1] = 9;
//      } else {
//        countdown = false;
//      }
//    }
//  }
//}
//
//void DRIVE_MOTOR(bool VCC, bool GND, uint8_t INDEX) {
//  digitalWrite(IN13, VCC);
//  digitalWrite(IN24, GND);
//  analogWrite(ENAB, speed_arr[INDEX]);
//}
