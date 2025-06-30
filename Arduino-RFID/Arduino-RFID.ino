/*
 * -------------------------------------
 *             MFRC522      Arduino     
 *             Reader       Uno    
 * Signal      Pin          Pin         
 * --------------------------------------
 * RST/Reset   RST          9            
 * SPI SS      SDA(SS)      10           
 * SPI MOSI    MOSI         11 / ICSP-4  
 * SPI MISO    MISO         12 / ICSP-1  
 * SPI SCK     SCK          13 / ICSP-3  
 */

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

#define RST_PIN         9          
#define SS_PIN          10         

MFRC522 mfrc522(SS_PIN, RST_PIN);

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

void setup() {
	Serial.begin(9600);		
	while (!Serial);		
	SPI.begin();			
	mfrc522.PCD_Init();		
	delay(4);				
	mfrc522.PCD_DumpVersionToSerial();	
	Serial.println(F("Scan TAG to see info..."));
	lcd.begin(16, 2);  
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

 mfrc522.PICC_DumpToSerial(&(mfrc522.uid));  // show in Serial Monitor

  lcd.clear();                
  lcd.setCursor(0, 0);
  lcd.print("UID:");

for (byte i = 0; i < mfrc522.uid.size; i++) {
  if (mfrc522.uid.uidByte[i] < 0x10)
    lcd.print("0"); 

  lcd.print(mfrc522.uid.uidByte[i], HEX);

  if (i != mfrc522.uid.size - 1) {
    lcd.print(" "); // Add space between bytes
  }
}

delay(2000);  // Pause to let user read it
lcd.clear();
lcd.print("Scan RFID tag");

}
