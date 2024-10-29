#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
#define S0_PIN 2// mau sawx
#define S1_PIN 3
#define S2_PIN 4
#define S3_PIN 5
#define OUT_PIN 6
#define BTN_PIN_1 7  // nut training
#define BTN_PIN_2 8   // nut training
#define BTN_PIN_display 9 // nuts bat tat man
#define Relay_quat 10  // relay2
#define Relay_dongco 11
#define IR1 12 // Hong ngoai
#define IR2 13
 // 2 servo
const int maxColors = 70;
int trainedR[maxColors], trainedG[maxColors], trainedB[maxColors];
int colorCount = 0, display_count = 0;
int eepromAddress = 0;
int qua_xanh = 0, qua_chin = 0;
bool isStart = false;
void setup() {
  lcd.init();
  pinMode(S0_PIN, OUTPUT);
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(S3_PIN, OUTPUT);
  pinMode(OUT_PIN, INPUT);
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(Relay_dongco, OUTPUT);
  pinMode(Relay_quat, OUTPUT);
  pinMode(BTN_PIN_1, INPUT_PULLUP);
  pinMode(BTN_PIN_2, INPUT_PULLUP);
  pinMode(BTN_PIN_display, INPUT_PULLUP);
  /// pinMode(BTN_PIN_reset, INPUT_PULLUP);
  digitalWrite(S0_PIN, HIGH);
  digitalWrite(S1_PIN, LOW);

  Serial.begin(9600);

  // Uncomment the next line if you want to reset EEPROM on startup
  // resetEEPROM();

  // Load trained colors from EEPROM
  loadColorsFromEEPROM();
}

void loop() {

  if (digitalRead(BTN_PIN_display) == LOW) {
    while (digitalRead(BTN_PIN_display) == LOW) {}
    display_count++;

    if (display_count > 3) { display_count = 0; }
  }
  display();
  readColor();



  delay(1000);
}

void display() { 
  if (display_count == 1) { //  hoạt dộng chính trong này
    // man hinh dem so qua chin va so qua xanh
    if (digitalRead(BTN_PIN_2) == LOW) {
      while (digitalRead(BTN_PIN_2) == LOW) {
      }
      isStart = !isStart;
    }
    if (isStart) {
      digitalWrite(Relay_dongco, HIGH);
    } else {
      digitalWrite(Relay_dongco, LOW);
    }

    lcd.setCursor(0, 0);
    lcd.print("Qua chin:");
    lcd.print(qua_chin);
    lcd.setCursor(0, 1);
    lcd.print("Qua xanh:");
    lcd.print(qua_xanh);

  } else if (display_count == 2) {
    if (digitalRead(BTN_PIN_1) == LOW) {
      while (digitalRead(BTN_PIN_1) == LOW) {}
      trainingColor();
    }
    lcd.setCursor(0, 0);
    lcd.print("Training Color");
    if (colorCount >= 0 && colorCount < 35) {
      lcd.setCursor(0, 1);
      lcd.print("Qua xanh thu:");
      lcd.print(colorCount + 1);
    } else if (colorCount >= 35 && colorCount < 70) {
      lcd.setCursor(0, 1);
      lcd.print("Qua do thu:  ");
      lcd.print(colorCount - 34);
    }

     else if (display_count == 0) {
    // man hinh home , bat tat dong co den quat
    if (digitalRead(BTN_PIN_1) == LOW) {
      while (digitalRead(BTN_PIN_1) == LOW) {
      }
      digitalWrite(Relay_dongco, !digitalRead(Relay_dongco));
    }
    if (digitalRead(BTN_PIN_2) == LOW) {
      while (digitalRead(BTN_PIN_2) == LOW) {
      }
      digitalWrite(Relay_quat, !digitalRead(Relay_quat));
    }
    lcd.setCursor(0, 0);
    lcd.print("Bang chuyen:");
    if (digitalRead(Relay_dongco) == HIGH) {
      lcd.print("ON");
    } else {
      lcd.print("OFF");
    }
    lcd.setCursor(0, 1);
    lcd.print("Quat hut:");
    if (digitalRead(Relay_quat) == HIGH) {
      lcd.print("ON");
    } else {
      lcd.print("OFF");
    }
  }

  else if (display_count == 3) {
    // man hinh home , bat tat dong co den quat

    if (digitalRead(BTN_PIN_1) == LOW) {
      delay(4000);
      while (digitalRead(BTN_PIN_1) == LOW) {
        resetEEPROM();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Done!!!");
      }
    }
    lcd.setCursor(0, 0);
    lcd.print("Xoa du lieu da train ?");
    lcd.setCursor(0, 1);
    lcd.print("Hold_1_ToReset");
  } else if (display_count == 5) {
    // 

    if (digitalRead(BTN_PIN_1) == LOW) {
      delay(4000);
      while (digitalRead(BTN_PIN_1) == LOW) {
        qua_chin = 0;
        qua_xanh = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Done!!!");
      }
    }
    lcd.setCursor(0, 0);
    lcd.print("Xoa du lieu da dem ?");
    lcd.setCursor(0, 1);
    lcd.print("Hold_1_ToRestart");
  }
  lcd.clear();
}
}
void trainingColor() {
  if (colorCount < maxColors) {
    Serial.print("Training Color ");
    Serial.println(colorCount + 1);

    trainedR[colorCount] = process_red_value();
    delay(200);
    trainedG[colorCount] = process_green_value();
    delay(200);
    trainedB[colorCount] = process_blue_value();
    delay(200);

    Serial.print("Trained Color - R: ");
    Serial.print(trainedR[colorCount]);
    Serial.print(" G: ");
    Serial.print(trainedG[colorCount]);
    Serial.print(" B: ");
    Serial.println(trainedB[colorCount]);

    // Save the trained color to EEPROM
    saveColorToEEPROM(colorCount);

    colorCount++;
  } else {
    Serial.println("Maximum number of colors trained.");
  }
}

void readColor() {
  int r = process_red_value();
  delay(200);
  int g = process_green_value();
  delay(200);
  int b = process_blue_value();
  delay(200);

  Serial.print("r = ");
  Serial.print(r);
  Serial.print(" ");
  Serial.print("g = ");
  Serial.print(g);
  Serial.print(" ");
  Serial.print("b = ");
  Serial.println(b);

  bool colorMatched = false;
  for (int i = 0; i < colorCount; i++) {
    if (abs(r - trainedR[i]) < 10 && abs(g - trainedG[i]) < 10 && abs(b - trainedB[i]) < 10) {
      if (i < 35) {
        Serial.print("Quả Xanh ");
        Serial.println(i + 1);
        qua_xanh++;

      } else if (i >= 35) {
        Serial.print("Quả Đỏ ");
        Serial.println(i + 1);
        qua_chin++;
      }

      colorMatched = true;
      break;
    }
  }

  if (!colorMatched) {
    Serial.println("Color does not match any trained colors");
  }
}

void saveColorToEEPROM(int index) {
  int address = index * 3;
  EEPROM.write(address, trainedR[index]);
  EEPROM.write(address + 1, trainedG[index]);
  EEPROM.write(address + 2, trainedB[index]);
}

void loadColorsFromEEPROM() {
  for (int i = 0; i < maxColors; i++) {
    int address = i * 3;
    trainedR[i] = EEPROM.read(address);
    trainedG[i] = EEPROM.read(address + 1);
    trainedB[i] = EEPROM.read(address + 2);
    if (trainedR[i] != 255 || trainedG[i] != 255 || trainedB[i] != 255) {  // Check for uninitialized EEPROM
      colorCount++;
    }
  }
  Serial.print("Loaded ");
  Serial.print(colorCount);
  Serial.println(" colors from EEPROM.");
}

void resetEEPROM() {
  Serial.println("Resetting EEPROM...");
  for (int i = 0; i < maxColors * 3; i++) {
    EEPROM.write(i, 255);  // Reset all EEPROM addresses to 255
  }
  Serial.println("EEPROM reset complete.");
}

int process_red_value() {
  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, LOW);
  int pulse_length = pulseIn(OUT_PIN, LOW);
  return pulse_length;
}

int process_green_value() {
  digitalWrite(S2_PIN, HIGH);
  digitalWrite(S3_PIN, HIGH);
  int pulse_length = pulseIn(OUT_PIN, LOW);
  return pulse_length;
}

int process_blue_value() {
  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, HIGH);
  int pulse_length = pulseIn(OUT_PIN, LOW);
  return pulse_length;
}
