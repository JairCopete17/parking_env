#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

/////////////////////////
//////// Entrada ////////
#define NR_OF_READERS 2
#define RST_PIN       22
#define SS_1_PIN      21
#define SS_2_PIN      5
#define SERVO_IN_PIN  2
#define SERVO_OUT_PIN 4
/////////////////////////
/////// Parking 1 ///////
//#define GREEN_1_PIN   A
//#define RED_1_PIN     B
//#define YELLOW_1_PIN  C
/////////////////////////
/////// Parking 2 ///////
//#define GREEN_2_PIN   A
//#define RED_2_PIN     B
//#define YELLOW_2_PIN  C
/////////////////////////
///// Parking Bici //////
//#define RED_BICI_PIN  B
//#define FIN_CAR_PIN   A
#define SS_3_PIN        A
// #define ELECTOIMAN   A
/////////////////////////
//// Parking Scooter ////
//#define RED_BICI_PIN  B
//#define TRIG_PIN      A
//#define ECHO_PIN      A
//#define PINZA_PIN     A
//#define RFID          A

byte ssPins[] = {SS_1_PIN, SS_2_PIN};
MFRC522 mfrc522[NR_OF_READERS];
Servo servoIn;
Servo servoOut;

int cnt = 0;
int initposIn = 140;
int initposOut = 35;
byte LecturaUID[4];
byte Usuario1[4]= {0x0C, 0x51, 0x8B, 0x17} ;    // UID de llavero
byte Usuario2[4]= {0xB4, 0x56, 0xC9, 0xE7} ;    // UID de carnet de la universidad Santiago

void setup() {
  Serial.begin(9600);
  SPI.begin();
  
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
  
  servoIn.attach(SERVO_IN_PIN, 500, 2400);
  servoIn.write(initposIn);
  servoOut.attach(SERVO_OUT_PIN, 500, 2400);
  servoOut.write(initposOut);
  Serial.println("Inicio del registro");
}

void loop() {
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      Serial.print(F(": Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));

      mfrc522[reader].PICC_HaltA();
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader

  servoIn.write(initposIn);
  servoOut.write(initposOut);
  delay(500);
  servoIn.write(35);
  servoOut.write(75);
  delay( 500);
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
