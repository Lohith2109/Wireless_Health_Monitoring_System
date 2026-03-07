#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>

/* ================= NRF ================= */
RF24 radio(8, 7);                 
const byte address[6] = "HLTH1";

/* ================= LCD ================= */
LiquidCrystal_I2C lcd(0x20, 20, 4);

/* ================= RFID ================= */
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// Authorized UID (F2 68 68 4B)
byte authorizedUID[4] = {0xF2, 0x68, 0x68, 0x4B};

/* ================= PACKET ============== */
struct HealthPacket {
  int ecg;
  float ambientTemp;
  float bodyTemp;
  int bpm;
  int spo2;
  unsigned long timeSec;
  int iteration;
};

HealthPacket packet;

/* ================= SESSION CONTROL ===== */
bool accessGranted = false;
unsigned long sessionStart = 0;

void setup() {

  Serial.begin(9600);
  Wire.begin();
  SPI.begin();

  lcd.init();
  lcd.backlight();

  rfid.PCD_Init();

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.startListening();

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("---Health Monitor---");
  lcd.setCursor(0,1);
  lcd.print("Receiver Unit");

  delay(3000);
}

void loop() {

  /* ================= RFID LOGIN MODE ================= */
  while (!accessGranted) {

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Scan your card");

    while (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
      // waiting for card
    }

    bool match = true;
    for (byte i = 0; i < 4; i++) {
      if (rfid.uid.uidByte[i] != authorizedUID[i]) {
        match = false;
        break;
      }
    }

    lcd.clear();

    if (match) {
      lcd.setCursor(0,0);
      lcd.print("Access Granted");
      Serial.println("Access Granted");
      accessGranted = true;
      sessionStart = millis();   // Start 20s timer
      delay(2000);
    } 
    else {
      lcd.setCursor(0,0);
      lcd.print("Access Denied");
      Serial.println("Access Denied");
      delay(2000);
    }

    rfid.PICC_HaltA();
  }

  /* ================= SESSION ACTIVE ================= */
  if (accessGranted) {

    // Check if 20 seconds passed
    if (millis() - sessionStart >= 20000) {

      accessGranted = false;   // Reset access
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Session Expired");
      delay(2000);
      return;   // Go back to RFID mode
    }

    if (radio.available()) {

      radio.read(&packet, sizeof(packet));

      Serial.println("---------------------------");
      Serial.println("Receiver Unit");
      Serial.print("ECG Value: ");
      Serial.println(packet.ecg);

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("DR TE:");
      lcd.print(packet.timeSec);
      lcd.print("s Itr:");
      lcd.print(packet.iteration);

      lcd.setCursor(0, 1);
      lcd.print("ECG Value:");
      lcd.print(packet.ecg);

      lcd.setCursor(0, 2);
      lcd.print("AT:");
      lcd.print(packet.ambientTemp,1);
      lcd.print("C |BT:");
      lcd.print(packet.bodyTemp,1);
      lcd.print("C");

      lcd.setCursor(0, 3);
      lcd.print("HR:");
      lcd.print(packet.bpm);
      lcd.print(" |SPO2:");
      lcd.print(packet.spo2);
      lcd.print("%");
    }
  }
}