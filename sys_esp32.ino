/////////////////////////
/////// Entradas ////////
#define NR_OF_READERS 2
#define RST_PIN       22
#define SS_1_PIN      21
#define SS_2_PIN      5
#define SERVO_IN_PIN  4 
#define SERVO_OUT_PIN 2
/////////////////////////
///// Parking Bici //////
//#define SS_3_PIN    A
//#define FIN_CAR_PIN A
//#define ELECTOIMAN  A
/////////////////////////
//// Parking Scooter ////
//#define SS_4_PIN    A
/////////////////////////

#include <SPI.h>
#include <MFRC522.h>
#include <NewPing.h>
#include <ESP32Servo.h>

byte ssPins[] = {SS_1_PIN, SS_2_PIN};
MFRC522 mfrc522[NR_OF_READERS];
Servo servoIn;
Servo servoOut;
Servo pinza;
NewPing sonar(17, 18, 200);

int cnt = 0;
int initposIn = 140;
int finalposIn = 35;
int initposOut = 90;
int finalposOut = 0;
byte LecturaUID[4];
byte Usuario1[4]= {0x0C, 0x51, 0x8B, 0x17} ;    // UID de llavero
byte Usuario2[4]= {0xB4, 0x56, 0xC9, 0xE7} ;    // UID de carnet de la universidad Santiago
byte Usuario3[4]= {0xB2, 0xA9, 0xE6, 0xE9} ;    // UID de carnet de la universidad Jair

void setup() {
  Serial.begin(9600);
  SPI.begin();
  
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN);
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
  
  servoIn.attach(SERVO_IN_PIN, 500, 2400);
  servoIn.write(initposIn);
  servoOut.attach(SERVO_OUT_PIN, 500, 2400);
  servoOut.write(initposOut);
  pinza.attach(15, 500, 2400);
  pinza.write(0);

  //  pinMode(14, OUTPUT);  // Parking 1
  //  pinMode(12, OUTPUT);
  //  pinMode(13, OUTPUT);
  //  pinMode(25, OUTPUT);  // Parking 2
  //  pinMode(26, OUTPUT);
  //  pinMode(27, OUTPUT);
  //  pinMode(33, OUTPUT);  // Parking bici
  //  pinMode(32, OUTPUT);  // Parking scooter
  //  
  //  digitalWrite(14, HIGH); delay(100);
  //  digitalWrite(12, HIGH); delay(100);
  //  digitalWrite(13, HIGH); delay(100);
  //  digitalWrite(25, HIGH); delay(100);
  //  digitalWrite(26, HIGH); delay(100);
  //  digitalWrite(27, HIGH); delay(100);
  //  digitalWrite(32, HIGH); delay(100);
  //  digitalWrite(33, HIGH); delay(100);

  Serial.println("Inicio del registro");
}

void loop() {
  ultrasonido();
  digitalWrite(14, HIGH); delay(100);
  digitalWrite(25, HIGH); delay(100);
  
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      Serial.print(F(": Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();

      for (byte i = 0; i < mfrc522[reader].uid.size; i++) {
        LecturaUID[i] = mfrc522[reader].uid.uidByte[i];
      }

      if (reader == 0) {
        Serial.println("LECTOR DE SALIDA DETECTADO");
        servoOut.write(finalposOut);
        delay(2500);
        servoOut.write(initposOut);

        digitalWrite(14, HIGH);
      }
      
      if (reader == 1) {
        Serial.println("LECTOR DE ENTRADA DETECTADO");
        servoIn.write(finalposIn);
        delay(2500);
        servoIn.write(initposIn);
      }

      mfrc522[reader].PICC_HaltA();
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void ultrasonido(){
  // Trigger D17 Echo D18
  float distance_cm = sonar.ping_cm();
  Serial.print("Distancia: ");
  Serial.print(distance_cm);
  Serial.println(" cm");

  if (distance_cm >= 10.0) pinza.write(45);
    else pinza.write(0);
}
