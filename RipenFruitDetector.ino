#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>  // Thêm thư viện Servo

LiquidCrystal_I2C lcd(0x27, 16, 2);
#define S0_PIN 5  // Sensor pin S0
#define S1_PIN 6
#define S2_PIN 3
#define S3_PIN 2
#define OUT_PIN 4
#define BTN_PIN_1 10        // Training button
#define BTN_PIN_display 11  // Display toggle button
#define Relay_dongco 9      // Motor relay
#define GND 12              // Infrared sensor
#define IR2 A6              // Hồng ngoại phát hiện quả trong buồng đo
#define SERVO1_PIN 8        // Chân điều khiển servo
#define SERVO2_PIN 7        // Chân điều khiển servo
Servo myServo1, myServo2;   // Khởi tạo servo

// Constants for color training
const int maxColors = 70;
int trainedR[maxColors], trainedG[maxColors], trainedB[maxColors];
int colorCount = 0, display_count = 0;
int qua_xanh = 0, qua_chin = 0;
bool isStart = false, isOn = false;
int speed = 255;
void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(S0_PIN, OUTPUT);
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(S3_PIN, OUTPUT);
  pinMode(OUT_PIN, INPUT);
  pinMode(IR2, INPUT);
  pinMode(GND, OUTPUT);
  digitalWrite(GND, LOW);
  pinMode(Relay_dongco, OUTPUT);
  pinMode(BTN_PIN_1, INPUT_PULLUP);

  pinMode(BTN_PIN_display, INPUT_PULLUP);
  //digitalWrite(Relay_dongco, HIGH);
  digitalWrite(S0_PIN, HIGH);
  digitalWrite(S1_PIN, LOW);

  myServo1.attach(SERVO1_PIN);  // Khởi động servo
  myServo2.attach(SERVO2_PIN);
  myServo1.write(100);  // Đặt servo ở vị trí trung gian ban đầu
  myServo2.write(100);
  //Serial.begin(9600);

  // Load trained colors from EEPROM
  loadColorsFromEEPROM();
}

void loop() {
  if (digitalRead(BTN_PIN_display) == LOW) {
    while (digitalRead(BTN_PIN_display) == LOW) {}
    display_count = (display_count + 1) % 6;  // Cycle through display screens
    lcd.clear();                              // Clear the screen when changing display screens
  }

  display();

  // Phát hiện quả và đọc màu nếu quả đang ở trong buồng đo
  // if (digitalRead(IR2) == LOW) {  // IR2 phát hiện quả
  //  delay(500);                   // Đợi ổn định khi quả đến vị trí đo màu

  //}

  delay(500);
}

void display() {
  switch (display_count) {
    case 0:
      displayHomeScreen();
      break;
    case 1:
      displayFruitCounter();
      break;
    case 2:
      displayTrainingScreen();
      break;
    case 3:
      displayResetTrainingPrompt();
      break;
    case 5:
      displayResetCountPrompt();
      break;
    default:
      displayHomeScreen();
      break;
  }
}

void displayHomeScreen() {
  if (digitalRead(BTN_PIN_1) == LOW) {
    while (digitalRead(BTN_PIN_1) == LOW) {}
    // analogWrite(Relay_dongco, !digitalRead(Relay_dongco));  // Toggle motor relay
    isOn = !isOn;
    if (isOn) {
      analogWrite(Relay_dongco, 255);

    } else {
      analogWrite(Relay_dongco, 0);
    }
  }

  lcd.setCursor(0, 0);
  lcd.print("MOTOR:");
  lcd.print(digitalRead(Relay_dongco) == HIGH ? "ON " : "OFF");
  readColor();  // Đọc màu của quả
}

void displayFruitCounter() {

  lcd.setCursor(0, 0);
  lcd.print("RipeCount:");
  lcd.print(qua_chin);
  lcd.print("   ");
  lcd.setCursor(0, 1);
  lcd.print("UnripeCount:");
  lcd.print(qua_xanh);
  lcd.print("   ");
  readColor();  // Đọc màu của quả
}

void displayTrainingScreen() {
  if (digitalRead(BTN_PIN_1) == LOW) {
    while (digitalRead(BTN_PIN_1) == LOW) {}
    trainingColor();
  }

  lcd.setCursor(0, 0);
  lcd.print("Training Color   ");

  lcd.setCursor(0, 1);
  if (colorCount < 35) {

    lcd.print("Unripe fruit:");
    lcd.print(colorCount + 1);
    lcd.print("   ");
  } else if (colorCount < maxColors) {
    lcd.print("Ripe fruit:");
    lcd.print(colorCount - 34);
    lcd.print("   ");
  } else {
    lcd.print("Training Full   ");
  }
}

void displayResetTrainingPrompt() {
  if (digitalRead(BTN_PIN_1) == LOW) {
    delay(4000);  // Long press
    while (digitalRead(BTN_PIN_1) == LOW) {
      resetEEPROM();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Done!!!        ");
    }
  }

  lcd.setCursor(0, 0);
  lcd.print("Del trained Data?");
  lcd.setCursor(0, 1);
  lcd.print("Hold 1 to RS");
}

void displayResetCountPrompt() {
  if (digitalRead(BTN_PIN_1) == LOW) {
    delay(4000);  // Long press
    while (digitalRead(BTN_PIN_1) == LOW) {
      qua_chin = 0;
      qua_xanh = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Done!!!        ");
    }
  }

  lcd.setCursor(0, 0);
  lcd.print("Del counted Data?");
  lcd.setCursor(0, 1);
  lcd.print("Hold 1 to RS");
}

void trainingColor() {
  if (colorCount < maxColors) {
    trainedR[colorCount] = process_red_value();
    delay(100);
    trainedG[colorCount] = process_green_value();
    delay(100);
    trainedB[colorCount] = process_blue_value();
    delay(100);

    saveColorToEEPROM(colorCount);
    colorCount++;
  } else {
    //Serial.println("Maximum number of colors trained.");
  }
}

void readColor() {
  int r = process_red_value();
  delay(2);
  int g = process_green_value();
  delay(2);
  int b = process_blue_value();
  delay(2);

  bool colorMatched = false;
  for (int i = 0; i < colorCount; i++) {
    if (abs(r - trainedR[i]) < 10 && abs(g - trainedG[i]) < 10 && abs(b - trainedB[i]) < 10) {
      if (i < 35) {
        qua_xanh++;
        myServo1.write(60);  // Quả xanh đi thẳng
        myServo2.write(60);  // Quả xanh đi thẳng
      } else if (i >= 35 && i < 70) {
        qua_chin++;
        myServo1.write(100);  // Quả chín gạt sang phải-
        myServo2.write(100);  // Quả chín gạt sang phải
      } else {
        myServo1.write(135);  // Quả chín gạt sang phải
        myServo2.write(135);  // Quả chín gạt sang phải
      }
      colorMatched = true;
      delay(2500);          // Thời gian chờ để servo giữ vị trí
      myServo1.write(100);  // Đặt lại servo về vị trí ban đầu
      myServo2.write(100);  // Đặt lại servo về vị trí ban đầu
      break;
    }
  }

  if (!colorMatched) {
    //Serial.println("Color does not match any trained colors");
  }
}

void saveColorToEEPROM(int index) {
  int address = index * 3;
  EEPROM.write(address, trainedR[index]);
  EEPROM.write(address + 1, trainedG[index]);
  EEPROM.write(address + 2, trainedB[index]);
}

void loadColorsFromEEPROM() {
  colorCount = 0;  // Reset color count before loading
  for (int i = 0; i < maxColors; i++) {
    int address = i * 3;
    trainedR[i] = EEPROM.read(address);
    trainedG[i] = EEPROM.read(address + 1);
    trainedB[i] = EEPROM.read(address + 2);
    if (trainedR[i] != 255 || trainedG[i] != 255 || trainedB[i] != 255) {  // Uninitialized check
      colorCount++;
    }
  }
  //Serial.print("Loaded ");
  //Serial.print(colorCount);
  //Serial.println(" colors from EEPROM.");
}

void resetEEPROM() {
  //Serial.println("Đang xóa dữ liệu EEPROM...");
  for (int i = 0; i < maxColors * 3; i++) {
    EEPROM.write(i, 255);  // Reset tất cả địa chỉ trong EEPROM về 255
  }
  colorCount = 0;  // Đặt lại số lượng màu đã huấn luyện
  qua_xanh = 0;    // Đặt lại bộ đếm quả xanh
  qua_chin = 0;    // Đặt lại bộ đếm quả chín
  //Serial.println("Hoàn thành xóa EEPROM.");

  loadColorsFromEEPROM();
}

int process_red_value() {
  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, LOW);
  return pulseIn(OUT_PIN, LOW);
}

int process_green_value() {
  digitalWrite(S2_PIN, HIGH);
  digitalWrite(S3_PIN, HIGH);
  return pulseIn(OUT_PIN, LOW);
}

int process_blue_value() {
  digitalWrite(S2_PIN, LOW);
  digitalWrite(S3_PIN, HIGH);
  return pulseIn(OUT_PIN, LOW);
}
