#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Servo.h>

#define RST_PIN 9
#define SS_PIN 10
#define ANALOG_BUTTON_PIN A0
#define SERVO_PIN A1  // Use A1 for servo

MFRC522 mfrc522(SS_PIN, RST_PIN);  // RFID
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);  // LCD
Servo myServo;  // Servo object

String tagID = "";
byte readCard[4];
int buttonPressed = 0;

int getButton() {
  int val = analogRead(ANALOG_BUTTON_PIN);
  if (val < 100) return 1; //right
  if (val < 200) return 2; //up
  if (val < 300) return 3; //down
  if (val < 450) return 4; //left
  if (val < 800) return 5; //select
  return 0;
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(A1,OUTPUT);

  lcd.begin(16, 2);
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("Scan Your Card>>");

  myServo.attach(SERVO_PIN);
  myServo.write(90);  // Closed
}

void loop() {
  buttonPressed = getButton();

  if (buttonPressed == 4) {
    lcd.clear();
    lcd.print("Learning Mode");
    delay(1000);
    learnNewTag();
  } else if (buttonPressed == 5) {
    lcd.clear();
    lcd.print("Clearing Tags");
    delay(1000);
    clearEEPROM();
  }

  while (getID()) {
    lcd.clear();
    lcd.setCursor(0, 0);

    if (isTagAuthorized()) {
      lcd.print(" Access Granted!");
      lcd.setCursor(0, 1);
      lcd.print("ID: ");
      lcd.print(tagID);
      activateServo();
      delay(200);
      resetLCD();
    } else {
      lcd.print(" Access Denied!");
      lcd.setCursor(0, 1);
      lcd.print("ID: ");
      lcd.print(tagID);
      delay(2000);
      resetLCD();
    }
  }
}

boolean getID() {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;

  tagID = "";
  for (uint8_t i = 0; i < 4; i++) {
    readCard[i] = mfrc522.uid.uidByte[i];
    if (readCard[i] < 0x10) tagID.concat("0");
    tagID.concat(String(readCard[i], HEX));
    if (i < 3) tagID.concat(" ");
  }

  tagID.toUpperCase();
  mfrc522.PICC_HaltA();
  return true;
}

void resetLCD() {
  lcd.clear();
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("Scan Your Card>>");
}

bool isTagAuthorized() {
  byte storedTag[4];
  int tagCount = EEPROM.read(100);

  for (int i = 0; i < tagCount; i++) {
    for (int j = 0; j < 4; j++) {
      storedTag[j] = EEPROM.read(i * 4 + j);
    }

    String tempID = "";
    for (int k = 0; k < 4; k++) {
      if (storedTag[k] < 0x10) tempID.concat("0");
      tempID.concat(String(storedTag[k], HEX));
      if (k < 3) tempID.concat(" ");
    }
    tempID.toUpperCase();

    if (tempID == tagID) return true;
  }

  return false;
}

void learnNewTag() {
  lcd.clear();
  lcd.print("Scan New Tag...");
  while (!getID());
  delay(500);

  if (isTagAuthorized()) {
    lcd.clear();
    lcd.print("Tag Already");
    lcd.setCursor(0, 1);
    lcd.print("Authorized!");
    delay(2000);
    resetLCD();
    return;
  }

  int tagCount = EEPROM.read(100);
  if (tagCount >= 10) {
    lcd.clear();
    lcd.print("Storage Full!");
    delay(2000);
    resetLCD();
    return;
  }

  for (int i = 0; i < 4; i++) {
    EEPROM.write(tagCount * 4 + i, readCard[i]);
  }

  EEPROM.write(100, tagCount + 1);

  lcd.clear();
  lcd.print("Tag Stored!");
  delay(2000);
  resetLCD();
}

void clearEEPROM() {
  for (int i = 0; i < 100; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.write(100, 0);

  lcd.clear();
  lcd.print("EEPROM Cleared!");
  delay(2000);
  resetLCD();
}

void activateServo() { 
  myServo.write(0);    // Full speed one direction (open)
  delay(150);         // Run 
  myServo.write(90);   // Stop
  delay(5000);         // Wait 
  myServo.write(180);  // Full speed other direction (close)
  delay(150);         // Run 
  myServo.write(90);   // Stop
}
