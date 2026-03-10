#include <Wire.h>// Library to enable I2C protocol
#include <LiquidCrystal_I2C.h> // Library for LCD
#include <DFRobot_MLX90614.h>  // Library for IR sensor
#include <DFRobot_BloodOxygen_S.h> //Library for MAX30102
#include <SPI.h>// Library for SPI communication protocol
#include <nRF24L01.h>//Library for nRF24L01 module
#include <RF24.h>// Library to send and receive data through nRF24L01

#define I2C_ADDRESS 0x57 //I2C address of MAX30102
DFRobot_BloodOxygen_S_I2C MAX30102(&Wire, I2C_ADDRESS);

RF24 radio(8, 7);                  
const byte address[6] = "HLTH1";  

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

unsigned long startMillis;
unsigned long seconds;

DFRobot_MLX90614_I2C sensor;          
LiquidCrystal_I2C lcd(0x20, 20, 4);   
const int heartPin = A1;

int iteration = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  lcd.backlight();

  radio.begin();
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("\n---Initialising sensors---");
  startMillis = millis();
  seconds = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("---Health Monitor---");

  while (NO_ERR != sensor.begin()) {
    Serial.println("IR sensor (MLX90614) failed");
    delay(3000);
  }

  Serial.println("MLX90614 okay!");
  lcd.setCursor(0, 1);
  lcd.print("MLX90614 okay");

  while (!MAX30102.begin()) {
    Serial.println("MAX30102 Failed");
    delay(1000);
  }

  Serial.println("MAX30102 Okay!");
  MAX30102.sensorStartCollect();
  lcd.setCursor(0, 2);
  lcd.print("MAX30102 okay");

  lcd.setCursor(0, 3);
  lcd.print("ECG okay|NRF okay SU");

  delay(5000);
}

void loop() {
  iteration += 1;
  seconds = (millis() - startMillis) / 1000;

  int heartValue = analogRead(heartPin);
  float ambientTemp = sensor.getAmbientTempCelsius();
  float objectTemp  = sensor.getObjectTempCelsius()+5.5;

  /* ================= AVERAGING FOR SPO2 & BPM ================= */

  int spo2_sum = 0;
  int bpm_sum = 0;
  int validSamples = 0;

  for (int i = 0; i < 5; i++) {
    MAX30102.getHeartbeatSPO2();

    int temp_spo2 = MAX30102._sHeartbeatSPO2.SPO2;
    int temp_bpm  = MAX30102._sHeartbeatSPO2.Heartbeat;

    if (temp_spo2 > 0 && temp_bpm > 0) {
      spo2_sum += temp_spo2;
      bpm_sum += temp_bpm;
      validSamples++;
    }

    delay(100);
  }

  int spo2 = (validSamples > 0) ? spo2_sum / validSamples : 0;
  int bpm  = (validSamples > 0) ? (bpm_sum / validSamples) - 40 : 0;


  /* ================= SERIAL DEBUG ================= */

  Serial.println("\n---------------------------");

  Serial.print("Data Sent!\n");
  Serial.print("Time Elapsed: ");
  Serial.print(seconds);
  Serial.print(" s | Iteration: ");
  Serial.println(iteration);
  Serial.print("ECG Value:");
  Serial.println(heartValue);

  Serial.print("Ambient temperature : ");
  Serial.print(ambientTemp);
  Serial.println(" °C");

  Serial.print("Object temperature : ");
  Serial.print(objectTemp);
  Serial.println(" °C");

  Serial.print("SPO2: ");
  Serial.print(spo2);
  Serial.println("%");

  Serial.print("HeartRate: ");
  Serial.print(bpm);
  Serial.print(" BPM\n");

  /* ================= LCD DISPLAY ================= */

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DS TE:");
  lcd.print(seconds);
  lcd.print("s Itr:");
  lcd.print(iteration);

  lcd.setCursor(0, 1);
  lcd.print("ECG Value:");

  if (heartValue < 270) {
    lcd.print(0);
  } else {
    lcd.print(heartValue);
  }

  lcd.setCursor(0, 2);
  lcd.print("AT:");
  lcd.print(ambientTemp);
  lcd.print("C |BT:");
  lcd.print(objectTemp);
  lcd.print("C");

  lcd.setCursor(0, 3);
  lcd.print("HR:");
  if (bpm < 40) {
    lcd.print("-");
  } else {
    lcd.print(bpm);
    lcd.print("BPM");
  }

  lcd.print(" |SPO2:");
  if (spo2 < 60) {
    lcd.print("-");
  } else {
    lcd.print(spo2);
    lcd.print("%");
  }

  /* ================= PACKET TRANSMISSION ================= */

  packet.ecg = heartValue;
  packet.ambientTemp = ambientTemp;
  packet.bodyTemp = objectTemp;
  packet.bpm = bpm;
  packet.spo2 = spo2;
  packet.timeSec = seconds;
  packet.iteration = iteration;

  radio.write(&packet, sizeof(packet));

  delay(3000);
}