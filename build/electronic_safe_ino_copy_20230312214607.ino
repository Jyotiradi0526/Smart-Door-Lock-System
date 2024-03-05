
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>
#include "SafeState.h"
#include "icons.h"
#define SERVO_PIN 6
#define SERVO_LOCK_POS   125
#define SERVO_UNLOCK_POS 20
Servo lockServo;
int buz=13;
int led1=A5;
int led2=A4;
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);


const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2};
byte colPins[KEYPAD_COLS] = {A3, A2, A1, A0};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);
SafeState safeState;

void lock() {
  lockServo.write(SERVO_LOCK_POS);
  safeState.lock();

}

void unlock() {
  lockServo.write(SERVO_UNLOCK_POS);
  
}

void showStartupMessage() {
  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  delay(1000);

  lcd.setCursor(0, 2);
  String message = "GIETU Smart Door";
  for (byte i = 0; i < message.length(); i++) {
    lcd.print(message[i]);
    delay(100);
  }
  delay(500);
}

String inputSecretCode() {
  lcd.setCursor(5, 1);
  lcd.print("[____]");
  lcd.setCursor(6, 1);
  String result = "";
  while (result.length() < 4) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      lcd.print('*');
      result += key;
    }
  }
  return result;
}

void showWaitScreen(int delayMillis) {
  lcd.setCursor(2, 1);
  lcd.print("[..........]");
  lcd.setCursor(3, 1);
  for (byte i = 0; i < 10; i++) {
    delay(delayMillis);
    lcd.print("=");
  }
}

bool setNewCode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter new code:");
  String newCode = inputSecretCode();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirm new code");
  String confirmCode = inputSecretCode();

  if (newCode.equals(confirmCode)) {
    safeState.setCode(newCode);
    return true;
   
  } else {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Code mismatch");
    lcd.setCursor(0, 1);
    lcd.print("Door not locked!");
    
    digitalWrite(led1,HIGH);
     tone(buz,HIGH);
    delay(150);
    noTone(buz);
    delay(100);
    tone(buz,HIGH);
    delay(150);
     noTone(buz);
     delay(150);
    tone(buz,HIGH);
    delay(100);
     noTone(buz);
    delay(400);
    digitalWrite(led1, LOW);
    return false;
  }
}

void showUnlockMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  lcd.setCursor(4, 0);
  lcd.print("Door Unlocked!");
  lcd.setCursor(15, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  digitalWrite(led2, HIGH);
  tone(buz,HIGH);
  delay(500);
  noTone(buz);
  digitalWrite(led2,LOW);
  
}

void safeUnlockedLogic() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.write(ICON_UNLOCKED_CHAR);
  lcd.setCursor(1, 0);
  lcd.print("Press #to lock");
  lcd.setCursor(15, 0);
  lcd.write(ICON_UNLOCKED_CHAR);

  bool newCodeNeeded = true;

  if (safeState.hasCode()) {
    lcd.setCursor(0, 1);
    lcd.print("  A = new code");
    newCodeNeeded = false;
    
    
   
  }

  auto key = keypad.getKey();
  while (key != 'A' && key != '#') {
    key = keypad.getKey();
  }
  bool readyToLock = true;
  if (key == 'A' || newCodeNeeded) {
    readyToLock = setNewCode();
  }

  if (readyToLock) {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.write(ICON_UNLOCKED_CHAR);
    lcd.print(" ");
    lcd.write(ICON_RIGHT_ARROW);
    lcd.print(" ");
    lcd.write(ICON_LOCKED_CHAR);

    safeState.lock();
    lock();
    tone(buz,HIGH);
    delay(200);
    noTone(buz);

  }
}

void safeLockedLogic() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(ICON_LOCKED_CHAR);
  lcd.print(" Door Locked! ");
  lcd.write(ICON_LOCKED_CHAR);
  String userCode = inputSecretCode();
  bool unlockedSuccessfully = safeState.unlock(userCode);
  showWaitScreen(200);
  if (unlockedSuccessfully) {
    showUnlockMessage();
    unlock();
  } 
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Denied!");
    digitalWrite(led1, HIGH);
    tone(buz,HIGH);
    delay(500);
    noTone(buz);
    delay(100);
    tone(buz,HIGH);
    delay(500);
    noTone(buz);
    delay(100);
    tone(buz,HIGH);
    delay(500);
    noTone(buz);
    delay(100);
    tone(buz,HIGH);
    delay(500);
    noTone(buz);
    digitalWrite(led1,LOW);  

  }
  }

void setup() {
  lcd.begin(16, 2);
  init_icons(lcd);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(buz, OUTPUT);
  lockServo.attach(SERVO_PIN);
  Serial.begin(115200);
  if (safeState.locked()) {
    lock();
    
  } else {
    unlock();
   
  }

  showStartupMessage();
}

void loop() {

  if (safeState.locked()) {
    safeLockedLogic();
  } 

  else {
    safeUnlockedLogic();
   
  }
}
