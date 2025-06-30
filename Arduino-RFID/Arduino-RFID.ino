#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

#define RST_PIN 9
#define SS_PIN 10
#define ANALOG_BUTTON_PIN A0

MFRC522 mfrc522(SS_PIN, RST_PIN);  // RFID
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);  // LCD

String tagID = "";
byte readCard[4];
int buttonPressed = 0;

// Button thresholds
int getButton() {
  int val = analogRead(ANALOG_BUTTON_PIN);
  if (val < 100) return 1;
  if (val < 300) return 2;
  if (val < 500) return 3;
  if (val < 750) return 4;
  if (val < 950) return 5;
  return 0;
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  lcd.begin(16, 2);
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("Scan Your Card>>");
}

void loop() {
  buttonPressed = getButton();
  if (buttonPressed == 1) {
    lcd.clear();
    lcd.print("Learning Mode");
    delay(1000);
    learnNewTag();
  }

  while (getID()) {
    lcd.clear();
    lcd.setCursor(0, 0);

    if (isTagAuthorized()) {
      lcd.print(" Access Granted!");
    } else {
      lcd.print(" Access Denied!");
    }

    lcd.setCursor(0, 1);
    lcd.print(" ID : ");
    lcd.print(tagID);
    delay(2000);
    resetLCD();
  }
}

boolean getID() {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;

  tagID = "";
  for (uint8_t i = 0; i < 4; i++) {
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(readCard[i], HEX));
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
      tempID.concat(String(storedTag[k], HEX));
    }
    tempID.toUpperCase();

    if (tempID == tagID) return true;
  }

  return false;
}

void learnNewTag() {
  lcd.clear();
  lcd.print("Scan New Tag...");
  while (!getID());  // Wait for a new tag
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
