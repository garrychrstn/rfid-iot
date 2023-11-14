#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);
int authorizedUidAddress = 0; // EEPROM address for storing authorized UID

void dump_byte_array(byte *buffer, byte bufferSize);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("1 for read; 2 for register; 3 for login"));
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    Serial.flush();
    Serial.println("Received command: " + String(command));
    switch (command) {
    case 'a':
    case 'a' + ' ':
    case 'a' - ' ':
        // Read RFID card
        Serial.println("Ready to read");
        readCard();
        break;

      case 'b':
        // Register RFID card
        Serial.println("Ready to Register");
        registerCard();
        break;

      case 'c':
        // Login with RFID card
        Serial.println("Ready to login");
        login();
        break;

      default:
        Serial.println("Invalid command");
        break;
    }
  }
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
  Serial.println();
}

void readCard() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("UID tag: ");
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
}

void registerCard() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print("UID tag: ");
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);

    // Save UID to EEPROM
    saveUidToEeprom(mfrc522.uid.uidByte, mfrc522.uid.size);

    mfrc522.PICC_HaltA();
  }
}

void login() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print("Scanned UID: ");
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);

    // Compare the scanned UID with the authorized UID in EEPROM
    if (checkAuthorizedUid(mfrc522.uid.uidByte, mfrc522.uid.size)) {
      Serial.println("Access granted!");
      // Add your access control logic here
    } else {
      Serial.println("Access denied!");
      // Add your access denied logic here
    }

    mfrc522.PICC_HaltA();
  }
}

void saveUidToEeprom(byte *uid, byte uidSize) {
  int eepromAddress = calculateEepromAddress(uid, uidSize);

  for (byte i = 0; i < uidSize; i++) {
    EEPROM.write(eepromAddress + i, uid[i]);
  }

  Serial.println("UID saved to EEPROM.");
}

int calculateEepromAddress(byte *uid, byte uidSize) {
  int address = 0;
  for (byte i = 0; i < uidSize; i++) {
    address += uid[i];
  }
  return address;
}

bool checkAuthorizedUid(byte *scannedUid, byte uidSize) {
  byte storedUid[uidSize];
  for (byte i = 0; i < uidSize; i++) {
    storedUid[i] = EEPROM.read(authorizedUidAddress + i);
  }

  for (byte i = 0; i < uidSize; i++) {
    if (scannedUid[i] != storedUid[i]) {
      return false; // Access denied
    }
  }

  return true; // Access granted
}
